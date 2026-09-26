<p align="center">
  <img src=".github/assets/banner.svg" alt="RateThrottle" width="100%">
</p>

<p align="center">
  <img src="https://img.shields.io/github/v/release/privateMwb/RateThrottle?style=for-the-badge&logo=github&color=A3E635&labelColor=07090A" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-C154D8?style=for-the-badge&labelColor=07090A" alt="License - MIT">
  <img src="https://img.shields.io/badge/C%2B%2B-23-D946EF?style=for-the-badge&labelColor=07090A" alt="C++ - 23">
</p>

<p align="center">
  <img src=".github/assets/divider.svg" alt="" width="100%">
</p>

<p align="center"><sub><b>CI / CD</b></sub></p>
<p align="center">
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/build.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/build.yml/badge.svg" alt="Build and Test">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/benchmark.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/benchmark.yml/badge.svg" alt="Benchmarks">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/packaging.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/packaging.yml/badge.svg" alt="Packaging">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/release.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/release.yml/badge.svg" alt="Release">
  </a>
</p>

<p align="center"><sub><b>Code Quality &amp; Safety</b></sub></p>
<p align="center">
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/coverage.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/coverage.yml/badge.svg" alt="Coverage">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/sanitizers.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/sanitizers.yml/badge.svg" alt="Sanitizers">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/clang-tidy.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/clang-tidy.yml/badge.svg" alt="Clang Tidy">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/clang-format.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/clang-format.yml/badge.svg" alt="Clang Format">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/codeql.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/codeql.yml/badge.svg" alt="CodeQL">
  </a>
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/cflite_pr.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/cflite_pr.yml/badge.svg" alt="Fuzzing">
  </a>
  <a href="https://www.bestpractices.dev/projects/14950">
    <img src="https://www.bestpractices.dev/projects/14950/badge" alt="OpenSSF Best Practices">
  </a>
</p>

<p align="center"><sub><b>Documentation</b></sub></p>
<p align="center">
  <a href="https://github.com/privateMwb/RateThrottle/actions/workflows/docs.yml">
    <img src="https://github.com/privateMwb/RateThrottle/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
</p>

<p align="center">
  <img src=".github/assets/divider.svg" alt="" width="100%">
</p>

<p align="center"><sub><b>Compiler Support</b></sub></p>
<p align="center">
  <img src="https://img.shields.io/badge/GCC-support-B46F1B?style=flat&logo=gnu" alt="GCC - support">
  <img src="https://img.shields.io/badge/Clang-support-045891?style=flat&logo=llvm" alt="Clang - support">
  <img src="https://img.shields.io/badge/MSVC-support-5C2D91?style=flat" alt="MSVC - support">
  <img src="https://img.shields.io/badge/AppleClang-support-000000?style=flat&logo=apple" alt="AppleClang - support">
</p>

<p align="center">
  <img src=".github/assets/divider.svg" alt="" width="100%">
</p>

<p align="center">RateThrottle is a framework-agnostic C++ fixed-window rate limiter — no knowledge of HTTP, requests, or middleware, just `allow(key, now)` — backed by this author's own LRU cache library.</p>

<br>

