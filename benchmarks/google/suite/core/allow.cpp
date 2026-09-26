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

#include <ThrottlePro/RateLimiter.h>

#include <benchmark/benchmark.h>

using namespace ThrottlePro;

namespace {
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};

// Far longer than any of these benchmarks could ever run, so windows
// that are meant to stay active never expire mid-run.
constexpr std::chrono::milliseconds kLongWindowDuration{24 * 60 * 60 * 1000};

// Comfortably above any single benchmark's iteration count, so the
// "new key" case never wraps around and reuses a key.
constexpr std::size_t kNewKeyCapacity = 1'200'000;

// Comfortably above any single benchmark's iteration count, so the
// limit is never reached in the "within window" case.
constexpr std::size_t kWithinWindowRequestsPerWindow = 2'000'000;

constexpr std::size_t kDenyRequestsPerWindow = 1;

constexpr std::chrono::milliseconds kResetWindowDuration{1};
} // namespace

// Measures allow() on a never-before-seen key, with room to spare.
static void AllowNewKey(benchmark::State& state) {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kNewKeyCapacity);
    std::size_t counter = 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow("key-" + std::to_string(counter)));
        ++counter;
    }
}
BENCHMARK(AllowNewKey);

// Measures allow() on an existing key with room left in its window.
static void AllowWithinWindow(benchmark::State& state) {
    RateLimiter limiter(kWithinWindowRequestsPerWindow, kLongWindowDuration, 16);
    const std::string key = "key";

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow(key));
    }
}
BENCHMARK(AllowWithinWindow);

// Measures allow() once the limit has already been reached.
static void AllowDeny(benchmark::State& state) {
    RateLimiter limiter(kDenyRequestsPerWindow, kLongWindowDuration, 16);
    const std::string key = "key";

    // Prime the limiter: consume the one allowed request before timing
    // starts, so every timed call below is a denial.
    (void)limiter.allow(key);

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow(key));
    }
}
BENCHMARK(AllowDeny);

// Measures allow() where the stored window has always just expired.
static void AllowWindowReset(benchmark::State& state) {
    RateLimiter limiter(kRequestsPerWindow, kResetWindowDuration, 16);
    const std::string key = "key";
    auto now = std::chrono::steady_clock::now();

    for (auto _ : state) {
        // Always exceeds kResetWindowDuration, guaranteeing the
        // "expired" branch every call.
        now += kResetWindowDuration + std::chrono::milliseconds(1);
        benchmark::DoNotOptimize(limiter.allow(key, now));
    }
}
BENCHMARK(AllowWindowReset);
