// RateLimiter window expiry test suite.
//
// Coverage:
// - A request after the window has fully elapsed starts a fresh window
// - A request exactly at the window boundary starts a fresh window
// - A request just before the boundary is still governed by the old window
// - The reset count after expiry is independent of the previous window's count

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies a request after the window has elapsed starts a fresh window.
static void expiry_after_full_window() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));

    auto later = now + std::chrono::milliseconds(1500);
    CHK(limiter.allow("k1", later));
}

// Verifies a request exactly at the window boundary counts as expired.
static void expiry_at_exact_boundary() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));

    auto boundary = now + std::chrono::milliseconds(1000);
    CHK(limiter.allow("k1", boundary));
}

// Verifies a request just before the boundary is still bound by the
// previous window's count, not treated as expired.
static void no_expiry_before_boundary() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));

    auto justBefore = now + std::chrono::milliseconds(999);
    CHK(!limiter.allow("k1", justBefore));
}

// Verifies the count resets to 1 (not accumulated) after expiry.
static void expiry_resets_count() {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));

    auto later = now + std::chrono::milliseconds(2000);
    CHK(limiter.allow("k1", later));
    CHK(limiter.allow("k1", later));
    CHK(!limiter.allow("k1", later));
}

// Executes all window expiry test cases.
static void run_tests() {
    RUN(expiry_after_full_window);
    RUN(expiry_at_exact_boundary);
    RUN(no_expiry_before_boundary);
    RUN(expiry_resets_count);
}

REGISTER_TEST_SUITE();
