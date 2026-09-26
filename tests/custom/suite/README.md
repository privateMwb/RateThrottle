# Test Suite

This document describes the test categories under `suite/` — what each one
verifies, and the individual test files it contains.

| Category | Focus |
|---|---|
| [Concurrency](#concurrency) | Thread-safety — concurrent `allow()` calls from multiple threads, and correctness under simultaneous access |
| [Integration](#integration) | Multiple components working together end-to-end, such as `RateLimiter` and its underlying `LRUCache` under eviction pressure |
| [Lifecycle](#lifecycle) | Object lifetime operations — construction, and the state it leaves the limiter in |
| [Regression](#regression) | Specific, previously fixed bugs and documented tradeoffs staying exactly as intended |
| [Unit](#unit) | Individual functions or methods in isolation |
| [Conventions](#conventions) | Registration, assertion, and structure conventions specific to the custom framework |

Unlike the benchmark suite, tests validate the library's own correctness
directly — there is no reference implementation to compare against, so
results are simply pass or fail.

Every test suite registers itself automatically via `REGISTER_TEST_SUITE()`
at startup, and is assigned a sequential id within its category (e.g. `C1`,
`C2` for Concurrency; `U1`, `U2` for Unit) — there's no suite list to
maintain by hand. This applies uniformly across every category below.

---

## Concurrency

Verifies thread-safety — concurrent `allow()` calls from multiple threads,
and correctness under simultaneous access.

### Tests

| File | What it covers |
|---|---|
| `external_mutex.cpp` | Concurrent `allow()` calls stay correct under RateLimiter's internal locking: same-key calls never exceed the limit, distinct keys stay isolated |

---

## Integration

Verifies multiple components working together end-to-end — for example,
`RateLimiter`'s interaction with the underlying `LRUCache` — rather than a
single function in isolation.

### Tests

| File | What it covers |
|---|---|
| `lru_eviction_reset.cpp` | A key evicted mid-window by cache capacity pressure gets a free burst; resident keys stay unaffected |
| `multi_key_isolation.cpp` | Independent counts, denial, and expiry per key, including across many distinct keys |
| `sustained_traffic.cpp` | Realistic mixed sequences across several consecutive windows, including uneven request timing |

---

## Lifecycle

Verifies object lifetime operations — construction, and the state it leaves
the limiter in.

### Tests

| File | What it covers |
|---|---|
| `construction.cpp` | Ctor behavior across valid, zero-limit, zero-window, and zero-capacity parameters |

---

## Regression

Verifies that a specific, previously fixed bug — or a documented tradeoff —
stays exactly as intended. One test per resolved issue or pinned contract,
added at the time it's settled.

### Tests

| File | What it covers |
|---|---|
| `boundary_double_burst.cpp` | The documented ~2x burst at a window boundary is allowed, and capped at exactly 2x, no more |
| `zero_limit_regression.cpp` | A zero-limit RateLimiter denies the very first `allow()` call for a never-seen key, rather than falling through to the fresh-window branch |

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

| File | What it covers |
|---|---|
| `allow_basic.cpp` | First request, requests within limit, denial at limit, denial doesn't mutate state |
| `window_expiry.cpp` | Window boundary crossing, exact-boundary edge case, no premature expiry, count resets on expiry |
| `zero_limit.cpp` | `requestsPerWindow == 0` denies unconditionally, for every key, across window expiry |

---

## Conventions

- **Registration** — every case is a `static void <name>()` named for the
  behavior it verifies (`expiry_resets_count`), called from the file's
  `run_tests()` via `RUN(<name>)`. The file ends with
  `REGISTER_TEST_SUITE();`.
- **Assertions** — `CHK(condition)` for every check, and `CHK(!condition)`
  to assert a denial. A case may hold several `CHK`s. An expected
  constructor exception (`construct_zero_capacity`) is asserted by wrapping
  the construction in a `try`/`catch`, setting a `bool`, and checking it
  with `CHK` — there is no dedicated throw-assertion macro in this suite.
- **Structure** — each file opens with a header comment naming its subject
  and a `Coverage:` bullet list, each case has a `// Verifies ...` comment
  above it, and `run_tests()` has `// Executes all <subject> test cases.`
  above it.
- **Isolation** — each case constructs its own `RateLimiter`, so no state is
  shared between cases.
- **Deterministic time** — every case drives the limiter through explicit
  `std::chrono::steady_clock` time points passed directly to `allow()`,
  offsetting a captured `now` by fixed durations to simulate elapsed time —
  never `sleep`, so window expiry and boundary behavior are exact and
  reproducible.
- **Concurrency** — worker threads only call `allow()` and increment an
  atomic counter (or write to their own indexed slot); every `CHK` runs on
  the main thread after every thread has been `join()`ed.
