# Google Test Suite

This document describes the test categories under `suite/` — what each one
verifies, and the individual test files it contains.

| Category | Focus |
|---|---|
| [Concurrency](#concurrency) | Thread-safety — concurrent `allow()` calls from multiple threads, and correctness under simultaneous access |
| [Integration](#integration) | Multiple components working together end-to-end, such as `RateLimiter` and its underlying `LRUCache` under eviction pressure |
| [Lifecycle](#lifecycle) | Object lifetime operations — construction, and the state it leaves the limiter in |
| [Regression](#regression) | Specific, previously fixed bugs and documented tradeoffs staying exactly as intended |
| [Unit](#unit) | Individual functions or methods in isolation |
| [Conventions](#conventions) | Registration, assertion, and structure conventions specific to Google Test |

Unlike the benchmark suite, tests validate the library's own correctness
directly — there is no reference implementation to compare against, so
results are simply pass or fail.

Every test is a `TEST()` that Google Test registers automatically at
startup — there's no suite list to maintain by hand, and no sequential ids.
This applies uniformly across every category below.

---

## Concurrency

Verifies thread-safety — concurrent `allow()` calls from multiple threads,
and correctness under simultaneous access.

### Tests

| File | What it covers |
|---|---|
| `external_mutex_test.cpp` | Concurrent `allow()` calls stay correct under RateLimiter's internal locking: same-key calls never exceed the limit, distinct keys stay isolated |

---

## Integration

Verifies multiple components working together end-to-end — for example,
`RateLimiter`'s interaction with the underlying `LRUCache` — rather than a
single function in isolation.

### Tests

| File | What it covers |
|---|---|
| `lru_eviction_reset_test.cpp` | A key evicted mid-window by cache capacity pressure gets a free burst; resident keys stay unaffected |
| `multi_key_isolation_test.cpp` | Independent counts, denial, and expiry per key, including across many distinct keys |
| `sustained_traffic_test.cpp` | Realistic mixed sequences across several consecutive windows, including uneven request timing |

---

## Lifecycle

Verifies object lifetime operations — construction, and the state it leaves
the limiter in.

### Tests

| File | What it covers |
|---|---|
| `construction_test.cpp` | Ctor behavior across valid, zero-limit, zero-window, and zero-capacity parameters |

---

## Regression

Verifies that a specific, previously fixed bug — or a documented tradeoff —
stays exactly as intended. One test per resolved issue or pinned contract,
added at the time it's settled.

### Tests

| File | What it covers |
|---|---|
| `boundary_double_burst_test.cpp` | The documented ~2x burst at a window boundary is allowed, and capped at exactly 2x, no more |
| `zero_limit_regression_test.cpp` | A zero-limit RateLimiter denies the very first `allow()` call for a never-seen key, rather than falling through to the fresh-window branch |

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

| File | What it covers |
|---|---|
| `allow_basic_test.cpp` | First request, requests within limit, denial at limit, denial doesn't mutate state |
| `window_expiry_test.cpp` | Window boundary crossing, exact-boundary edge case, no premature expiry, count resets on expiry |
| `zero_limit_test.cpp` | `requestsPerWindow == 0` denies unconditionally, for every key, across window expiry |

---

## Conventions

- **Registration** — every case is a `TEST(<Suite>, <TestName>)`. The suite
  is the CamelCase name of the area under test with a trailing `Test`
  (`RateLimiterAllowBasicTest`, `RateLimiterConstructionTest`,
  `RateLimiterExternalMutexTest`), and the test name is a CamelCase
  description of the behavior (`DeniesOverLimit`, `ExpiryResetsCount`) — no
  underscores. Google Test registers each `TEST` itself, so there is no
  `run_tests()`, no `REGISTER_TEST_SUITE()`, and no `main` in these files;
  link against `gtest_main` or supply your own. Run one suite with
  `--gtest_filter=RateLimiterAllowBasicTest.*`.
- **Assertions** — `EXPECT_*` by default, so a failing check doesn't stop
  the rest of the case: `EXPECT_TRUE`/`EXPECT_FALSE` for `allow()`'s
  boolean result, and `EXPECT_ANY_THROW` for an expected constructor
  exception (`ConstructZeroCapacity`). A case may hold several `EXPECT_*`
  calls in sequence to check a run of `allow()` results.
- **Structure** — each file includes `<ThrottlePro/RateLimiter.h>`, then
  `<gtest/gtest.h>`. Each opens with a header comment naming its subject
  and a `Coverage:` bullet list, and each `TEST` has a `// Verifies ...`
  comment above it.
- **Isolation** — each case constructs its own `RateLimiter`, so no state
  is shared between cases.
- **Deterministic time** — every case drives the limiter through explicit
  `std::chrono::steady_clock` time points passed directly to `allow()`,
  offsetting a captured `now` by fixed durations to simulate elapsed time —
  never `sleep`, so window expiry and boundary behavior are exact and
  reproducible.
- **Concurrency** — worker threads only call `allow()` and increment an
  atomic counter (or write to their own indexed slot); every `EXPECT_*`
  runs on the main thread after every thread has been `join()`ed, since
  Google Test assertions aren't safe to call from other threads.
