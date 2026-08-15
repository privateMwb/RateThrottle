// Ignoring allow()'s return value.
//
// Demonstrates:
// - allow() is marked [[nodiscard]] specifically because it has a
//   side effect (recording the request) even when the caller doesn't
//   look at the result
// - The mistake: calling allow() only to record traffic, assuming it's
//   a passive logging call with no meaningful return
// - Why this silently defeats the entire point of throttling

#include <support/framework.h>

using namespace ThrottlePro;

static void run_examples() {

    // The mistake: calling allow() as if it were a side-effect-only
    // logging call, and never checking whether the request should
    // actually be rejected. [[nodiscard]] makes this a compiler warning
    // (or error, depending on flags) rather than a silent bug -- but
    // it's still possible to explicitly discard the result with (void).
    setTitle("The mistake: discarding the result");

    RateLimiter limiter(2, std::chrono::milliseconds(1000), 100);
    auto now = std::chrono::steady_clock::now();

    (void)limiter.allow("client-a", now);
    (void)limiter.allow("client-a", now);
    (void)limiter.allow("client-a", now); // over the limit, but nothing acted on it

    std::cout << "3 requests recorded, but the 3rd was over-limit and nothing rejected it\n\n";

    // Every over-limit request that gets processed anyway defeats the
    // purpose of having a limiter in the first place -- the downstream
    // resource still sees full, unthrottled traffic.
    setTitle("The consequence");

    std::cout << "the limiter's internal state is still correct (denied #3 internally),\n"
              << "but the caller never found out, so the request was processed anyway\n\n";

    // The correct pattern: always branch on the result and act on it.
    setTitle("The correct pattern");

    RateLimiter correct(2, std::chrono::milliseconds(1000), 100);

    for (int i = 0; i < 3; ++i) {
        if (correct.allow("client-b", now)) {
            std::cout << "request " << i << ": processed\n";
        } else {
            std::cout << "request " << i << ": rejected\n";
        }
    }
}

REGISTER_EXAMPLE_SUITE();
