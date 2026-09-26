// RateLimiter zero-limit test suite.
//
// Coverage:
// - A limiter constructed with requestsPerWindow == 0 denies the first
//   request for a never-seen key
// - It continues to deny across repeated calls for the same key
// - It denies for every distinct key, not just the first one seen
// - Denial holds even after the window would otherwise have expired

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies a zero-limit limiter denies the very first request.
TEST(RateLimiterZeroLimitTest, DeniesFirstRequest) {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_FALSE(limiter.allow("k1", now));
}

// Verifies a zero-limit limiter keeps denying the same key.
TEST(RateLimiterZeroLimitTest, DeniesRepeatedly) {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_FALSE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k1", now));
}

// Verifies a zero-limit limiter denies every distinct key.
TEST(RateLimiterZeroLimitTest, DeniesAllKeys) {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_FALSE(limiter.allow("k1", now));
    EXPECT_FALSE(limiter.allow("k2", now));
    EXPECT_FALSE(limiter.allow("k3", now));
}

// Verifies a zero-limit limiter still denies after the window elapses.
TEST(RateLimiterZeroLimitTest, DeniesAfterWindow) {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_FALSE(limiter.allow("k1", now));

    auto later = now + std::chrono::milliseconds(5000);
    EXPECT_FALSE(limiter.allow("k1", later));
}
