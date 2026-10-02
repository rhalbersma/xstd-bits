#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Pinned by digest, as the workflows pin their actions: the builder's clang and libc++ are what the targets compile with.
FROM gcr.io/oss-fuzz-base/base-builder@sha256:e67da2b0f6156b29b62734305a8c74db61aaf3158c7ab96386a07974fb33cdc8

# The builder's CMake is 3.29, and CMakeLists.txt requires 3.30; the same hash-pinned wheel the toolchain script installs.
COPY tools/cmake-requirements.txt /tmp/cmake-requirements.txt
RUN unset CFLAGS CXXFLAGS && pip3 install --no-cache-dir --require-hashes -r /tmp/cmake-requirements.txt && rm /tmp/cmake-requirements.txt

# Boost 1.92, the release CI builds against, verified against the digest tools/setup-toolchain.sh pins; build.sh builds it.
ARG BOOST_VERSION=1.92.0
ARG BOOST_SHA256=f51707c27359a0df0cac1beada86de31bb5eed5e8285592dadec384df99c2984
RUN curl -fsSL "https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}-cmake.tar.gz" -o /tmp/boost.tar.gz \
    && echo "${BOOST_SHA256}  /tmp/boost.tar.gz" | sha256sum --check --quiet \
    && mkdir -p $SRC/boost \
    && tar -xzf /tmp/boost.tar.gz -C $SRC/boost --strip-components=1 \
    && rm /tmp/boost.tar.gz

COPY . $SRC/xstd-bits
WORKDIR $SRC/xstd-bits
COPY .clusterfuzzlite/build.sh $SRC/
