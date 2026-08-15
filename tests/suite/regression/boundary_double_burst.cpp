// RateLimiter boundary double-burst regression test suite.
//
// Verifies the documented, accepted fixed-window tradeoff behaves
// exactly as specified: a client can burst up to ~2x the configured
// limit right at a window boundary (max requests at the end of one
// window, then immediately max requests again at the start of the
// next), and no more than that.

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies a full burst at the end of one window followed immediately
// by a full burst at the start of the next window is allowed in total.
static void boundary_allows_double_burst() {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    // Exhaust the limit right at the tail end of the window.
    auto windowEnd = now + std::chrono::milliseconds(999);
    CHK(limiter.allow("k1", windowEnd));
    CHK(limiter.allow("k1", windowEnd));
    CHK(limiter.allow("k1", windowEnd));
    CHK(!limiter.allow("k1", windowEnd));

    // Immediately exhaust the limit again at the start of the next window.
    // The window actually started at windowEnd (that's when the fresh
    // window was created above), so its boundary is windowEnd + duration,
    // not now + duration.
    auto nextWindowStart = windowEnd + std::chrono::milliseconds(1000);
    CHK(limiter.allow("k1", nextWindowStart));
    CHK(limiter.allow("k1", nextWindowStart));
    CHK(limiter.allow("k1", nextWindowStart));
}

// Verifies the burst is capped at exactly 2x the limit, not more.
static void boundary_burst_capped_at_2x() {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    auto windowEnd = now + std::chrono::milliseconds(999);
    CHK(limiter.allow("k1", windowEnd));
    CHK(limiter.allow("k1", windowEnd));
    CHK(limiter.allow("k1", windowEnd));

    auto nextWindowStart = windowEnd + std::chrono::milliseconds(1000);
    CHK(limiter.allow("k1", nextWindowStart));
    CHK(limiter.allow("k1", nextWindowStart));
    CHK(limiter.allow("k1", nextWindowStart));

    // A 7th request (2x limit + 1) within the second window must be denied.
    CHK(!limiter.allow("k1", nextWindowStart));
}

// Executes all boundary double-burst regression test cases.
static void run_tests() {
    RUN(boundary_allows_double_burst);
    RUN(boundary_burst_capped_at_2x);
}

REGISTER_TEST_SUITE();
