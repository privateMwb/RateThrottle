// Deterministic testing with explicit time points.
//
// Demonstrates:
// - Why allow()'s `now` parameter is overridable instead of always
//   using steady_clock::now() internally
// - Simulating a window expiring without sleeping the test
// - Simulating many windows in sequence instantly

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // allow() defaults to steady_clock::now(), but accepts an explicit
    // time point -- this is what makes window-boundary behavior testable
    // without an actual std::this_thread::sleep_for().
    setTitle("Why the time is overridable");

    RateLimiter limiter(1, std::chrono::milliseconds(1000), 100);
    auto simulatedNow = std::chrono::steady_clock::now();

    std::cout << "request at t=0ms   : " << limiter.allow("client-a", simulatedNow) << "\n";
    std::cout << "request at t=0ms   : " << limiter.allow("client-a", simulatedNow)
              << " (denied)\n\n";

    // Advancing the simulated clock past the window duration, without
    // any real time passing, exercises the expiry path directly.
    setTitle("Simulating window expiry");

    auto simulatedLater = simulatedNow + std::chrono::milliseconds(1000);
    std::cout << "request at t=1000ms: " << limiter.allow("client-a", simulatedLater)
              << " (fresh window)\n\n";

    // The same technique scales to simulating many windows in sequence
    // instantly, which would otherwise take real wall-clock time.
    setTitle("Simulating several windows in sequence");

    for (int window = 0; window < 4; ++window) {
        auto windowStart = simulatedNow + std::chrono::milliseconds(window * 1000);
        bool allowed = limiter.allow("client-b", windowStart);
        std::cout << "window " << window << ": " << (allowed ? "allowed" : "denied") << "\n";
    }
}

REGISTER_EXAMPLE_SUITE();
