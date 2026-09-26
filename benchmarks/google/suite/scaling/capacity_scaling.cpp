// RateThrottle Scaling Benchmark Suite — Capacity Scaling
// Measures whether RateLimiter::allow()'s per-call cost changes as
// cacheCapacity grows, independent of call volume.
//
// DEVIATION FROM LRUCache'S Scaling CATEGORY: LRUCache's own Scaling
// benchmarks (reallocation.cpp, reserve.cpp, shrink_to_fit.cpp) time an
// explicit resize-family operation. CachePro::LRUCache has no
// equivalent public API to resize after construction -- capacity is
// fixed at construction time. So instead, this file constructs two
// limiters at very different fixed capacities and runs the same
// eviction-pressure workload (see Scaling/eviction_pressure.cpp)
// against each, to see whether steady-state per-call cost holds
// constant as capacity itself scales up. Flagging this as a deliberate
// reinterpretation, not a literal port of the LRUCache pattern.
//
// Covers:
// - allow() steady-state eviction cost at a small capacity
// - allow() steady-state eviction cost at a large capacity

#include <ThrottlePro/RateLimiter.h>

#include <benchmark/benchmark.h>

using namespace ThrottlePro;

namespace {
constexpr std::size_t kSmallCapacity = 100;
constexpr std::size_t kLargeCapacity = 1'000'000;
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};
} // namespace

// Measures steady-state allow() cost with a small cache capacity.
static void AllowSmallCapacity(benchmark::State& state) {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kSmallCapacity);
    std::size_t counter = 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow("key-" + std::to_string(counter)));
        ++counter;
    }
}
BENCHMARK(AllowSmallCapacity);

// Measures steady-state allow() cost with a large cache capacity.
static void AllowLargeCapacity(benchmark::State& state) {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kLargeCapacity);
    std::size_t counter = 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow("key-" + std::to_string(counter)));
        ++counter;
    }
}
BENCHMARK(AllowLargeCapacity);
