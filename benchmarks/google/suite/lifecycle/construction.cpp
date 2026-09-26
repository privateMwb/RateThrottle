// RateThrottle Lifecycle Benchmark Suite — Construction
// Measures constructing (and, since it goes out of scope immediately,
// destructing) a RateLimiter sized for a representative capacity.
//
// Covers:
// - Constructing an empty RateLimiter

#include <ThrottlePro/RateLimiter.h>

#include <benchmark/benchmark.h>

using namespace ThrottlePro;

namespace {
constexpr std::size_t kCapacity = 100'000;
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};
} // namespace

// Measures constructing a RateLimiter.
static void Construction(benchmark::State& state) {
    for (auto _ : state) {
        RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kCapacity);
        benchmark::DoNotOptimize(limiter);
    }
}
BENCHMARK(Construction);
