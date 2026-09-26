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

FROM gcr.io/oss-fuzz-base/base-builder@sha256:32a88ae7628e937252cdaaead897f8da4083dd08d862e27f449b59e96972cad0

COPY . $SRC/RateThrottle

# CachePro is the library's namespace/alias -- the actual repo is named
# LRUCache (see the Dependencies table in README.md). Adjust the ref
# (branch/tag/commit) to whatever version RateThrottle is actually built
# against.
RUN git clone --depth 1 --branch main \
    https://github.com/privateMwb/LRUCache.git $SRC/CachePro

COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/RateThrottle
