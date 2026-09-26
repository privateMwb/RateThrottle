# Example Suite

This document describes the example categories under `suite/` — what each
one demonstrates, and the individual example files it contains.

| Category | Focus |
|---|---|
| [Advanced](#advanced) | Deeper mechanics — custom timestamps, boundary bursts, and eviction |
| [Integration](#integration) | Interoperability with the surrounding codebase |
| [Misuse](#misuse) | Common mistakes and the surprising behavior they lead to |
| [Patterns](#patterns) | Usage idioms built on top of the core API |
| [Quickstart](#quickstart) | Fundamental, everyday usage |
| [Conventions](#conventions) | Registration conventions specific to the example suite |

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse where
instructive (see [Misuse](#misuse)), so the reader sees both the correct
pattern and the mistake it guards against.

---

## Advanced

Demonstrates deeper mechanics of the limiter — injecting a request's own
timestamp instead of the call-time default, the documented boundary-burst
tradeoff, and the observable consequence of LRU eviction on a key's window.

### Examples

| File | What it demonstrates |
|---|---|
| `custom_time_source.cpp` | Passing a request's own timestamp instead of the default `now()`, including out-of-order processing |
| `boundary_burst_behavior.cpp` | The documented ~2x burst at a window boundary, and why it's bounded there |
| `eviction_free_burst.cpp` | A key evicted mid-window by cache pressure looks never-seen on its next request |

---

## Integration

Demonstrates interoperability with the rest of a codebase — wrapping the
limiter behind a middleware-style handler, and sizing its cache capacity
relative to the surrounding system's expected load.

### Examples

| File | What it demonstrates |
|---|---|
| `embedding_in_middleware.cpp` | Wrapping RateLimiter as a private implementation detail behind a request-handling class |
| `cache_capacity_tuning.cpp` | Sizing `cacheCapacity` relative to the expected number of distinct clients, and the consequence of undersizing it |

---

## Misuse

Demonstrates common mistakes and the surprising behavior they lead to,
alongside the correct pattern — including the specific tradeoffs and API
contracts that are easy to get backwards.

### Examples

| File | What it demonstrates |
|---|---|
| `zero_limit_surprise.cpp` | `requestsPerWindow == 0` denies everything; it is not a sentinel for "unlimited" |
| `assuming_sliding_window.cpp` | Expecting a rolling-window guarantee the fixed-window algorithm doesn't provide |
| `ignoring_return_value.cpp` | Discarding `allow()`'s `[[nodiscard]]` result silently defeats the throttle |

---

## Patterns

Demonstrates common usage idioms built on top of the core API — keying by
client identity, and driving the limiter deterministically in tests.

### Examples

| File | What it demonstrates |
|---|---|
| `per_client_throttling.cpp` | Keying by client IP, independent state per client across a stream of requests |
| `deterministic_testing.cpp` | Simulating window expiry and multiple windows in sequence via explicit time points, without sleeping |

---

## Quickstart

Demonstrates fundamental, everyday usage — construction, allowing and
denying requests, and reacting correctly to a denial.

### Examples

| File | What it demonstrates |
|---|---|
| `basic_usage.cpp` | Construction, `allow()` up to the limit, denial at the limit, explicit time points |
| `denial_handling.cpp` | Branching on `allow()`'s result, rejecting the caller once the limit is reached |

---

## Conventions

- **Registration** — every example file ends with `REGISTER_EXAMPLE_SUITE()`,
  which derives the suite's category from its containing directory and
  assigns it a sequential id within that category. This applies uniformly
  across every category above.
