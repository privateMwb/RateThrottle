// Basic RateLimiter usage.
//
// Demonstrates:
// - Constructing a limiter with a fixed limit, window, and cache capacity
// - Allowing requests with allow(), up to the configured limit
// - A request denied once the limit is reached within the window
// - Passing an explicit time point instead of relying on the default now()

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // A RateLimiter is constructed with the max requests allowed per
    // window, the window's duration, and the max distinct keys the
    // underlying cache will track before evicting the least recently
    // used one.
    setTitle("Construction");

    RateLimiter limiter(3, std::chrono::milliseconds(1000), 100);

    std::cout << "limiter ready: 3 requests per 1000ms window\n\n";

    // allow() records a request for a key and returns whether it's
    // allowed. The first calls for a new key succeed until the limit
    // is reached.
    setTitle("Allowing requests");

    auto now = std::chrono::steady_clock::now();

    std::cout << "allow(\"client-a\") #1: " << limiter.allow("client-a", now) << "\n";
    std::cout << "allow(\"client-a\") #2: " << limiter.allow("client-a", now) << "\n";
    std::cout << "allow(\"client-a\") #3: " << limiter.allow("client-a", now) << "\n\n";

    // Once a key has reached its limit within the current window,
    // further requests are denied.
    setTitle("Denial at the limit");

    bool allowed = limiter.allow("client-a", now);
    std::cout << "allow(\"client-a\") #4: " << allowed << " (limit reached)\n\n";

    // allow() accepts an explicit time point instead of relying on the
    // default steady_clock::now(), which is what makes deterministic
    // testing of window expiry possible.
    setTitle("Explicit time points");

    auto nextWindow = now + std::chrono::milliseconds(1000);
    std::cout << "allow(\"client-a\") after window elapses: "
              << limiter.allow("client-a", nextWindow) << "\n";
}

REGISTER_EXAMPLE_SUITE();
