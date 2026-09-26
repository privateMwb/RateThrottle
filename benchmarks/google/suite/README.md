# Google Benchmark Suite

This document describes the benchmark categories under `benchmarks/` — what each
one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Concurrency](#concurrency) | `allow()` under contention from multiple threads on the same key |
| [Core](#core) | The fundamental paths through `allow()` — new key, within window, deny, window reset |
| [Lifecycle](#lifecycle) | Construction |
| [Scaling](#scaling) | Cost vs. capacity, independent of iteration count |
| [Utility](#utility) | Introspection and bookkeeping operations |
| [Conventions](#conventions) | Registration, sizing, and elision conventions specific to Google Benchmark |

`RateLimiter` has no naive baseline to compare against — there is no
`stdThrottle`. Every benchmark below is solo: a single benchmark timing
`RateLimiter` alone, with no paired counterpart.

Every benchmark is run by [Google Benchmark](https://github.com/google/benchmark),
which scales each one's iteration count automatically until timing is stable —
there are no fixed SMALL/MEDIUM/LARGE tiers. This applies uniformly across the
whole suite; it is not specific to any one category. The **Scaling** category
below measures something different: how per-call cost changes as capacity
itself grows, independent of iteration count.

---

## Concurrency

Benchmarks `allow()` under concurrent access — the same key contended by
multiple threads at once, rather than a single caller in isolation.

### Benchmarks

| File | What it covers |
|---|---|
| `contention.cpp` | `allow()` on a shared key, contended by a background thread pool hammering the same limiter's mutex |

---

## Core

Benchmarks the fundamental, most frequently exercised paths through
`allow()` — new keys, repeated requests within a window, denial once the
limit is reached, and window expiry.

### Benchmarks

| File | What it covers |
|---|---|
| `allow.cpp` | `allow()` new key (fresh window insert), within window (in-place increment), deny (limit reached), window reset (expired window) |

---

## Lifecycle

Benchmarks object lifetime operations — construction. `RateLimiter` has no
copy or move semantics of its own to benchmark (it holds a `std::mutex`,
which is neither copyable nor movable, so both are implicitly deleted).

### Benchmarks

| File | What it covers |
|---|---|
| `construction.cpp` | Constructing an empty `RateLimiter` sized for a representative capacity |

---

## Scaling

Benchmarks whether `allow()`'s per-call cost changes as capacity itself
grows — a separate axis from iteration count: iteration count repeats the
same fixed-size operation more times, while Scaling changes the cache's
capacity itself and observes the resulting cost.

`RateLimiter` has no `resize()`-family API — capacity is fixed at
construction — so this category constructs limiters at different fixed
capacities rather than timing a resize operation directly.

### Benchmarks

| File | What it covers |
|---|---|
| `eviction_pressure.cpp` | `allow()` on a new key with the cache already at capacity (steady-state LRU eviction on every call) |
| `capacity_scaling.cpp` | Steady-state eviction cost compared between a small and a large fixed capacity |

---

## Utility

Benchmarks introspection and bookkeeping operations that don't belong to any
of the categories above.

### Benchmarks

None yet — `RateLimiter` currently exposes no introspection surface (no
hit/miss counters, no current-state reporting). Revisit once one exists.

---

## Conventions

- **Registration** — every case is a `static void Name(benchmark::State&
  state)` that does its work in a `for (auto _ : state)` loop, registered
  directly below it with `BENCHMARK(Name)`. The function name is the
  benchmark name — no `->Name()` override, no `BM_` prefix, and no
  `BENCHMARK_MAIN()` (`main` is supplied separately). Since there is no
  `stdThrottle` to pair against, there's no `_stack`/`_std`-style variant
  split: one function per case (`AllowNewKey`, `AllowWithinWindow`,
  `AllowDeny`, `AllowWindowReset`, `Construction`, `AllowEvictionPressure`,
  `AllowSmallCapacity`, `AllowLargeCapacity`, `AllowContention`).
- **Preventing elision** — every loop passes its result to
  `benchmark::DoNotOptimize()` so the call can't be optimized away, including
  the constructed object itself in `construction.cpp`, which has no other
  result to consume.
- **No baseline to match** — with no `stdThrottle` equivalent, there's no
  second implementation whose work must be kept in step with `RateLimiter`'s;
  every benchmark measures `RateLimiter` in isolation.
- **Capacity sizing** — `cacheCapacity` and `requestsPerWindow` constants
  (e.g. `kNewKeyCapacity`, `kWithinWindowRequestsPerWindow`) were carried over
  from the custom framework's fixed 1,110,000-call budget per run. Google
  Benchmark instead scales iteration count on its own until timing is
  stable, so a fast benchmark can run past that original budget; benchmarks
  that generate a fresh key or otherwise grow unboundedly per iteration
  (`AllowNewKey`, `AllowEvictionPressure`, `AllowSmallCapacity`,
  `AllowLargeCapacity`) may need an explicit `->Iterations(N)` pin, sized to
  their capacity constant, to guarantee they can't run past it — not yet
  applied here and worth revisiting.
- **Consumed inputs** — setup stays outside the loop except for state that
  must be fresh every iteration. `allow.cpp`'s new-key benchmark and both
  Scaling benchmarks generate a fresh key per iteration via a counter;
  `allow.cpp`'s window-reset benchmark advances a clock past the window's
  expiry on every iteration; `contention.cpp` spins up its background thread
  pool once, before the timing loop starts, and joins it once, after the
  loop ends.
