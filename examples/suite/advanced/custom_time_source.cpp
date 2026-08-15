// Custom time sources.
//
// Demonstrates:
// - Supplying a request's own timestamp instead of the moment allow()
//   happens to run
// - Wrapping RateLimiter behind a small helper that injects time from
//   an external clock (e.g. a request struct, a message queue's
//   enqueue time)
// - Why this matters for requests processed slightly out of order or
//   with queueing delay

#include <support/framework.h>

using namespace ThrottlePro;

// A stand-in for whatever timestamp a real request already carries --
// e.g. when it arrived at a load balancer, not when this code runs.
struct IncomingRequest {
    std::string clientKey;
    std::chrono::steady_clock::time_point receivedAt;
};

static void run_examples() {

    // allow()'s default argument (steady_clock::now()) is only a
    // convenience for the common case. Passing the request's own
    // timestamp is often more correct if there's any delay between
    // when a request arrived and when it's actually processed.
    setTitle("Using a request's own timestamp");

    RateLimiter limiter(2, std::chrono::milliseconds(1000), 100);
    auto baseline = std::chrono::steady_clock::now();

    IncomingRequest req{"client-a", baseline};
    std::cout << "processed using request's receivedAt: "
              << limiter.allow(req.clientKey, req.receivedAt) << "\n\n";

    // If requests are dequeued out of order (e.g. from a priority
    // queue), each one still carries its own correct timestamp, so the
    // limiter sees them in logical arrival order regardless of
    // processing order.
    setTitle("Out-of-order processing, correct results");

    IncomingRequest earlier{"client-b", baseline};
    IncomingRequest later{"client-b", baseline + std::chrono::milliseconds(500)};

    // Processed out of order: `later` handled first.
    std::cout << "later   allowed: " << limiter.allow(later.clientKey, later.receivedAt) << "\n";
    std::cout << "earlier allowed: " << limiter.allow(earlier.clientKey, earlier.receivedAt)
              << "\n";

    // A small helper that wraps allow() and hides the timestamp
    // plumbing from call sites.
    setTitle("Wrapping the timestamp plumbing");

    auto handle = [&](const IncomingRequest& r) {
        return limiter.allow(r.clientKey, r.receivedAt);
    };

    IncomingRequest req2{"client-c", baseline};
    std::cout << "handle(req2): " << handle(req2) << "\n";
}

REGISTER_EXAMPLE_SUITE();
