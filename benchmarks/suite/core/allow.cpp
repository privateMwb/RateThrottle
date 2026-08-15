// RateThrottle Core Benchmark Suite — allow()
// Measures RateLimiter::allow() across its four branches.
//
// Covers:
// - New key: never seen before (fresh window, cache_.put() insert)
// - Within window: existing key, count below limit (in-place ++count,
//   no cache_.put() re-insert needed since get() returns a live
//   pointer)
// - Deny: existing key, limit already reached, window still active
//   (denial deliberately skips the increment path)
// - Window reset: existing key whose window has just expired (same
//   code path as a first-ever key)

#include <support/framework.h>

using namespace ThrottlePro;

namespace {
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};

// Far longer than any of these benchmarks could ever run, so windows
// that are meant to stay active never expire mid-run.
constexpr std::chrono::milliseconds kLongWindowDuration{24 * 60 * 60 * 1000};

// 10K + 100K + 1M = 1,110,000 total allow() calls across one BENCH()
// run.
constexpr std::size_t kNewKeyCapacity = 1'200'000;

// Comfortably above the cumulative 1,110,000 calls in one BENCH() run,
// so the limit is never reached.
constexpr std::size_t kWithinWindowRequestsPerWindow = 2'000'000;

constexpr std::size_t kDenyRequestsPerWindow = 1;

constexpr std::chrono::milliseconds kResetWindowDuration{1};
} // namespace

// Measures allow() on a never-before-seen key, with room to spare.
static void bench_allow_new_key() {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kNewKeyCapacity);
    std::size_t counter = 0;

    auto c = [&] {
        (void)limiter.allow("key-" + std::to_string(counter));
        ++counter;
    };

    BENCH("allow() new key", c);
}

// Measures allow() on an existing key with room left in its window.
static void bench_allow_within_window() {
    RateLimiter limiter(kWithinWindowRequestsPerWindow, kLongWindowDuration, 16);
    const std::string key = "key";

    auto c = [&] { (void)limiter.allow(key); };

    BENCH("allow() within window", c);
}

// Measures allow() once the limit has already been reached.
static void bench_allow_deny() {
    RateLimiter limiter(kDenyRequestsPerWindow, kLongWindowDuration, 16);
    const std::string key = "key";

    // Prime the limiter: consume the one allowed request before timing
    // starts, so every timed call below is a denial.
    (void)limiter.allow(key);

    auto c = [&] { (void)limiter.allow(key); };

    BENCH("allow() deny", c);
}

// Measures allow() where the stored window has always just expired.
static void bench_allow_window_reset() {
    RateLimiter limiter(kRequestsPerWindow, kResetWindowDuration, 16);
    const std::string key = "key";
    auto now = std::chrono::steady_clock::now();

    auto c = [&] {
        // Always exceeds kResetWindowDuration, guaranteeing the
        // "expired" branch every call.
        now += kResetWindowDuration + std::chrono::milliseconds(1);
        (void)limiter.allow(key, now);
    };

    BENCH("allow() window reset", c);
}

// Executes all Core/allow benchmark cases.
static void run_benchmarks() {
    bench_allow_new_key();
    std::cout << "\n";

    bench_allow_within_window();
    std::cout << "\n";

    bench_allow_deny();
    std::cout << "\n";

    bench_allow_window_reset();
    std::cout << "\n";
}

REGISTER_BENCH_SUITE();
