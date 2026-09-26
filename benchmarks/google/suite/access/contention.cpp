// RateThrottle Concurrency Benchmark Suite — Contention
// Measures RateLimiter::allow() from the timed thread while a fixed
// pool of background threads hammers the SAME key concurrently -- the
// worst case for RateLimiter's design, since its mutex guards the whole
// cache, not sharded per key. This exercises realistic mutex contention
// rather than best-case uncontended latency.
//
// Covers:
// - allow() on a shared key, contended by background threads

#include <ThrottlePro/RateLimiter.h>

#include <benchmark/benchmark.h>

#include <atomic>
#include <thread>
#include <vector>

using namespace ThrottlePro;

namespace {
constexpr std::size_t kCapacity = 16;
constexpr std::size_t kRequestsPerWindow = 2'000'000;
// Far longer than this benchmark could ever run, so no thread's calls
// ever hit the deny or window-reset branches -- contention on the
// shared mutex is the only thing being measured, not branch behavior.
constexpr std::chrono::milliseconds kWindowDuration{24 * 60 * 60 * 1000};

// Background threads generating contention, in addition to the
// foreground (timed) thread that the benchmark loop itself drives.
constexpr unsigned kBackgroundThreads = 4;
} // namespace

// Measures allow() on a shared key while background threads contend
// for the same limiter's mutex.
static void AllowContention(benchmark::State& state) {
    RateLimiter limiter(kRequestsPerWindow, kWindowDuration, kCapacity);
    const std::string key = "key";

    std::atomic<bool> stop{false};
    std::vector<std::thread> workers;
    workers.reserve(kBackgroundThreads);

    for (unsigned i = 0; i < kBackgroundThreads; ++i) {
        workers.emplace_back([&] {
            while (!stop.load(std::memory_order_relaxed)) {
                benchmark::DoNotOptimize(limiter.allow(key));
            }
        });
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(limiter.allow(key));
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& worker : workers) {
        worker.join();
    }
}
BENCHMARK(AllowContention);
