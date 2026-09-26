// RateLimiter window expiry test suite.
//
// Coverage:
// - A request after the window has fully elapsed starts a fresh window
// - A request exactly at the window boundary starts a fresh window
// - A request just before the boundary is still governed by the old window
// - The reset count after expiry is independent of the previous window's count

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies a request after the window has elapsed starts a fresh window.
TEST(RateLimiterWindowExpiryTest, ExpiryAfterFullWindow) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));

    auto later = now + std::chrono::milliseconds(1500);
    EXPECT_TRUE(limiter.allow("k1", later));
}

// Verifies a request exactly at the window boundary counts as expired.
TEST(RateLimiterWindowExpiryTest, ExpiryAtExactBoundary) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));

    auto boundary = now + std::chrono::milliseconds(1000);
    EXPECT_TRUE(limiter.allow("k1", boundary));
}

// Verifies a request just before the boundary is still bound by the
// previous window's count, not treated as expired.
TEST(RateLimiterWindowExpiryTest, NoExpiryBeforeBoundary) {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));

    auto justBefore = now + std::chrono::milliseconds(999);
    EXPECT_FALSE(limiter.allow("k1", justBefore));
}

// Verifies the count resets to 1 (not accumulated) after expiry.
TEST(RateLimiterWindowExpiryTest, ExpiryResetsCount) {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));

    auto later = now + std::chrono::milliseconds(2000);
    EXPECT_TRUE(limiter.allow("k1", later));
    EXPECT_TRUE(limiter.allow("k1", later));
    EXPECT_FALSE(limiter.allow("k1", later));
}
