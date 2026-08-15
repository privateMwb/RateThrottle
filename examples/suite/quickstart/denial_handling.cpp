// Handling denied requests.
//
// Demonstrates:
// - Branching on allow()'s return value
// - A denied request does not disturb the key's existing window state
// - A typical "reject the caller" response pattern
// - Repeated denials within the same window stay consistent

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // allow() is [[nodiscard]] — the caller is expected to branch on
    // the result rather than ignore it.
    setTitle("Branching on the result");

    RateLimiter limiter(2, std::chrono::milliseconds(1000), 100);
    auto now = std::chrono::steady_clock::now();

    if (limiter.allow("client-a", now)) {
        std::cout << "request #1: allowed\n";
    } else {
        std::cout << "request #1: denied\n";
    }

    if (limiter.allow("client-a", now)) {
        std::cout << "request #2: allowed\n";
    } else {
        std::cout << "request #2: denied\n";
    }

    // Once the limit is reached, the typical response is to reject the
    // caller (e.g. an HTTP 429) rather than silently drop or retry.
    setTitle("Rejecting once the limit is reached");

    if (!limiter.allow("client-a", now)) {
        std::cout << "request #3: denied -- respond with 429 Too Many Requests\n";
    }

    // Denials are consistent: they don't corrupt state, so repeated
    // calls in the same window keep returning the same answer.
    setTitle("Repeated denials stay consistent");

    std::cout << "request #4: " << (limiter.allow("client-a", now) ? "allowed" : "denied") << "\n";
    std::cout << "request #5: " << (limiter.allow("client-a", now) ? "allowed" : "denied") << "\n";
}

REGISTER_EXAMPLE_SUITE();
