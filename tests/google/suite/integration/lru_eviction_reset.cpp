// RateLimiter + LRUCache eviction integration test suite.
//
// Coverage:
// - A key evicted mid-window (due to cache capacity pressure from other
//   keys) gets a free burst: the next allow() treats it as never-seen
// - Keys that stay within capacity are unaffected by eviction pressure
//   on other keys
// - The evicted key's fresh window behaves like any other fresh window
//   (count resets to 1, subsequent denial still applies at the limit)

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies an evicted key's window resets rather than carrying over
// its prior count.
TEST(RateLimiterLruEvictionResetTest, EvictionResetsWindow) {
    // Capacity 2: inserting a 3rd distinct key evicts the LRU one.
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now)); // k1 at limit

    EXPECT_TRUE(limiter.allow("k2", now));
    EXPECT_TRUE(limiter.allow("k3", now)); // evicts k1 (LRU)

    // k1 looks never-seen again: fresh window, allowed.
    EXPECT_TRUE(limiter.allow("k1", now));
}

// Verifies keys that remain within capacity are unaffected by eviction
// pressure caused by other keys.
TEST(RateLimiterLruEvictionResetTest, EvictionDoesNotAffectResidentKeys) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now)); // k1 at limit

    EXPECT_TRUE(limiter.allow("k2", now));
    EXPECT_FALSE(limiter.allow("k2", now)); // k2 at limit, no eviction yet (capacity 2)

    // k1 and k2 both still resident and both still denied.
    EXPECT_FALSE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k2", now));
}

// Verifies a post-eviction fresh window still enforces the limit
// correctly (not just the first allow() after eviction).
TEST(RateLimiterLruEvictionResetTest, EvictionFreshWindowStillEnforcesLimit) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k2", now));
    EXPECT_TRUE(limiter.allow("k3", now)); // evicts k1

    EXPECT_TRUE(limiter.allow("k1", now));  // fresh window, allowed
    EXPECT_FALSE(limiter.allow("k1", now)); // limit reached again
}
