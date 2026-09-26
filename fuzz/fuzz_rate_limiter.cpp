// ============================================================
// fuzz/fuzz_rate_limiter.cpp
//
// Differential fuzzer for ThrottlePro::RateLimiter, checked after
// every single allow() call (not just at the end) against a shadow
// model implementing the same fixed-window algorithm directly, so a
// fuzzer-found mismatch localizes to the exact call that caused it.
//
// The shadow model is only meaningful while RateLimiter's underlying
// LRUCache never evicts -- once eviction happens, an evicted key's
// window legitimately (and by design, see RateLimiter.h) resets,
// which the shadow model has no way to predict without duplicating
// CachePro::LRUCache's own eviction-order logic. So this harness
// keeps two separate modes instead of trying to fake that -- each
// backed by its OWN RateLimiter instance, so the two pools never
// contend for the same cache slots (an early version of this harness
// shared one instance across both pools; overflow-key insertions
// could then evict a "steady" key the shadow model assumed was
// eviction-proof, producing a false-positive mismatch that had
// nothing to do with RateLimiter itself):
//
//   - "steady" keys, drawn from a fixed pool no larger than the
//     configured cache capacity, so no eviction is ever possible for
//     them. Every allow() on a steady key is checked bit-for-bit
//     against the shadow model. This is where the actual
//     fixed-window logic -- window expiry, count increment, deny at
//     the limit -- gets exercised precisely.
//   - "overflow" keys, drawn from a strictly-increasing counter, used
//     to force the cache past capacity and trigger real eviction.
//     These are NOT differentially checked (no reliable oracle for
//     which key gets evicted), but every call still passes through
//     ASan/UBSan, and the requestsPerWindow == 0 invariant --
//     "always deny, no exceptions" -- is checked here too, since
//     that invariant must hold regardless of eviction.
//
// Time is fully fuzzer-controlled rather than wall-clock: each step
// advances a synthetic steady_clock::time_point by a fuzzer-chosen
// delta, biased toward landing exactly on or just inside/outside the
// window boundary, since that boundary is where a fixed-window
// off-by-one would hide.
//
// Specifically targets:
//   - fresh-window-on-first-sight and fresh-window-on-expiry, exactly
//     at the `>= windowDuration_` boundary (not one tick before or
//     after)
//   - count increment / deny-at-limit for requestsPerWindow across a
//     range of small values, including 1
//   - requestsPerWindow == 0 short-circuiting to always-deny, checked
//     both for never-seen keys and for keys with an existing window
//     (steady keys) and for evicted/overflow keys
//   - RateLimitWindow state surviving get()-then-mutate-in-place
//     (existing->count++) without a put(), across many calls
//
// Deliberately NOT covered here:
//   - Which specific key CachePro::LRUCache evicts under contention
//     (would require differentially modeling LRUCache itself, which
//     belongs in CachePro's own fuzz suite, not this one)
//   - Concurrent allow() calls from multiple threads. RateLimiter's
//     mutex_ makes this a thread-safety claim, not just a sequential
//     algorithm; a real concurrency test needs a threaded harness
//     (e.g. TSan + several threads hammering shared keys), which
//     libFuzzer's single-threaded-per-input model doesn't exercise.
//     Natural follow-up harness, not a change to this one.
// ============================================================

#include <ThrottlePro/RateLimiter.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>

using ThrottlePro::RateLimiter;

namespace {

using Clock = std::chrono::steady_clock;

struct ShadowWindow {
    Clock::time_point windowStart;
    std::size_t count = 0;
};

// Mirrors RateLimiter::allow()'s fixed-window algorithm exactly, for
// keys we know can never be evicted from the real cache.
bool shadowAllow(std::unordered_map<std::string, ShadowWindow>& shadow, std::size_t requestsPerWindow,
                  std::chrono::milliseconds windowDuration, const std::string& key, Clock::time_point now) {
    if (requestsPerWindow == 0)
        return false;

    auto it = shadow.find(key);
    if (it == shadow.end() || (now - it->second.windowStart) >= windowDuration) {
        shadow[key] = ShadowWindow{now, 1};
        return true;
    }
    if (it->second.count < requestsPerWindow) {
        ++it->second.count;
        return true;
    }
    return false;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size < 4)
        return 0;

    // --- Config header: capacity, limit, window duration ---
    const std::size_t cacheCapacity = (data[0] % 8) + 1;       // [1, 8]
    const std::size_t requestsPerWindow = data[1] % 9;         // [0, 8]
    const auto windowDuration = std::chrono::milliseconds((data[2] % 200) + 1); // [1ms, 200ms]
    data += 3;
    size -= 3;

    // Separate instances -- and separate underlying caches -- for each
    // pool. steadyLimiter is only ever touched by steadyKeys (see
    // below), so it can never need to evict; overflowLimiter absorbs
    // all the eviction pressure instead, without threatening that
    // guarantee.
    RateLimiter steadyLimiter(requestsPerWindow, windowDuration, cacheCapacity);
    RateLimiter overflowLimiter(requestsPerWindow, windowDuration, cacheCapacity);
    std::unordered_map<std::string, ShadowWindow> shadow;

    // Steady key pool sized exactly to cacheCapacity: as long as only
    // these keys are used against steadyLimiter, its cache can never
    // need to evict, so the shadow model's predictions stay exact.
    std::string steadyKeys[8];
    for (std::size_t i = 0; i < cacheCapacity; ++i)
        steadyKeys[i] = "steady-" + std::to_string(i);

    std::uint64_t overflowCounter = 0;
    Clock::time_point now = Clock::now();

    for (std::size_t i = 0; i < size; ++i) {
        const std::uint8_t byte = data[i];
        const std::uint8_t op = byte % 4;

        // --- Advance the synthetic clock ---
        switch (op) {
        case 0:
            // Small forward step, well inside a window.
            now += std::chrono::milliseconds(byte % 5);
            break;
        case 1:
            // Land exactly on the boundary: this call's window should
            // be treated as expired (`>=`, not `>`).
            now += windowDuration;
            break;
        case 2:
            // Land one tick before the boundary: this call's window
            // must still be treated as active.
            if (windowDuration.count() > 0)
                now += windowDuration - std::chrono::milliseconds(1);
            break;
        default:
            // No time movement this step -- several rapid-fire calls
            // in the same instant.
            break;
        }

        const bool useOverflowKey = (byte % 16 == 0); // occasional eviction pressure

        if (useOverflowKey) {
            const std::string key = "overflow-" + std::to_string(overflowCounter++);
            const bool realResult = overflowLimiter.allow(key, now);

            // The only thing guaranteed regardless of eviction.
            if (requestsPerWindow == 0 && realResult)
                std::abort();
        } else {
            const std::string& key = steadyKeys[byte % cacheCapacity];
            const bool realResult = steadyLimiter.allow(key, now);
            const bool shadowResult = shadowAllow(shadow, requestsPerWindow, windowDuration, key, now);

            if (realResult != shadowResult)
                std::abort();
        }
    }

    return 0;
}
