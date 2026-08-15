// RateThrottle Lifecycle Benchmark Suite — Construction
// Measures constructing (and, since it goes out of scope immediately,
// destructing) a RateLimiter sized for a representative capacity.
//
// Covers:
// - Constructing an empty RateLimiter

#include <support/framework.h>

using namespace ThrottlePro;

namespace {
constexpr std::size_t kCapacity = 100'000;
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};
} // namespace

// Measures constructing a RateLimiter.
static void bench_construction() {
    auto c = [&] { RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kCapacity); };

    BENCH_CUSTOM("construction", c);
}

// Executes all Lifecycle/construction benchmark cases.
static void run_benchmarks() {
    bench_construction();
}

REGISTER_BENCH_SUITE();
