// RateThrottle Scaling Benchmark Suite — Eviction Pressure
// Measures RateLimiter::allow() with a deliberately small cacheCapacity
// relative to the number of distinct keys requested, so cache_.put()
// evicts the LRU tail on nearly every call once capacity is first
// reached -- the rate-limiter analog of LRUCache's own steady-state
// eviction benchmark (push_back.cpp).
//
// Covers:
// - allow() on a never-before-seen key, cache already at capacity
//   (steady-state LRU eviction on every call)

#include <ThrottlePro/RateLimiter.h>

#include <benchmark/benchmark.h>

using namespace ThrottlePro;

namespace {
// Deliberately far below typical benchmark iteration counts, so
// eviction kicks in almost immediately and stays steady-state for the
// rest of the run.
constexpr std::size_t kCapacity = 1'000;
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};
} // namespace

// Measures allow() on a new key once the cache is already at capacity.
static void AllowEvictionPressure(benchmark::State& state) {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kCapacity);
    std::size_t counter = 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow("key-" + std::to_string(counter)));
        ++counter;
    }
}
BENCHMARK(AllowEvictionPressure);
