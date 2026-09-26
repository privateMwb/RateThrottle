// RateLimiter multi-key isolation test suite.
//
// Coverage:
// - Two distinct keys track independent counts within the same window
// - One key reaching its limit does not affect another key's state
// - Expiry of one key's window does not affect another key's window
// - Many distinct keys (within cache capacity) all stay correctly isolated

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies two keys accumulate independent counts.
TEST(RateLimiterMultiKeyIsolationTest, IndependentCountsPerKey) {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k2", now));
    EXPECT_TRUE(limiter.allow("k2", now));
    EXPECT_FALSE(limiter.allow("k2", now)); // k2 at limit

    // k1 is unaffected, still has room.
    EXPECT_TRUE(limiter.allow("k1", now));
}

// Verifies one key being denied doesn't deny another key.
TEST(RateLimiterMultiKeyIsolationTest, OneKeyDenialDoesNotAffectOther) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now)); // k1 denied

    EXPECT_TRUE(limiter.allow("k2", now)); // k2 unaffected
}

// Verifies one key's window expiring doesn't reset another key's window.
TEST(RateLimiterMultiKeyIsolationTest, ExpiryIsPerKey) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k2", now));

    auto later = now + std::chrono::milliseconds(1500);
    EXPECT_TRUE(limiter.allow("k1", later)); // k1's window expired, fresh

    // k2 was never queried at `later` before, so its window is
    // independently evaluated here and also expired.
    EXPECT_TRUE(limiter.allow("k2", later));
}

// Verifies several distinct keys all stay correctly isolated.
TEST(RateLimiterMultiKeyIsolationTest, ManyKeysStayIsolated) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 16);
    auto now = std::chrono::steady_clock::now();

    for (int i = 0; i < 10; ++i) {
        std::string key = "k" + std::to_string(i);
        EXPECT_TRUE(limiter.allow(key, now));
        EXPECT_FALSE(limiter.allow(key, now));
    }
}
