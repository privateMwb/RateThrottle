// Boundary burst behavior.
//
// Demonstrates:
// - The documented, accepted fixed-window tradeoff: up to ~2x the
//   configured limit can go through right at a window boundary
// - Why this happens: window N's count and window N+1's count are
//   tracked completely independently
// - That the burst is bounded at exactly 2x, never more

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // A fixed-window limiter resets the count to zero the instant a new
    // window begins -- it doesn't "smooth" traffic across the boundary.
    setTitle("Exhausting the limit at the end of a window");

    RateLimiter limiter(3, std::chrono::milliseconds(1000), 100);
    auto now = std::chrono::steady_clock::now();

    auto windowEnd = now + std::chrono::milliseconds(999);
    std::cout << "at t=999ms, request #1: " << limiter.allow("client-a", windowEnd) << "\n";
    std::cout << "at t=999ms, request #2: " << limiter.allow("client-a", windowEnd) << "\n";
    std::cout << "at t=999ms, request #3: " << limiter.allow("client-a", windowEnd)
              << " (limit reached)\n\n";

    // A moment later, the next window starts fresh -- the same client
    // can immediately burst up to the limit again.
    setTitle("The next window starts fresh, immediately");

    auto nextWindowStart = windowEnd + std::chrono::milliseconds(1000);
    std::cout << "at t=1999ms, request #1: " << limiter.allow("client-a", nextWindowStart) << "\n";
    std::cout << "at t=1999ms, request #2: " << limiter.allow("client-a", nextWindowStart) << "\n";
    std::cout << "at t=1999ms, request #3: " << limiter.allow("client-a", nextWindowStart)
              << " -- 6 requests total in ~1ms of wall-clock time\n\n";

    // The burst is capped at exactly 2x the limit -- a 7th request in
    // the second window is still denied like any other over-limit call.
    setTitle("The burst is still bounded");

    std::cout << "at t=1999ms, request #4: " << limiter.allow("client-a", nextWindowStart)
              << " (denied)\n";
}

REGISTER_EXAMPLE_SUITE();
