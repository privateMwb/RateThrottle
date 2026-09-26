// RateLimiter allow() test suite.
//
// Coverage:
// - First request for a never-seen key is allowed, starting a fresh window
// - Requests within the configured limit are allowed
// - The request at the limit is allowed, the next one is denied
// - Denial does not mutate the stored window state

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies the first request for a new key is allowed.
TEST(RateLimiterAllowBasicTest, FirstRequest) {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
}

// Verifies requests below the limit are all allowed.
TEST(RateLimiterAllowBasicTest, WithinLimit) {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k1", now));
}

// Verifies the request that reaches the limit is still allowed, and the
// next one within the same window is denied.
TEST(RateLimiterAllowBasicTest, DeniesOverLimit) {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));
}

// Verifies a denied request doesn't corrupt state: still denied on the
// next call within the same window.
TEST(RateLimiterAllowBasicTest, DenyDoesNotMutateState) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));
}