## 📑 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Development](#development)
- [Benchmarks](#benchmarks)
- [Fuzzing](#fuzzing)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [Security](#security)
- [License](#license)

<br>

## <a id="features"></a>✨ Features

- **Fixed-window rate limiting** — `RateLimiter::allow(key, now)` tracks a per-key request count and window-start timestamp via `CachePro::LRUCache`, denying once the configured limit is reached within the current window.
- **Framework-agnostic core** — no knowledge of HTTP, requests, or middleware; `allow()` is the entire public surface, so it can sit behind any framework's own request pipeline.
- **Deterministic, testable time** — `allow()`'s `now` parameter defaults to `std::chrono::steady_clock::now()` but is fully overridable, so window-boundary and reset behavior can be tested without waiting on real time.
- **Thread-safe by construction** — every cache access is guarded by an internal mutex, since `CachePro::LRUCache` provides no synchronization of its own.
- **Runtime-configurable limits** — requests-per-window, window duration, and cache capacity are constructor parameters, not hardcoded constants.
- **Honestly documented trade-offs** — the fixed-window algorithm's known ~2x boundary-burst behavior, and the LRU-eviction-mid-window edge case (an evicted-but-active key's window silently restarting), are both tested and documented rather than hidden.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="requirements"></a>📋 Requirements

- A C++23-conformant compiler (tested: GCC, Clang, MSVC, AppleClang)
- CMake 3.20+
- Git submodules initialized — unlike this author's other, dependency-free libraries, RateThrottle is a consumer of `CachePro` (see [Dependencies](#dependencies)) and needs its source present to build from source

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="dependencies"></a>🔗 Dependencies

RateThrottle is built on this author's own `CachePro`, vendored as a git submodule under `libs/internal/`:

| Library | Provides | Repository |
|---|---|---|
| CachePro | `LRUCache<K,V>`, backing `RateLimiter`'s per-key window tracking | [privateMwb/LRUCache](https://github.com/privateMwb/LRUCache) |

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="installation"></a>📦 Installation

**From source:**

```bash
git clone --recurse-submodules https://github.com/privateMwb/RateThrottle.git
cd RateThrottle
cmake -B build \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_REGRESSION=OFF \
  -DBUILD_EXAMPLES=OFF
cmake --install build
```

Then, in your own `CMakeLists.txt`:

```cmake
find_package(RateThrottle CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE RateThrottle::RateThrottle)
```

> vcpkg and Conan packages are built and verified (recipe in
> `packaging/recipes/ratethrottle/`, port in `packaging/vcpkg/ports/ratethrottle/`),
> but not yet published to the public registries. This section will be
> updated once they are.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="quick-start"></a>🚀 Quick Start

```cpp
#include <ratethrottle/RateLimiter.h>

using namespace ThrottlePro;

int main() {
    // 100 requests per key, per 60-second window, tracking up to 10,000
    // distinct keys before the least-recently-used one is evicted.
    RateLimiter limiter(/*requestsPerWindow=*/100,
                         /*windowDuration=*/std::chrono::milliseconds(60'000),
                         /*cacheCapacity=*/10'000);

    std::string clientIp = "203.0.113.7";

    if (!limiter.allow(clientIp)) {
        // Reject however your framework prefers (429 Too Many Requests,
        // for HTTP) -- RateLimiter has no opinion on that itself.
        return 1;
    }

    // request proceeds
}
```

`allow()`'s `now` parameter makes window behavior deterministic to test, without waiting on real time:

```cpp
RateLimiter limiter(2, std::chrono::milliseconds(1000), 16);
auto now = std::chrono::steady_clock::now();

limiter.allow("client", now); // 1st request: allowed
limiter.allow("client", now); // 2nd request: allowed (at the limit)
limiter.allow("client", now); // 3rd request: denied

limiter.allow("client", now + std::chrono::milliseconds(1001)); // window expired: allowed again
```

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="project-structure"></a>🗂️ Project Structure

```
RateThrottle/
├── include/
│   └── ratethrottle/
│       └── RateLimiter.h
│
├── src/
│   └── ratethrottle/
│       └── RateLimiter.cpp
│
├── libs/
│   └── internal/
│       └── CachePro/
│
├── tests/
│   ├── custom/
│   ├── google/
│   ├── CMakeLists.txt
│   └── README.md
│
├── benchmarks/
│   ├── custom/
│   ├── google/
│   ├── baselines/
│   ├── results/
│   ├── CMakeLists.txt
│   └── README.md
│
├── examples/
│   ├── support/
│   ├── suite/
│   ├── example_main.cpp
│   └── CMakeLists.txt
│
├── regression/
│   ├── custom/
│   ├── google/
│   ├── results/
│   ├── CMakeLists.txt
│   └── README.md
│
├── fuzz/
│   └── fuzz_rate_limiter.cpp
│
├── .clusterfuzzlite/
│   ├── Dockerfile
│   ├── build.sh
│   └── project.yaml
│
├── packaging/
│   ├── README.md
│   ├── requirements.in
│   ├── requirements.txt
│   ├── recipes/
│   ├── vcpkg/
│   └── vcpkg-smoke-test/
│
├── scripts/
│   └── update_package_files.py
│
├── .github/
│   ├── assets/
│   ├── releases/
│   ├── workflows/
│   ├── CODEOWNERS
│   └── dependabot.yml
│
├── cmake/
│   └── ThrottleProConfig.cmake.in
│
├── docs/
│   ├── Doxyfile
│   └── README.md
│
├── .clang-format
├── .clang-tidy
├── .gitignore
├── CMakeLists.txt
├── README.md
├── CONTRIBUTING.md
├── CHANGELOG.md
├── SECURITY.md
├── FUZZING.md
└── LICENSE
```

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="development"></a>🛠️ Development

The from-source install above builds the library only. To work on
RateThrottle itself — running tests, benchmarks, or the regression tool —
build with everything enabled (the default):

```bash
cmake -B build
cmake --build build
```

**Run the test suite:**

```bash
ctest --test-dir build
```

**Run benchmarks and check for regressions:**

```bash
./build/benchmarks
./build/regression                  # latest baseline vs. benchmarks/results/benchmark_results.json
./build/regression v1.2.0           # a specific baseline vs. current
./build/regression v1.2.0 v1.4.0    # two baselines against each other
```

`regression` picks the latest baseline by semantic version (`v1.10.0`
correctly outranks `v1.9.0`), not alphabetical filename order, and
auto-names its output (`regression_v1.2.0_vs_current.md`/`.json`, etc.).

See [packaging/README.md](packaging/README.md) for notes on verifying the vcpkg
port and Conan recipe locally, and [FUZZING.md](FUZZING.md) for running the
fuzz harness locally.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="benchmarks"></a>📊 Benchmarks

RateThrottle has no naive baseline to compare against — there is no
`stdThrottle` — so figures below are `RateLimiter` measured alone. Full
results across every benchmark and scale: `benchmarks/results/v1_0_0.md`.

| Operation | RateThrottle(1M) |
|---|---|
| Allow() Window Reset | 28.41 ms |
| Allow() Within Window | 49.92 ms |
| Allow() Deny | 50.00 ms |
| Allow() Small Capacity | 115.87 ms |
| Allow() Eviction Pressure | 131.94 ms |
| Allow() New Key | 198.63 ms |
| Allow() Large Capacity | 212.54 ms |
| Allow() Contention | 691.20 ms |

The branch taken (new window vs. increment vs. deny) turns out not to be
the dominant cost — reusing the same key lands every branch in the same
~28–50 ms band. What actually separates fast from slow is whether each
call constructs and hashes a brand-new unique key, which costs roughly
**4–7x more** across the board.

`Contention` is the clear outlier: a single global mutex guarding the
whole cache, hammered by four background threads on the same key, is by
far the most expensive path measured — the direct, now-measured cost of
not sharding the lock per key in v1.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="fuzzing"></a>🐛 Fuzzing

`ThrottlePro::RateLimiter::allow()` is fuzzed via
[ClusterFuzzLite](https://google.github.io/clusterfuzzlite/), under
AddressSanitizer and UndefinedBehaviorSanitizer. A 5-minute pass runs on
every PR touching `RateLimiter` or the harness; a one-hour batch pass
runs nightly.

`fuzz_rate_limiter` is a differential fuzzer, checked after every single
`allow()` call against a shadow model implementing the same fixed-window
algorithm directly, so a failing input localizes to the exact call that
broke an invariant. Time is fully fuzzer-controlled — a synthetic clock
advances by fuzzer-chosen deltas biased toward landing exactly on, or one
tick either side of, the window boundary, since that's exactly where a
fixed-window off-by-one would hide. Keys are split into a fixed "steady"
pool sized to the configured cache capacity (never evicted, so the shadow
model's predictions stay exact) and an "overflow" pool that deliberately
forces real `CachePro::LRUCache` eviction, checked only for the one
invariant that must hold regardless of eviction: `requestsPerWindow == 0`
always denies.

Concurrent `allow()` calls from multiple threads are deliberately not
covered by this harness — libFuzzer's single-threaded-per-input model
can't exercise `RateLimiter`'s internal mutex as a thread-safety claim,
which is left to a dedicated ThreadSanitizer-based test instead.

See [FUZZING.md](FUZZING.md) for running the harness locally and
reproducing a failing input.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="documentation"></a>📖 Documentation

Full API reference, generated with Doxygen from `docs/Doxyfile`:

**https://privateMwb.github.io/RateThrottle/**

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="contributing"></a>🤝 Contributing

Issues and pull requests are welcome. Before submitting a PR:

- Run the test suite (`ctest --test-dir build`)
- If you're changing a hot path, run `./build/regression` and mention
  the results in your PR description
- If you're changing `RateLimiter::allow()`'s window logic, a quick
  local fuzz pass (see [FUZZING.md](FUZZING.md)) before pushing catches most
  fixed-window edge-case regressions before CI does

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="changelog"></a>📝 Changelog

See the [Releases](https://github.com/privateMwb/RateThrottle/releases)
page for version history and release notes.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="security"></a>🔒 Security

Please don't report a suspected vulnerability in a public issue. Use
GitHub's private reporting instead — this repository's **Security** tab,
then **Report a vulnerability** — so it can be fixed before it's disclosed.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="license"></a>📄 License

MIT — see [LICENSE](LICENSE) for details.

<p align="center">
  <sub>Built with C++23</sub>
</p>

<p align="center">
  <a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a>
</p>
