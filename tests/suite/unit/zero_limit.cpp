// RateLimiter zero-limit test suite.
//
// Coverage:
// - A limiter constructed with requestsPerWindow == 0 denies the first
//   request for a never-seen key
// - It continues to deny across repeated calls for the same key
// - It denies for every distinct key, not just the first one seen
// - Denial holds even after the window would otherwise have expired

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies a zero-limit limiter denies the very first request.
static void zero_limit_denies_first_request() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(!limiter.allow("k1", now));
}

// Verifies a zero-limit limiter keeps denying the same key.
static void zero_limit_denies_repeatedly() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(!limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));
}

// Verifies a zero-limit limiter denies every distinct key.
static void zero_limit_denies_all_keys() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(!limiter.allow("k1", now));
    CHK(!limiter.allow("k2", now));
    CHK(!limiter.allow("k3", now));
}

// Verifies a zero-limit limiter still denies after the window elapses.
static void zero_limit_denies_after_window() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(!limiter.allow("k1", now));

    auto later = now + std::chrono::milliseconds(5000);
    CHK(!limiter.allow("k1", later));
}

// Executes all zero-limit test cases.
static void run_tests() {
    RUN(zero_limit_denies_first_request);
    RUN(zero_limit_denies_repeatedly);
    RUN(zero_limit_denies_all_keys);
    RUN(zero_limit_denies_after_window);
}

REGISTER_TEST_SUITE();
