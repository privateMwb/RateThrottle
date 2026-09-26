#!/bin/bash -eu
# ============================================================
# .clusterfuzzlite/build.sh
#
# RateThrottle is not header-only: RateLimiter.cpp
# is its own translation unit, so it's compiled once to an object
# file and linked into each fuzz binary alongside the harness.
#
# RateLimiter.h pulls in CachePro/LRUCache.h, so both include/ trees
# (this repo's and CachePro's, cloned by the Dockerfile) need to be
# on the include path.
#
# Add more `${SRC}/RateThrottle/fuzz/fuzz_*.cpp` harnesses here as
# they're added; each becomes its own $OUT binary. RateLimiter.o only
# needs to be built once and can be reused across all of them.
# ============================================================

cd "${SRC}/RateThrottle"

$CXX $CXXFLAGS -std=c++20 \
  -I"${SRC}/RateThrottle/include" \
  -I"${SRC}/CachePro/include" \
  -c src/ThrottlePro/RateLimiter.cpp \
  -o "${WORK}/RateLimiter.o"

$CXX $CXXFLAGS -std=c++20 \
  -I"${SRC}/RateThrottle/include" \
  -I"${SRC}/CachePro/include" \
  fuzz/fuzz_rate_limiter.cpp \
  "${WORK}/RateLimiter.o" \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_rate_limiter"
