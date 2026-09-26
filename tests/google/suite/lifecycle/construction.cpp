// RateLimiter construction test suite.
//
// Coverage:
// - Normal construction with valid limit/window/capacity works and the
//   limiter is immediately usable
// - Zero requestsPerWindow constructs cleanly (behavior covered separately
//   in the zero-limit unit suite)
// - Zero windowDuration constructs cleanly and is immediately usable
// - Zero cacheCapacity throws, since the underlying LRUCache rejects a
//   zero capacity outright

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies normal construction produces a usable limiter.
TEST(RateLimiterConstructionTest, ConstructValidParams) {
    RateLimiter limiter(5, std::chrono::milliseconds(1000), 16);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
}

// Verifies construction with a zero limit doesn't throw or misbehave
// structurally; functional behavior is covered in the zero-limit suite.
TEST(RateLimiterConstructionTest, ConstructZeroLimit) {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_FALSE(limiter.allow("k1", now));
}

// Verifies construction with a zero window duration is usable: every
// call effectively starts a fresh window immediately.
TEST(RateLimiterConstructionTest, ConstructZeroWindow) {
    RateLimiter limiter(1, std::chrono::milliseconds(0), 8);
    auto now = std::chrono::steady_clock::now();

    EXPECT_TRUE(limiter.allow("k1", now));
    EXPECT_TRUE(limiter.allow("k1", now));
}

// Verifies construction with a zero cache capacity throws, since the
// underlying LRUCache rejects a zero capacity outright.
TEST(RateLimiterConstructionTest, ConstructZeroCapacity) {
    EXPECT_ANY_THROW({ RateLimiter limiter(1, std::chrono::milliseconds(1000), 0); });
}
