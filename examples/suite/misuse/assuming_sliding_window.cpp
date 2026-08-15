// Assuming sliding-window semantics.
//
// Demonstrates:
// - The mistake: expecting a "requests per rolling N seconds" guarantee,
//   which RateLimiter does NOT provide
// - What actually happens instead: fixed, non-overlapping windows that
//   reset abruptly at boundaries
// - Why this surprises people coming from sliding-window or
//   token-bucket limiters

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // The mistake: assuming "3 requests per second" means no 4th
    // request can ever land within any rolling 1-second span.
    setTitle("The assumption: a rolling window guarantee");

    RateLimiter limiter(3, std::chrono::milliseconds(1000), 100);
    auto now = std::chrono::steady_clock::now();

    std::cout << "expected (sliding window): never more than 3 requests in any 1000ms span\n\n";

    // What actually happens: three requests land right at the end of
    // one fixed window, then three more land right at the start of the
    // next -- six requests within roughly 1ms, well inside what a
    // sliding window would have blocked.
    setTitle("What fixed windows actually allow");

    auto windowEnd = now + std::chrono::milliseconds(999);
    (void)limiter.allow("client-a", windowEnd);
    (void)limiter.allow("client-a", windowEnd);
    (void)limiter.allow("client-a", windowEnd);

    auto nextWindowStart = windowEnd + std::chrono::milliseconds(1000);
    bool r1 = limiter.allow("client-a", nextWindowStart);
    bool r2 = limiter.allow("client-a", nextWindowStart);
    bool r3 = limiter.allow("client-a", nextWindowStart);

    std::cout << "6 requests allowed within ~1ms of wall-clock time: " << (r1 && r2 && r3)
              << "\n\n";

    // This tradeoff is deliberate and documented, not a bug -- see
    // RateLimiter.h and boundary_burst_behavior.cpp. If a true rolling
    // guarantee is required, a fixed-window limiter is the wrong
    // algorithm; a sliding-window or token-bucket limiter would be
    // needed instead.
    setTitle("This is a known, documented tradeoff");

    std::cout << "fixed-window is v1; sliding window / token bucket are candidate v2 algorithms\n";
}

REGISTER_EXAMPLE_SUITE();
