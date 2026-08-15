// RateLimiter concurrency test suite.
//
// Coverage:
// - Concurrent allow() calls for the same key from multiple threads
//   never allow more than requestsPerWindow requests total
// - Concurrent allow() calls across multiple distinct keys stay correct
//   per key, with no cross-key interference
// - No crashes or data races under concurrent access (internal mutex
//   covers the LRUCache, which has no thread-safety of its own)

#include <atomic>
#include <support/framework.h>
#include <thread>
#include <vector>

using namespace ThrottlePro;

// Verifies concurrent callers hitting the same key never exceed the
// configured limit in total allowed requests.
static void concurrent_same_key_respects_limit() {
    constexpr std::size_t limit = 50;
    constexpr int threadCount = 8;
    constexpr int callsPerThread = 20;

    RateLimiter limiter(limit, std::chrono::milliseconds(60000), 8);
    auto now = std::chrono::steady_clock::now();

    std::atomic<int> allowedCount{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < callsPerThread; ++i) {
                if (limiter.allow("shared", now)) {
                    ++allowedCount;
                }
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    CHK(allowedCount.load() == static_cast<int>(limit));
}

// Verifies concurrent callers hitting distinct keys stay correctly
// isolated, with each key independently respecting its own limit.
static void concurrent_distinct_keys_stay_isolated() {
    constexpr std::size_t limit = 10;
    constexpr int keyCount = 6;
    constexpr int callsPerKey = 30;

    RateLimiter limiter(limit, std::chrono::milliseconds(60000), 16);
    auto now = std::chrono::steady_clock::now();

    std::vector<std::atomic<int>> allowedPerKey(keyCount);
    for (auto& counter : allowedPerKey) {
        counter.store(0);
    }

    std::vector<std::thread> threads;
    for (int k = 0; k < keyCount; ++k) {
        threads.emplace_back([&, k]() {
            std::string key = "key" + std::to_string(k);
            for (int i = 0; i < callsPerKey; ++i) {
                if (limiter.allow(key, now)) {
                    ++allowedPerKey[k];
                }
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    for (int k = 0; k < keyCount; ++k) {
        CHK(allowedPerKey[k].load() == static_cast<int>(limit));
    }
}

// Executes all concurrency test cases.
static void run_tests() {
    RUN(concurrent_same_key_respects_limit);
    RUN(concurrent_distinct_keys_stay_isolated);
}

REGISTER_TEST_SUITE();
