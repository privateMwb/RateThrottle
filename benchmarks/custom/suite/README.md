# Benchmark Suite

This document describes the benchmark categories under `suite/` — what each
one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Concurrency](#concurrency) | `allow()` under contention from multiple threads on the same key |
| [Core](#core) | The fundamental paths through `allow()` — new key, within window, deny, window reset |
| [Lifecycle](#lifecycle) | Construction |
| [Scaling](#scaling) | Cost vs. capacity, independent of iteration count |
| [Utility](#utility) | Introspection and bookkeeping operations |
| [Conventions](#conventions) | Registration, sizing, and elision conventions specific to the custom framework |

`RateLimiter` has no naive baseline to compare against — there is no
`stdThrottle`. Every `BENCH()` call below times `RateLimiter` alone.

Every `BENCH()` call, in every category below, is automatically repeated at
three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) — to
smooth out timing noise and show whether relative performance holds steady
as call volume increases. This applies uniformly across the whole suite; it
is not specific to any one category. The **Scaling** category below measures
something different: how per-call cost changes as capacity itself grows,
independent of iteration count.

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
grows — a separate axis from the SMALL/MEDIUM/LARGE iteration tiers
described above: those repeat the same fixed-size operation more times,
while Scaling changes the cache's capacity itself and observes the
resulting cost.

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

- **Registration** — every case is a `static void bench_<n>()` called from
  the file's `run_benchmarks()`, and the file ends with
  `REGISTER_BENCH_SUITE();`. `BENCH_SOLO("label", c)` times `RateLimiter`
  alone — the only form used here, since there is no baseline to pair it
  against.
- **Preventing elision** — every lambda's result is consumed (typically via
  a `(void)` cast) so the call can't be optimized away.
- **No baseline to match** — with no `stdThrottle` equivalent, there's no
  second implementation whose work must be kept in step with `RateLimiter`'s;
  every benchmark measures `RateLimiter` in isolation.
- **Capacity sizing** — `cacheCapacity` and `requestsPerWindow` are sized so
  that, across the cumulative 1,110,000 calls one `BENCH()` run can make
  (10K + 100K + 1M), no benchmark can plausibly exhaust the limit or evict a
  key mid-run unless that is deliberately the thing being measured (`deny`,
  `window_reset`, `eviction_pressure`).
- **Consumed inputs** — setup stays outside the lambda except for state that
  must be fresh every call. `allow.cpp`'s new-key benchmark and both Scaling
  benchmarks generate a fresh key per call via a counter; `allow.cpp`'s
  window-reset benchmark advances a clock past the window's expiry on every
  call; `contention.cpp` spins up its background thread pool once, before
  `BENCH_SOLO()` runs, and joins it once, after `BENCH_SOLO()` returns.
