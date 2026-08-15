// Minimal smoke test: confirms the vcpkg-installed package's headers
// are reachable and the library links, by constructing the core type.
//
// Same scope as the Conan test_package smoke test — proves the
// package is installable and linkable, not that its behavior is
// correct.
#include <ThrottlePro/RateLimiter.h>

#include <chrono>
#include <iostream>

int main() {
    rain::RateLimiter limiter(3, std::chrono::milliseconds(1000), 8);
    bool allowed = limiter.allow("smoke-test-key");
    std::cout << "ThrottlePro linked and constructed successfully. allow(): " << allowed << "\n";
    return 0;
}
