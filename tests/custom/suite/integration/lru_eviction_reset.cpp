// RateLimiter + LRUCache eviction integration test suite.
//
// Coverage:
// - A key evicted mid-window (due to cache capacity pressure from other
//   keys) gets a free burst: the next allow() treats it as never-seen
// - Keys that stay within capacity are unaffected by eviction pressure
//   on other keys
// - The evicted key's fresh window behaves like any other fresh window
//   (count resets to 1, subsequent denial still applies at the limit)

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies an evicted key's window resets rather than carrying over
// its prior count.
static void eviction_resets_window() {
    // Capacity 2: inserting a 3rd distinct key evicts the LRU one.
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now)); // k1 at limit

    CHK(limiter.allow("k2", now));
    CHK(limiter.allow("k3", now)); // evicts k1 (LRU)

    // k1 looks never-seen again: fresh window, allowed.
    CHK(limiter.allow("k1", now));
}

// Verifies keys that remain within capacity are unaffected by eviction
// pressure caused by other keys.
static void eviction_does_not_affect_resident_keys() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now)); // k1 at limit

    CHK(limiter.allow("k2", now));
    CHK(!limiter.allow("k2", now)); // k2 at limit, no eviction yet (capacity 2)

    // k1 and k2 both still resident and both still denied.
    CHK(!limiter.allow("k1", now));
    CHK(!limiter.allow("k2", now));
}

// Verifies a post-eviction fresh window still enforces the limit
// correctly (not just the first allow() after eviction).
static void eviction_fresh_window_still_enforces_limit() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k2", now));
    CHK(limiter.allow("k3", now)); // evicts k1

    CHK(limiter.allow("k1", now));  // fresh window, allowed
    CHK(!limiter.allow("k1", now)); // limit reached again
}

// Executes all eviction integration test cases.
static void run_tests() {
    RUN(eviction_resets_window);
    RUN(eviction_does_not_affect_resident_keys);
    RUN(eviction_fresh_window_still_enforces_limit);
}

REGISTER_TEST_SUITE();
