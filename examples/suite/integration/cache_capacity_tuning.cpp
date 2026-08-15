// Cache capacity tuning.
//
// Demonstrates:
// - cacheCapacity should be sized relative to the number of distinct
//   keys expected concurrently within a window, not picked arbitrarily
// - The visible consequence of undersizing it: legitimate clients get
//   a free burst instead of being correctly throttled (see also
//   eviction_free_burst.cpp)
// - Sizing it generously enough that eviction doesn't happen under
//   expected load

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // With 5 active clients but a cache capacity of only 2, most
    // clients' state gets evicted before their window naturally expires.
    setTitle("Undersized capacity for the client population");

    RateLimiter undersized(1, std::chrono::milliseconds(1000), 2);
    auto now = std::chrono::steady_clock::now();

    std::vector<std::string> clients = {"c1", "c2", "c3", "c4", "c5"};

    for (const auto& client : clients) {
        std::cout << client << " request #1: " << undersized.allow(client, now) << "\n";
    }
    std::cout << "\n";

    // Re-querying c1 -- evicted long ago by c2 through c5 -- shows it
    // gets a free burst instead of a correct denial.
    setTitle("The consequence: early clients get a free burst");

    std::cout << "c1 request #2: " << undersized.allow("c1", now)
              << " (should be denied, isn't)\n\n";

    // Sizing capacity to comfortably exceed the expected number of
    // concurrent distinct keys avoids this: every client's state
    // survives for its full window.
    setTitle("Correctly sized capacity");

    RateLimiter sized(1, std::chrono::milliseconds(1000), 100);

    for (const auto& client : clients) {
        (void)sized.allow(client, now);
    }
    std::cout << "c1 request #2: " << sized.allow("c1", now)
              << " (correctly denied, no eviction)\n";
}

REGISTER_EXAMPLE_SUITE();
