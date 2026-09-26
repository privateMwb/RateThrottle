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

FROM gcr.io/oss-fuzz-base/base-builder@sha256:8b4a73d83374b298a0a771eeec9a1d44ceff3416a678b7d7946303389d022aca

COPY . $SRC/RateThrottle

# Adjust the ref (branch/tag/commit) to whatever CachePro version
# RateThrottle is actually built against.
RUN git clone --depth 1 --branch main \
    https://github.com/privateMwb/CachePro.git $SRC/CachePro

COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/RateThrottle
