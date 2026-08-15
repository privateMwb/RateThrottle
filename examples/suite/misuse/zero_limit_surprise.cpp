// The zero-limit surprise.
//
// Demonstrates:
// - requestsPerWindow == 0 does NOT mean "no limit" -- it means
//   "deny everything", which is easy to get backwards
// - The mistake: assuming 0 is a sentinel for "unlimited", the way it
//   sometimes is in other APIs
// - The correct pattern for an actually-unlimited limiter

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // The mistake: reaching for 0 to mean "don't throttle this route".
    // In RateLimiter, 0 means the opposite -- every request is denied,
    // including the very first one for a key that's never been seen.
    setTitle("The mistake: 0 does not mean unlimited");

    RateLimiter mistaken(/*requestsPerWindow=*/0, std::chrono::milliseconds(1000), 100);
    auto now = std::chrono::steady_clock::now();

    std::cout << "allow(\"client-a\"): " << mistaken.allow("client-a", now)
              << " -- every request denied, not unlimited\n\n";

    // RateLimiter has no built-in "unlimited" mode -- there's no
    // sentinel value that disables throttling. If a route genuinely
    // shouldn't be throttled, don't route it through allow() at all.
    setTitle("There is no unlimited sentinel");

    std::cout << "correct approach: skip calling allow() entirely for that route,\n"
              << "or use a limit high enough that it's effectively never reached.\n\n";

    // A large limit is the closest equivalent to "unlimited" this API
    // supports directly.
    setTitle("The practical workaround");

    RateLimiter effectivelyUnlimited(SIZE_MAX, std::chrono::milliseconds(1000), 100);
    std::cout << "allow(\"client-a\"): " << effectivelyUnlimited.allow("client-a", now) << "\n";
}

REGISTER_EXAMPLE_SUITE();
