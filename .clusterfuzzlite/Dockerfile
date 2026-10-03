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

FROM gcr.io/oss-fuzz-base/base-builder@sha256:fa91d623389589a099c27600abaca3d1dd31299b12715c37ce386ec27fb46f42

COPY . $SRC/RateThrottle

# CachePro is the library's namespace/alias -- the actual repo is named
# LRUCache (see the Dependencies table in README.md). Adjust the ref
# (branch/tag/commit) to whatever version RateThrottle is actually built
# against.
RUN git clone --depth 1 --branch main \
    https://github.com/privateMwb/LRUCache.git $SRC/CachePro

COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/RateThrottle
