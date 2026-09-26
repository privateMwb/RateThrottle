# Changelog

All notable changes to RateThrottle are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Nothing yet.

## [1.0.0] - 2026-08-15

The first stable release of RateThrottle, a framework-agnostic fixed-window
rate limiter for modern C++, backed by `CachePro::LRUCache`.

### Added
- Fixed-window rate limiting: per-key request count and window-start
  timestamp, tracked via `CachePro::LRUCache`.
- Framework-agnostic core — no knowledge of HTTP, requests, or middleware;
  `RateLimiter::allow(key, now)` is the entire surface.
- Constructor-configurable limits: requests-per-window, window duration, and
  cache capacity are runtime parameters, not hardcoded constants.
- Thread-safe by design: all cache access guarded by an internal mutex,
  since `CachePro::LRUCache` provides no built-in synchronization of its own.
- Deterministic testing support: `allow()` accepts an explicit `now`
  timestamp, so window-boundary behavior can be tested without real time
  passing.
- `rain::` namespace alias for `RateLimiter`.

### Known Trade-offs
- Boundary bursting: a client can burst up to ~2x the configured limit at a
  window boundary — an accepted consequence of the fixed-window algorithm,
  not fixed in v1 (sliding-window and token-bucket are candidate v2
  algorithms).
- LRU-eviction interaction: an evicted-but-still-active key's window
  silently restarts on its next request, rather than being hidden as an
  edge case.

### Performance
- Favors simplicity and a small surface over the pool-allocated,
  open-addressing design of `CachePro::LRUCache` itself — `RateLimiter` is a
  thin, mutex-guarded layer on top of it, not a from-scratch data structure.
- A single global mutex guards the whole cache rather than sharding per key;
  contention under concurrent load on a shared key is the most expensive
  path measured (`Allow() Contention`, 1M tier: 691.20 ms), a direct,
  honest cost of not sharding the lock in v1.
- The increment path (`allow()` on an existing key, within its window)
  mutates the stored window in place via a live pointer from `get()`, with
  no re-insertion into the cache.
- Capacity is fixed at construction; there is no `resize()`-family API.
  Benchmarked at 100 and 1,000,000 fixed capacities to measure per-call cost
  as capacity scales (`Small Capacity` vs. `Large Capacity`).
- Benchmarked across nine operation categories at up to 1M iterations; the
  dominant cost driver is whether a call constructs and hashes a brand-new
  unique key (`New Key`, `Large Capacity`, `Eviction Pressure`: 4–7x costlier
  than same-key operations), not which allow/deny branch is taken. Full
  results, all iteration tiers: `benchmarks/results/v1_0_0.md`.

### Testing
- Comprehensive test suite covering allow/deny transitions across a window
  boundary; boundary-burst behavior (proven to occur, not prevented); window
  reset after expiry; the LRU-eviction-mid-window edge case; and concurrent
  access from multiple threads.
- 100.0% line coverage (17/17) and 100.0% function coverage (2/2), excluding
  test infrastructure and third-party dependencies.

### CI
- Automated builds and tests across GCC, Clang, MSVC, and AppleClang, each
  in Debug and Release configurations.

[Unreleased]: https://github.com/privateMwb/RateThrottle/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/privateMwb/RateThrottle/releases/tag/v1.0.0
