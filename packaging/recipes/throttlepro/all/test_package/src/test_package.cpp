// Minimal smoke test: confirms the installed package's headers are
// reachable and the library links, by constructing the core type.
//
// This deliberately doesn't exercise throttling behavior itself --
// that's covered by the main test suite. This only proves the
// installed package (headers + compiled static lib) is consumable
// from a fresh, standalone CMake project via find_package().
#include <ThrottlePro/RateLimiter.h>

#include <chrono>
#include <iostream>

int main() {
    rain::RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    bool allowed = limiter.allow("smoke-test-key");
    std::cout << "ThrottlePro linked and constructed successfully. allow(): " << allowed << "\n";
    return 0;
}
