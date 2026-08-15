/**
 * @file            RateLimiter.h
 *
 * @date            2026-14-8
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 privateMwb
 *                  All rights reserved.
 *                  https://github.com/privateMwb/RateThrottle
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include <chrono>  // std::chrono::steady_clock, std::chrono::milliseconds
#include <cstddef> // std::size_t
#include <mutex>   // std::mutex, std::lock_guard
#include <string>  // std::string
// clang-format on

#include <CachePro/LRUCache.h>

// Framework-agnostic fixed-window rate limiter. Given a key and the
// current time, decides allow/deny. No knowledge of HTTP, requests, or
// middleware — FalconHTTP integration is a separate middleware wrapper
// (Phase 2), not this file.
//
// Algorithm: fixed-window counter, v1. Each key maps to a window-start
// timestamp and a request count, stored in a CachePro::LRUCache keyed by
// client IP (caller's responsibility — this class has no notion of IPs).
// Known, accepted tradeoff: a client can burst up to ~2x the configured
// limit right at a window boundary (max requests at the end of one
// window, then immediately max requests again at the start of the
// next). Not fixed in v1 — documented, not silently ignored. Sliding
// window or token bucket are candidate v2 algorithms if this ever
// matters in practice.
//
// LRU eviction mid-window: an evicted entry looks identical to a
// never-seen key on the next `get()`, so an evicted-but-still-active
// client's window simply restarts — effectively a free burst. Real
// behavior given LRUCache is bounded-storage, not a hypothetical; see
// PROJECT_GUIDE.md Phase 5, tested explicitly there.
//
// requestsPerWindow == 0: a limiter configured with a zero limit denies
// every request, including the first one for a never-seen key. This is
// handled explicitly in `allow()` rather than falling through to the
// "fresh window" branch, which would otherwise allow exactly one
// request per window regardless of the configured limit.

namespace ThrottlePro {

/**
 * @brief Per-key fixed-window rate limiting state.
 */
struct RateLimitWindow {
    std::chrono::steady_clock::time_point windowStart;
    std::size_t count = 0;
};

/**
 * @brief Framework-agnostic fixed-window rate limiter.
 * @details Thread-safe: all access to the underlying LRU cache is
 * guarded by an internal mutex, since `CachePro::LRUCache` has no
 * built-in thread-safety of its own.
 */
class RateLimiter {
  public:
    /**
     * @brief Constructs a rate limiter with the given limit and window.
     * @param requestsPerWindow Maximum allowed requests per key, per window.
     * A value of 0 denies all requests.
     * @param windowDuration Length of the fixed window.
     * @param cacheCapacity Max distinct keys tracked before LRU eviction.
     * @details See PROJECT_GUIDE.md Phase 5 for the mid-window-eviction
     * edge case this implies (documented above the class as well).
     */
    RateLimiter(std::size_t requestsPerWindow, std::chrono::milliseconds windowDuration,
                std::size_t cacheCapacity);

    /**
     * @brief Records a request for `key` at time `now` and decides allow/deny.
     * @param key Rate-limit key (e.g. client IP).
     * @param now Current time. Defaults to `std::chrono::steady_clock::now()`;
     * overridable for deterministic unit tests.
     * @return `true` if the request is allowed, `false` if it should be rejected.
     */
    [[nodiscard]] bool
    allow(const std::string& key,
          std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

  private:
    std::size_t requestsPerWindow_;
    std::chrono::milliseconds windowDuration_;
    std::mutex mutex_;

    CachePro::LRUCache<std::string, RateLimitWindow> cache_;
};

} // namespace ThrottlePro

/// @brief Umbrella alias so this library's types are reachable as
/// `rain::RateLimiter`, alongside every other project library, while its
/// true namespace (and all internal diagnostics) remains `ThrottlePro`.
/// Reopens `rain` rather than aliasing it, since multiple libraries each
/// contribute their own names into the same `rain` namespace — an alias
/// (`namespace rain = ThrottlePro;`) can only ever bind to one target and
/// collides the moment a second library declares its own `rain` alias to
/// something else.
namespace rain {
using namespace ThrottlePro;
}
