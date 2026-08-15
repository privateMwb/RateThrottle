/**
 * @file RateLimiter.cpp
 * @brief RateLimiter implementation.
 *
 * Contains the implementation of ThrottlePro::RateLimiter's member
 * functions.
 */

// ============================================================
// Implementation for ThrottlePro::RateLimiter.
// ============================================================
//
//  Sections:
//   1. Constructor
//   2. Allow/Deny Decision
//
// ============================================================

// clang-format off
#include <ThrottlePro/RateLimiter.h> // ThrottlePro::RateLimiter, ThrottlePro::RateLimitWindow
// clang-format on

namespace ThrottlePro {

// ============================================================
//  Section 1 — Constructor
// ============================================================

RateLimiter::RateLimiter(std::size_t requestsPerWindow, std::chrono::milliseconds windowDuration,
                         std::size_t cacheCapacity)
    : requestsPerWindow_(requestsPerWindow), windowDuration_(windowDuration),
      cache_(cacheCapacity) {}

// ============================================================
//  Section 2 — Allow/Deny Decision
// ============================================================

bool RateLimiter::allow(const std::string& key, std::chrono::steady_clock::time_point now) {
    std::lock_guard<std::mutex> lock(mutex_);

    // A limit of 0 denies every request, including the first one for a
    // never-seen key. Handled up front so it doesn't fall through to the
    // "fresh window" branch below, which would otherwise unconditionally
    // allow one request per window regardless of requestsPerWindow_.
    if (requestsPerWindow_ == 0) {
        return false;
    }

    RateLimitWindow* existing = cache_.get(key);

    // No entry yet, or the existing entry's window has expired: start a
    // fresh window for this key. This is also the LRU-eviction-mid-window
    // path — an evicted entry looks identical to "no entry yet" here (see
    // header comment / PROJECT_GUIDE.md Phase 5).
    if (existing == nullptr || (now - existing->windowStart) >= windowDuration_) {
        cache_.put(key, RateLimitWindow{now, 1});
        return true;
    }

    if (existing->count < requestsPerWindow_) {
        // get() returns a live pointer into the cache, so mutating it in
        // place is sufficient — no need to put() the window back.
        ++existing->count;
        return true;
    }

    // Window still active and limit already reached: deny without
    // mutating the stored window, so the count doesn't run away past
    // the limit.
    return false;
}

} // namespace ThrottlePro
