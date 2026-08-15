// Per-client throttling.
//
// Demonstrates:
// - Using a caller-supplied key (e.g. client IP or API key) to track
//   distinct clients independently
// - One client reaching its limit does not affect another
// - A realistic request-handling loop over several clients

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // RateLimiter has no notion of "clients" -- the caller decides what
    // the key means. Here it's a client IP address.
    setTitle("Keying by client IP");

    RateLimiter limiter(2, std::chrono::milliseconds(1000), 100);
    auto now = std::chrono::steady_clock::now();

    std::cout << "192.168.1.10 request #1: " << limiter.allow("192.168.1.10", now) << "\n";
    std::cout << "192.168.1.10 request #2: " << limiter.allow("192.168.1.10", now) << "\n";
    std::cout << "192.168.1.10 request #3: " << limiter.allow("192.168.1.10", now) << "\n\n";

    // A different client's requests are tracked completely separately,
    // even though both share the same limiter instance.
    setTitle("A different client is unaffected");

    std::cout << "10.0.0.5 request #1: " << limiter.allow("10.0.0.5", now) << "\n";
    std::cout << "10.0.0.5 request #2: " << limiter.allow("10.0.0.5", now) << "\n";

    // A simple request-handling loop: each incoming request carries its
    // own client identifier, and the same limiter instance is reused
    // across every request.
    setTitle("Handling a stream of requests");

    std::vector<std::string> incoming = {"192.168.1.10", "10.0.0.5", "192.168.1.10", "203.0.113.7"};

    for (const auto& clientIp : incoming) {
        bool allowed = limiter.allow(clientIp, now);
        std::cout << clientIp << ": " << (allowed ? "allowed" : "denied") << "\n";
    }
}

REGISTER_EXAMPLE_SUITE();
