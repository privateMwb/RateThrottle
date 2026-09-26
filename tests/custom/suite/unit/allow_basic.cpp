// RateLimiter allow() test suite.
//
// Coverage:
// - First request for a never-seen key is allowed, starting a fresh window
// - Requests within the configured limit are allowed
// - The request at the limit is allowed, the next one is denied
// - Denial does not mutate the stored window state

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies the first request for a new key is allowed.
static void allow_first_request() {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
}

// Verifies requests below the limit are all allowed.
static void allow_within_limit() {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k1", now));
}

// Verifies the request that reaches the limit is still allowed, and the
// next one within the same window is denied.
static void allow_denies_over_limit() {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));
}

// Verifies a denied request doesn't corrupt state: still denied on the
// next call within the same window.
static void deny_does_not_mutate_state() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now));
}

// Executes all allow() basic test cases.
static void run_tests() {
    RUN(allow_first_request);
    RUN(allow_within_limit);
    RUN(allow_denies_over_limit);
    RUN(deny_does_not_mutate_state);
}

REGISTER_TEST_SUITE();
