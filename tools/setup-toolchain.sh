#!/usr/bin/env bash
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Puts this repository's stable rung on a fresh container, for a Claude Code cloud environment's setup script or
# for any Ubuntu 24.04 box. Noble ships GCC 13 and clang 18: GCC 13 rejects -std=c++2c outright, so without this
# there is no local compiler that can build the library at all, and the apt clang-format reports files that are
# clean against .clang-format as dirty.
#
# Installed by default, matching the `stable` column of the README's matrix:
#
#   GCC 15, clang 22, libc++ 22, range-v3 and Google Benchmark from apt, clang-format 22 and CMake 4.4 from PyPI, and
#   Boost 1.92 built from its CMake release archive into XSTD_BOOST_PREFIX (default ~/.local/opt/boost-1.92.0).
#
# Set XSTD_TOOLCHAIN_FULL=1 to add GCC 16, the qualification rung. Its libstdc++ is the oldest carrying
# <inplace_vector>, so a repository with a C++26 leg needs it to reach that leg locally.
#
# Boost is the version CI's vcpkg resolves, not noble's 1.83, which predates Boost.Hash2 and answers questions about
# a Boost that CI never sees. It comes from the release's own CMake archive rather than through vcpkg, because a
# Claude Code cloud session's GitHub proxy serves release assets and git clones of public repositories but refuses
# their source archives, and every vcpkg Boost port downloads one. Boost's CMake build, unlike b2, installs a CMake
# package for each header-only library too, Boost.Hash2 among them. It builds once: re-running finds the install
# and skips it. A configure then points at the prefix:
#
#   cmake -S . -B build -DCMAKE_PREFIX_PATH="${XSTD_BOOST_PREFIX:-$HOME/.local/opt/boost-1.92.0}"
#
# Building Boost takes a few minutes on a fresh container; set XSTD_TOOLCHAIN_BOOST=0 to skip it when only
# syntax-only checks, clang-tidy or the format gate are wanted. CMake still fetches any sibling xstd repository
# at configure time.

set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# A cloud setup script runs as root; a developer's shell does not.
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
        SUDO="sudo"
fi

readonly CLANG_VERSION=22
readonly GCC_VERSION=15

# The release CI's vcpkg resolves, and the digest of its CMake archive, which the release does not publish itself.
readonly BOOST_VERSION=1.92.0
readonly BOOST_SHA256=f51707c27359a0df0cac1beada86de31bb5eed5e8285592dadec384df99c2984
BOOST_PREFIX="${XSTD_BOOST_PREFIX:-${HOME:-/root}/.local/opt/boost-${BOOST_VERSION}}"
readonly BOOST_PREFIX

# apt.llvm.org for clang, the toolchain PPA for a GCC newer than noble's. The PPA signs with RSA-1024, so apt
# warns about a weak algorithm on every update; that is the archive's key, not a fault in this script.
curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | $SUDO gpg --batch --yes --dearmor -o /usr/share/keyrings/llvm.gpg
echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-${CLANG_VERSION} main" \
        | $SUDO tee /etc/apt/sources.list.d/llvm.list > /dev/null

curl -fsSL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x60c317803a41ba51845e371a1e9377a2ba9ef27f" \
        | $SUDO gpg --batch --yes --dearmor -o /usr/share/keyrings/ubuntu-toolchain.gpg
echo "deb [signed-by=/usr/share/keyrings/ubuntu-toolchain.gpg] https://ppa.launchpadcontent.net/ubuntu-toolchain-r/test/ubuntu noble main" \
        | $SUDO tee /etc/apt/sources.list.d/ubuntu-toolchain.list > /dev/null

$SUDO apt-get update -qq

packages=(
        "g++-${GCC_VERSION}"
        "clang-${CLANG_VERSION}"
        "clang-tidy-${CLANG_VERSION}"
        "libc++-${CLANG_VERSION}-dev"
        "libc++abi-${CLANG_VERSION}-dev"
        libbenchmark-dev
        librange-v3-dev
        # What the Boost build below calls for, beside the CMake from PyPI.
        curl
        ninja-build
)

if [[ "${XSTD_TOOLCHAIN_FULL:-0}" == "1" ]]; then
        packages+=("g++-16")
fi

$SUDO apt-get install -y -qq "${packages[@]}"

# The format gate pins 22, and before 22 clang-format reads `{ a * b }` in a requires-expression as a pointer
# declaration. CMakeLists.txt requires CMake 3.30, and noble's is 3.28. apt has neither, so both come from PyPI,
# pinned by hash in clang-format-requirements.txt and cmake-requirements.txt beside this script, and land in
# ~/.local/bin.
CLANG_FORMAT_REQUIREMENTS="$(dirname "${BASH_SOURCE[0]}")/clang-format-requirements.txt"
readonly CLANG_FORMAT_REQUIREMENTS
CMAKE_REQUIREMENTS="$(dirname "${BASH_SOURCE[0]}")/cmake-requirements.txt"
readonly CMAKE_REQUIREMENTS
pip install --quiet --user --break-system-packages --require-hashes -r "${CLANG_FORMAT_REQUIREMENTS}" -r "${CMAKE_REQUIREMENTS}" \
        || pip3 install --quiet --user --require-hashes -r "${CLANG_FORMAT_REQUIREMENTS}" -r "${CMAKE_REQUIREMENTS}"
CMAKE="${HOME:-/root}/.local/bin/cmake"
readonly CMAKE

# Boost's CMake release archive, verified against the pinned digest, with vcpkg.json's four libraries and what they
# depend on built once into BOOST_PREFIX.
if [[ "${XSTD_TOOLCHAIN_BOOST:-1}" == "1" && ! -f "${BOOST_PREFIX}/lib/cmake/boost_hash2-${BOOST_VERSION}/boost_hash2-config.cmake" ]]; then
        BOOST_ARCHIVE="boost-${BOOST_VERSION}-cmake.tar.gz"
        BOOST_WORK="$(mktemp -d)"
        curl -fsSL "https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/${BOOST_ARCHIVE}" -o "${BOOST_WORK}/${BOOST_ARCHIVE}"
        echo "${BOOST_SHA256}  ${BOOST_WORK}/${BOOST_ARCHIVE}" | sha256sum --check --quiet
        tar -xzf "${BOOST_WORK}/${BOOST_ARCHIVE}" -C "${BOOST_WORK}"
        "${CMAKE}" -S "${BOOST_WORK}/boost-${BOOST_VERSION}" -B "${BOOST_WORK}/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
                -DBUILD_SHARED_LIBS=OFF -DBOOST_INCLUDE_LIBRARIES="container;dynamic_bitset;hash2;test" \
                -DCMAKE_INSTALL_PREFIX="${BOOST_PREFIX}" > /dev/null
        "${CMAKE}" --build "${BOOST_WORK}/build" > /dev/null
        "${CMAKE}" --install "${BOOST_WORK}/build" > /dev/null
        rm -rf "${BOOST_WORK}"
fi

# Report what landed, but never fail a setup over a version banner: everything above has already
# installed by this point, and a caller that cannot print is not a caller that cannot build.
"g++-${GCC_VERSION}" --version | head -1 || true
"clang++-${CLANG_VERSION}" --version | head -1 || true
"${HOME:-/root}/.local/bin/clang-format" --version || true
"${CMAKE}" --version | head -1 || true
if [[ "${XSTD_TOOLCHAIN_BOOST:-1}" == "1" ]]; then
        grep -m1 'define BOOST_LIB_VERSION' "${BOOST_PREFIX}/include/boost/version.hpp" || true
fi
