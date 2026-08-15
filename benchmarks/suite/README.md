# Benchmark Suite

This document describes the benchmark categories under `suite/` — what
each one measures, and the individual benchmarks it contains.

RateLimiter has no naive baseline to compare against — there is no
`stdThrottle`. Every `BENCH()` call below times `RateLimiter` alone.

Every `BENCH()` call, in every category below, is automatically repeated
at three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) —
to smooth out timing noise and show whether relative performance holds
steady as call volume increases. This applies uniformly across the whole
suite; it is not specific to any one category. The **Scaling** category
below measures something different: how per-call cost changes as
capacity itself grows, independent of iteration count.

---

## Access

Benchmarks `allow()` under concurrent access — the same key contended by
multiple threads at once, rather than a single caller in isolation.

### Benchmarks

- `contention.cpp` — `allow()` on a shared key, contended by a
  background thread pool hammering the same limiter's mutex

---

## Core

Benchmarks the fundamental, most frequently exercised paths through
`allow()` — new keys, repeated requests within a window, denial once
the limit is reached, and window expiry.

### Benchmarks

- `allow.cpp` — `allow()` new key (fresh window insert), within window
  (in-place increment), deny (limit reached), window reset (expired
  window)

---

## Lifecycle

Benchmarks object lifetime operations — construction. `RateLimiter` has
no copy or move semantics of its own to benchmark (it holds a
`std::mutex`, which is neither copyable nor movable, so both are
implicitly deleted).

### Benchmarks

- `construction.cpp` — constructing an empty `RateLimiter` sized for a
  representative capacity

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

- `eviction_pressure.cpp` — `allow()` on a new key with the cache
  already at capacity (steady-state LRU eviction on every call)
- `capacity_scaling.cpp` — steady-state eviction cost compared between a
  small and a large fixed capacity

---

## Utility

Benchmarks introspection and bookkeeping operations that don't belong to
any of the categories above.

### Benchmarks

None yet — `RateLimiter` currently exposes no introspection surface
(no hit/miss counters, no current-state reporting). Revisit once one
exists.
