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

#include <support/framework.h>

using namespace ThrottlePro;

namespace {
// Deliberately far below the cumulative 1,110,000 calls in one BENCH()
// run, so eviction kicks in almost immediately and stays steady-state
// for the rest of the run.
constexpr std::size_t kCapacity = 1'000;
constexpr std::size_t kRequestsPerWindow = 100;
constexpr std::chrono::milliseconds kWindowDuration{60'000};
} // namespace

// Measures allow() on a new key once the cache is already at capacity.
static void bench_allow_eviction_pressure() {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kCapacity);
    std::size_t counter = 0;

    auto c = [&] {
        (void)limiter.allow("key-" + std::to_string(counter));
        ++counter;
    };

    BENCH("allow() eviction pressure", c);
}

// Executes all Scaling/eviction_pressure benchmark cases.
static void run_benchmarks() {
    bench_allow_eviction_pressure();
}

REGISTER_BENCH_SUITE();
