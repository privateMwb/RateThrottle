# Example Suite

This document describes the example categories under `suite/` — what
each one demonstrates, and the individual example files it contains.

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse
where instructive (see Misuse), so the reader sees both the correct
pattern and the mistake it guards against.

Every example file ends with `REGISTER_EXAMPLE_SUITE()`, which derives
the suite's category from its containing directory and assigns it a
sequential id within that category. This applies uniformly across
every category below.

---

## Advanced

Demonstrates deeper mechanics of the limiter — injecting a request's
own timestamp instead of the call-time default, the documented
boundary-burst tradeoff, and the observable consequence of LRU
eviction on a key's window.

### Examples

- `custom_time_source.cpp` — passing a request's own timestamp instead of the default now(), including out-of-order processing
- `boundary_burst_behavior.cpp` — the documented ~2x burst at a window boundary, and why it's bounded there
- `eviction_free_burst.cpp` — a key evicted mid-window by cache pressure looks never-seen on its next request

---

## Integration

Demonstrates interoperability with the rest of a codebase — wrapping
the limiter behind a middleware-style handler, and sizing its cache
capacity relative to the surrounding system's expected load.

### Examples

- `embedding_in_middleware.cpp` — wrapping RateLimiter as a private implementation detail behind a request-handling class
- `cache_capacity_tuning.cpp` — sizing cacheCapacity relative to the expected number of distinct clients, and the consequence of undersizing it

---

## Misuse

Demonstrates common mistakes and the surprising behavior they lead to,
alongside the correct pattern — including the specific tradeoffs and
API contracts that are easy to get backwards.

### Examples

- `zero_limit_surprise.cpp` — requestsPerWindow == 0 denies everything; it is not a sentinel for "unlimited"
- `assuming_sliding_window.cpp` — expecting a rolling-window guarantee the fixed-window algorithm doesn't provide
- `ignoring_return_value.cpp` — discarding allow()'s [[nodiscard]] result silently defeats the throttle

---

## Patterns

Demonstrates common usage idioms built on top of the core API —
keying by client identity, and driving the limiter deterministically
in tests.

### Examples

- `per_client_throttling.cpp` — keying by client IP, independent state per client across a stream of requests
- `deterministic_testing.cpp` — simulating window expiry and multiple windows in sequence via explicit time points, without sleeping

---

## Quickstart

Demonstrates fundamental, everyday usage — construction, allowing and
denying requests, and reacting correctly to a denial.

### Examples

- `basic_usage.cpp` — construction, allow() up to the limit, denial at the limit, explicit time points
- `denial_handling.cpp` — branching on allow()'s result, rejecting the caller once the limit is reached
