// Embedding RateLimiter in middleware.
//
// Demonstrates:
// - Wrapping RateLimiter as a private implementation detail behind a
//   small request-handling function
// - Keeping RateLimiter itself framework-agnostic while the wrapper
//   translates allow()/deny into whatever the surrounding code expects
//   (here, a plain status string standing in for an HTTP response)
// - The wrapper owning the limiter's lifetime for the duration of the
//   server

#include <support/framework.h>

using namespace ThrottlePro;

// A minimal stand-in for request-handling middleware. RateLimiter has
// no knowledge of HTTP -- this class is where that translation happens.
class ThrottleMiddleware {
  public:
    ThrottleMiddleware(std::size_t requestsPerWindow, std::chrono::milliseconds window)
        : limiter_(requestsPerWindow, window, /*cacheCapacity=*/1000) {}

    // Returns a stand-in status: "200 OK" or "429 Too Many Requests".
    std::string handle(const std::string& clientIp) {
        if (limiter_.allow(clientIp)) {
            return "200 OK";
        }
        return "429 Too Many Requests";
    }

  private:
    RateLimiter limiter_;
};

static void run_examples() {

    // The rest of the application only ever sees ThrottleMiddleware --
    // RateLimiter is an internal detail it doesn't need to know about.
    setTitle("Wrapping RateLimiter behind a handler");

    ThrottleMiddleware middleware(2, std::chrono::milliseconds(1000));

    std::cout << "GET /api/data from 203.0.113.7: " << middleware.handle("203.0.113.7") << "\n";
    std::cout << "GET /api/data from 203.0.113.7: " << middleware.handle("203.0.113.7") << "\n";

    // The wrapper's own vocabulary (HTTP-style statuses) is what the
    // rest of the codebase interacts with.
    setTitle("The limit surfaces as a normal response");

    std::cout << "GET /api/data from 203.0.113.7: " << middleware.handle("203.0.113.7") << "\n";

    // A different route can reuse the same pattern with its own
    // middleware instance and its own limit.
    setTitle("A second middleware instance with a different limit");

    ThrottleMiddleware loginMiddleware(1, std::chrono::milliseconds(5000));
    std::cout << "POST /login from 203.0.113.7: " << loginMiddleware.handle("203.0.113.7") << "\n";
    std::cout << "POST /login from 203.0.113.7: " << loginMiddleware.handle("203.0.113.7") << "\n";
}

REGISTER_EXAMPLE_SUITE();
