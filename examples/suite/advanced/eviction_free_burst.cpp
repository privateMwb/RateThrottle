// LRU eviction and the free-burst effect.
//
// Demonstrates:
// - RateLimiter's cache capacity is a bound on distinct tracked keys,
//   not a bound on correctness
// - A key evicted mid-window (because other keys pushed it out) looks
//   identical to a never-seen key on its next request
// - Why this matters when choosing cacheCapacity relative to the
//   number of distinct clients expected

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // A small cache capacity means a busy client population can push
    // each other out of the LRU cache mid-window.
    setTitle("A key reaches its limit");

    RateLimiter limiter(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    std::cout << "client-a request #1: " << limiter.allow("client-a", now) << "\n";
    std::cout << "client-a request #2: " << limiter.allow("client-a", now) << " (denied)\n\n";

    // Two more distinct keys arrive. With cache capacity 2, the second
    // one evicts client-a's entry -- the least recently used one.
    setTitle("Other keys evict it from the cache");

    std::cout << "client-b request #1: " << limiter.allow("client-b", now) << "\n";
    std::cout << "client-c request #1: " << limiter.allow("client-c", now)
              << " (evicts client-a)\n\n";

    // client-a's next request looks like a brand-new key: a fresh
    // window, allowed, even though its real window hadn't expired.
    setTitle("The evicted key gets a free burst");

    std::cout << "client-a request #3: " << limiter.allow("client-a", now)
              << " (looks like a new key -- limit effectively reset)\n";
}

REGISTER_EXAMPLE_SUITE();
