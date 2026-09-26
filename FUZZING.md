# Fuzzing

RateThrottle is fuzzed via [ClusterFuzzLite](https://google.github.io/clusterfuzzlite/),
running on every pull request that touches the fuzzed files, plus a
longer scheduled batch run every night.

## What's covered

**`fuzz_rate_limiter.cpp`** is a differential fuzzer for
`ThrottlePro::RateLimiter`. It runs the same sequence of `allow()`
calls against the real limiter and a shadow model that implements the
fixed-window algorithm directly, comparing the allow/deny result after
every single call (not just at the end), so a failing input localizes
to the exact call that broke an invariant.

Time is fully fuzzer-controlled: rather than wall-clock time, each
step advances a synthetic `steady_clock::time_point` by a
fuzzer-chosen delta, deliberately biased toward landing exactly on, or
one tick either side of, the window boundary — that's exactly where a
fixed-window off-by-one would hide.

The shadow model is only reliable for keys that can't be evicted, so
the harness splits keys into two pools:

- **Steady keys**, drawn from a fixed pool no larger than the
  configured cache capacity. Since the real `LRUCache` can never need
  to evict one of these, every `allow()` call on a steady key is
  checked bit-for-bit against the shadow model.
- **Overflow keys**, drawn from a strictly-increasing counter, used to
  push the cache past capacity and force real eviction. These aren't
  differentially checked — there's no reliable oracle here for which
  key `CachePro::LRUCache` evicts — but they still run under
  ASan/UBSan, and the harness explicitly checks the one invariant that
  must hold regardless of eviction: `requestsPerWindow == 0` always
  denies.

Specifically exercised:

- Fresh-window-on-first-sight and fresh-window-on-expiry, exactly at
  the `>= windowDuration_` boundary — not one tick before or after.
- Count increment and deny-at-limit for `requestsPerWindow` across a
  range of small values, including 1.
- `requestsPerWindow == 0` short-circuiting to always-deny, for
  never-seen keys, keys with an active window, and evicted/overflow
  keys alike.
- `RateLimitWindow` state surviving the `get()`-then-mutate-in-place
  path (`++existing->count`, no `put()`) across many repeated calls.

Built and run under both AddressSanitizer and UndefinedBehaviorSanitizer.

## What's deliberately NOT covered yet

- **Which specific key `CachePro::LRUCache` evicts under contention.**
  Predicting that would mean differentially modeling `LRUCache` itself
  inside this harness — that belongs in CachePro's own fuzz suite, not
  here. This harness only asserts the one eviction-independent
  invariant (`requestsPerWindow == 0` → always deny) for keys in the
  eviction-pressure pool.
- **Concurrent `allow()` calls from multiple threads.** `RateLimiter`'s
  internal `mutex_` is a thread-safety claim, not just a sequential
  algorithm one, and libFuzzer's single-threaded-per-input model
  doesn't exercise that. A real concurrency test needs a separate,
  threaded harness (e.g. ThreadSanitizer plus several threads hammering
  shared keys) — a natural follow-up harness, not a change to this one.

## Running locally

```bash
git clone --recursive https://github.com/google/oss-fuzz.git
cd oss-fuzz
python infra/helper.py build_fuzzers --sanitizer address RateThrottle /path/to/RateThrottle
python infra/helper.py run_fuzzer RateThrottle fuzz_rate_limiter
```

Or, without OSS-Fuzz's tooling, directly with clang (assuming a
checkout of CachePro alongside RateThrottle):

```bash
clang++ -std=c++20 -fsanitize=fuzzer,address \
  -Iinclude -I../CachePro/include \
  src/ThrottlePro/RateLimiter.cpp \
  fuzz/fuzz_rate_limiter.cpp \
  -o fuzz_rate_limiter

./fuzz_rate_limiter
```

Add `-fsanitize=fuzzer,undefined` instead to run under UBSan.

## Reproducing a crash

ClusterFuzzLite uploads the failing input as a workflow artifact when
a run fails. Download it, then:

```bash
./fuzz_rate_limiter path/to/crash-<hash>
```

This replays that exact byte sequence through
`LLVMFuzzerTestOneInput()` once, deterministically — no sanitizer flags
needed beyond however the binary was already built.

## Adding a new harness

1. Add `fuzz/fuzz_<target>.cpp` with an `extern "C" int
   LLVMFuzzerTestOneInput(const uint8_t*, size_t)` entry point.
2. Add the matching compile + link block to `.clusterfuzzlite/build.sh`.
3. No workflow changes needed — `cflite_pr.yml`/`cflite_batch.yml`
   build and run every binary `build.sh` produces in `$OUT`.
