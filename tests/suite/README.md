# Test Suite

This document describes the test categories under `suite/` — what each
one verifies, and the individual test files it contains.

Unlike the benchmark suite, tests validate the library's own
correctness directly — there is no reference implementation to compare
against, so results are simply pass or fail.

Every test suite registers itself automatically via
`REGISTER_TEST_SUITE()` at startup, and is assigned a sequential id
within its category (e.g. `U1`, `U2` for Unit; `L1`, `L2` for
Lifecycle) — there's no suite list to maintain by hand. This applies
uniformly across every category below.

---

## Concurrency

Verifies thread-safety — concurrent reads and writes from multiple
threads, and correctness under simultaneous access.

### Tests

- `external_mutex.cpp` — Concurrent allow() calls stay correct under RateLimiter's internal locking: same-key calls never exceed the limit, distinct keys stay isolated

---

## Integration

Verifies multiple components working together end-to-end — for
example, RateLimiter's interaction with the underlying LRUCache —
rather than a single function in isolation.

### Tests

- `lru_eviction_reset.cpp` — A key evicted mid-window by cache capacity pressure gets a free burst; resident keys stay unaffected
- `multi_key_isolation.cpp` — Independent counts, denial, and expiry per key, including across many distinct keys
- `sustained_traffic.cpp` — Realistic mixed sequences across several consecutive windows, including uneven request timing

---

## Lifecycle

Verifies object lifetime operations — construction, and the state it
leaves the limiter in.

### Tests

- `construction.cpp` — Ctor behavior across valid, zero-limit, zero-window, and zero-capacity parameters

---

## Regression

Verifies that a specific, previously fixed bug stays fixed, or that a
documented tradeoff continues to behave exactly as specified. One test
per resolved issue, added at the time the fix lands.

### Tests

- `boundary_double_burst.cpp` — The documented ~2x burst at a window boundary is allowed, and capped at exactly 2x, no more
- `zero_limit_regression.cpp` — A zero-limit RateLimiter denies the very first allow() call for a never-seen key, rather than falling through to the fresh-window branch

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

- `allow_basic.cpp` — First request, requests within limit, denial at limit, denial doesn't mutate state
- `window_expiry.cpp` — Window boundary crossing, exact-boundary edge case, no premature expiry, count resets on expiry
- `zero_limit.cpp` — requestsPerWindow == 0 denies unconditionally, for every key, across window expiry
