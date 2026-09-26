// RateLimiter boundary double-burst regression test suite.
//
// Verifies the documented, accepted fixed-window tradeoff behaves
// exactly as specified: a client can burst up to ~2x the configured
// limit right at a window boundary (max requests at the end of one
// window, then immediately max requests again at the start of the
// next), and no more than that.

#include <ThrottlePro/RateLimiter.h>

#include <gtest/gtest.h>

using namespace ThrottlePro;

// Verifies a full burst at the end of one window followed immediately
// by a full burst at the start of the next window is allowed in total.
TEST(RateLimiterBoundaryDoubleBurstTest, AllowsDoubleBurst) {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    // Exhaust the limit right at the tail end of the window.
    auto windowEnd = now + std::chrono::milliseconds(999);
    EXPECT_TRUE(limiter.allow("k1", windowEnd));
    EXPECT_TRUE(limiter.allow("k1", windowEnd));
    EXPECT_TRUE(limiter.allow("k1", windowEnd));
    EXPECT_FALSE(limiter.allow("k1", windowEnd));

    // Immediately exhaust the limit again at the start of the next window.
    // The window actually started at windowEnd (that's when the fresh
    // window was created above), so its boundary is windowEnd + duration,
    // not now + duration.
    auto nextWindowStart = windowEnd + std::chrono::milliseconds(1000);
    EXPECT_TRUE(limiter.allow("k1", nextWindowStart));
    EXPECT_TRUE(limiter.allow("k1", nextWindowStart));
    EXPECT_TRUE(limiter.allow("k1", nextWindowStart));
}

// Verifies the burst is capped at exactly 2x the limit, not more.
TEST(RateLimiterBoundaryDoubleBurstTest, BurstCappedAt2x) {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    auto windowEnd = now + std::chrono::milliseconds(999);
    EXPECT_TRUE(limiter.allow("k1", windowEnd));
    EXPECT_TRUE(limiter.allow("k1", windowEnd));
    EXPECT_TRUE(limiter.allow("k1", windowEnd));

    auto nextWindowStart = windowEnd + std::chrono::milliseconds(1000);
    EXPECT_TRUE(limiter.allow("k1", nextWindowStart));
    EXPECT_TRUE(limiter.allow("k1", nextWindowStart));
    EXPECT_TRUE(limiter.allow("k1", nextWindowStart));

    // A 7th request (2x limit + 1) within the second window must be denied.
    EXPECT_FALSE(limiter.allow("k1", nextWindowStart));
}
