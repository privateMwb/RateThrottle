# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
#
# RateThrottle isn't header-only: RateLimiter.h depends on
# CachePro::LRUCache, a separate library, so its headers must be
# present at build time too. Pin to a tag/commit rather than tracking
# a moving branch, so a break in CachePro can't silently break this
# repo's fuzzing CI.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:8a9ddfa32864f269c7cf052ddc11843645f7353a321d0399bb50327c6167770e

COPY . $SRC/RateThrottle

# CachePro is the library's namespace/alias -- the actual repo is named
# LRUCache (see the Dependencies table in README.md). Adjust the ref
# (branch/tag/commit) to whatever version RateThrottle is actually built
# against.
RUN git clone --depth 1 --branch main \
    https://github.com/privateMwb/LRUCache.git $SRC/CachePro

COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/RateThrottle
