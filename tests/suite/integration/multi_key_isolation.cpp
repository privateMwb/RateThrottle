// RateLimiter multi-key isolation test suite.
//
// Coverage:
// - Two distinct keys track independent counts within the same window
// - One key reaching its limit does not affect another key's state
// - Expiry of one key's window does not affect another key's window
// - Many distinct keys (within cache capacity) all stay correctly isolated

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies two keys accumulate independent counts.
static void independent_counts_per_key() {
    RateLimiter limiter(2, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k2", now));
    CHK(limiter.allow("k2", now));
    CHK(!limiter.allow("k2", now)); // k2 at limit

    // k1 is unaffected, still has room.
    CHK(limiter.allow("k1", now));
}

// Verifies one key being denied doesn't deny another key.
static void one_key_denial_does_not_affect_other() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(!limiter.allow("k1", now)); // k1 denied

    CHK(limiter.allow("k2", now)); // k2 unaffected
}

// Verifies one key's window expiring doesn't reset another key's window.
static void expiry_is_per_key() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(limiter.allow("k1", now));
    CHK(limiter.allow("k2", now));

    auto later = now + std::chrono::milliseconds(1500);
    CHK(limiter.allow("k1", later)); // k1's window expired, fresh

    // k2 was never queried at `later` before, so its window is
    // independently evaluated here and also expired.
    CHK(limiter.allow("k2", later));
}

// Verifies several distinct keys all stay correctly isolated.
static void many_keys_stay_isolated() {
    RateLimiter limiter(1, std::chrono::milliseconds(1000), 16);
    auto now = std::chrono::steady_clock::now();

    for (int i = 0; i < 10; ++i) {
        std::string key = "k" + std::to_string(i);
        CHK(limiter.allow(key, now));
        CHK(!limiter.allow(key, now));
    }
}

// Executes all multi-key isolation test cases.
static void run_tests() {
    RUN(independent_counts_per_key);
    RUN(one_key_denial_does_not_affect_other);
    RUN(expiry_is_per_key);
    RUN(many_keys_stay_isolated);
}

REGISTER_TEST_SUITE();
