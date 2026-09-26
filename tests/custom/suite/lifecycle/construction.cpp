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

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies normal construction produces a usable limiter.
static void construct_valid_params() {
    RateLimiter limiter(5, std::chrono::milliseconds(1000), 16);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
}

// Verifies construction with a zero limit doesn't throw or misbehave
// structurally; functional behavior is covered in zero_limit.cpp.
static void construct_zero_limit() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(!limiter.allow("k1", now));
}

// Verifies construction with a zero window duration is usable: every
// call effectively starts a fresh window immediately.
static void construct_zero_window() {
    RateLimiter limiter(1, std::chrono::milliseconds(0), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k1", now));
}

// Verifies construction with a zero cache capacity throws, since the
// underlying LRUCache rejects a zero capacity outright.
static void construct_zero_capacity() {
    bool threw = false;
    try {
        RateLimiter limiter(1, std::chrono::milliseconds(1000), 0);
    } catch (...) {
        threw = true;
    }

    CHK(threw);
}

// Executes all construction test cases.
static void run_tests() {
    RUN(construct_valid_params);
    RUN(construct_zero_limit);
    RUN(construct_zero_window);
    RUN(construct_zero_capacity);
}

REGISTER_TEST_SUITE();
