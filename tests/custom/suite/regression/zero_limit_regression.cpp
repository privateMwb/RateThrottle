// RateLimiter zero-limit regression test suite.
//
// Bug: previously, a limiter constructed with requestsPerWindow == 0
// would still allow exactly one request per window per key, because
// the very first allow() for a never-seen key always took the "fresh
// window" branch (existing == nullptr), which unconditionally set
// count = 1 and returned true without ever checking the configured
// limit. Fixed by checking requestsPerWindow_ == 0 up front in
// allow(), before the fresh-window branch runs.

#include <support/framework.h>

using namespace ThrottlePro;

// Verifies the exact bug scenario: the very first allow() call ever
// made against a zero-limit limiter, for a key that has never been
// seen before, must be denied rather than silently allowed.
static void zero_limit_first_ever_call_denied() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    // This is the exact path the bug took: existing == nullptr on a
    // never-seen key used to fall through to the fresh-window branch
    // and return true regardless of requestsPerWindow_.
    CHK(!limiter.allow("never-seen-key", now));
}

// Verifies the fix doesn't just deny the first call but holds for the
// first call of every subsequently-seen key too.
static void zero_limit_first_call_denied_for_every_key() {
    RateLimiter limiter(0, std::chrono::milliseconds(1000), 8);
    auto now = std::chrono::steady_clock::now();

    CHK(!limiter.allow("key-a", now));
    CHK(!limiter.allow("key-b", now));
    CHK(!limiter.allow("key-c", now));
}

// Executes all zero-limit regression test cases.
static void run_tests() {
    RUN(zero_limit_first_ever_call_denied);
    RUN(zero_limit_first_call_denied_for_every_key);
}

REGISTER_TEST_SUITE();
