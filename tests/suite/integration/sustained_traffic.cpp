// RateLimiter sustained traffic test suite.
//
// Coverage:
// - A realistic mixed sequence: burst, denial, window rollover, burst
//   again, repeated across several consecutive windows
// - Correct allow/deny pattern holds consistently window over window,
//   not just for the first one
// - Interleaved traffic from multiple keys across multiple windows stays
//   correct for each key independently

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies a single key's allow/deny pattern is correct across several
// consecutive windows.
static void sustained_traffic_single_key() {
    RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    for (int window = 0; window < 5; ++window) {
        auto windowStart = now + std::chrono::milliseconds(window * 1000);

        CHK(limiter.allow("k1", windowStart));
        CHK(limiter.allow("k1", windowStart));
        CHK(limiter.allow("k1", windowStart));
        CHK(!limiter.allow("k1", windowStart));
    }
}

// Verifies interleaved multi-key traffic across several windows stays
// correct for each key independently.
static void sustained_traffic_multiple_keys() {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    for (int window = 0; window < 3; ++window) {
        auto windowStart = now + std::chrono::milliseconds(window * 1000);

        CHK(limiter.allow("k1", windowStart));
        CHK(limiter.allow("k2", windowStart));
        CHK(limiter.allow("k1", windowStart));
        CHK(limiter.allow("k2", windowStart));

        CHK(!limiter.allow("k1", windowStart));
        CHK(!limiter.allow("k2", windowStart));
    }
}

// Verifies traffic that doesn't align to window boundaries (requests
// arriving at varying offsets within each window) is still handled
// correctly.
static void sustained_traffic_uneven_timing() {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k1", now + std::chrono::milliseconds(300)));
    CHK(!limiter.allow("k1", now + std::chrono::milliseconds(700)));

    // Next window starts relative to the original window start, not the
    // most recent call.
    CHK(limiter.allow("k1", now + std::chrono::milliseconds(1000)));
    CHK(limiter.allow("k1", now + std::chrono::milliseconds(1200)));
    CHK(!limiter.allow("k1", now + std::chrono::milliseconds(1900)));
}

// Executes all sustained traffic test cases.
static void run_tests() {
    RUN(sustained_traffic_single_key);
    RUN(sustained_traffic_multiple_keys);
    RUN(sustained_traffic_uneven_timing);
}

REGISTER_TEST_SUITE();
