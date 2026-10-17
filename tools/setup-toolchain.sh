#!/usr/bin/env bash
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Brings a fresh container as close to the GitHub Actions ladders as an Ubuntu 24.04 box gets, for a Claude Code
# cloud environment's setup script or for any noble machine. Noble ships GCC 13, clang 18, CMake 3.28 and Boost
# 1.83: GCC 13 rejects -std=c++2c outright, CMakeLists.txt asks for CMake 3.30, the apt clang-format reports files
# that are clean against .clang-format as dirty, and Boost 1.83 predates Boost.Hash2, which the library's headers
# include.
#
# Installed, matching the three rungs of the README's matrix and the Boost CI resolves:
#
#   GCC 15, 16 and the trunk snapshot as g++-17; clang 22, 23 and trunk as clang++-24, with clang-tidy for each;
#   clang-format 22, the format gate's one rung; libc++ 22; GCC 15's 32-bit multilib for the i686 leg; range-v3 and
#   Google Benchmark from apt; CMake, gcovr and the same clang-format 22 from PyPI; and Boost 1.92's Container,
#   Dynamic Bitset, Hash2, Test and Unordered, built from Boost's release tarball by GCC 15 into /usr/local.
#
# A compiler on libc++, on the trunk snapshot's libstdc++ or on a 32-bit target links a Boost.Test of its own, which
# boost-test.sh beside this script builds; CI gets one from vcpkg, which builds Boost with the compiler under test.
# The unit tests, clang-tidy and the format gate then all run locally, and ci-local.sh beside this script runs them
# leg by leg. CMake fetches xstd-ints, xstd-misc and the test suite's pre-release dependencies, such as Boost.Int128,
# from GitHub at configure time.

set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# A cloud setup script runs as root; a developer's shell does not.
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
        SUDO="sudo"
fi

readonly CLANG_VERSION=22
readonly GCC_VERSION=15

# The rungs above the stable one, as the shared workflows' toolchain table names them.
readonly GCC_QUALIFICATION_VERSION=16
readonly GCC_TRUNK_VERSION=17
readonly CLANG_QUALIFICATION_VERSION=23
readonly CLANG_TRUNK_VERSION=24

# The Boost that vcpkg resolves for CI, pinned by the SHA-256 Boost publishes beside the tarball.
readonly BOOST_VERSION=1.92.0
readonly BOOST_SHA256=9bed76128d4e46755dbe818487788c6fceb6f72b378f4daa49b7e1e600d9088d
readonly BOOST_PREFIX=/usr/local

# vcpkg.json's Boost libraries, the test feature's included, and what Boost's CMake build adds for them.
readonly BOOST_LIBRARIES="container;dynamic_bitset;hash2;test;unordered"

# apt.llvm.org for clang, the toolchain PPA for a GCC newer than noble's. The PPA signs with RSA-1024, so apt
# warns about a weak algorithm on every update; that is the archive's key, not a fault in this script.
curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | $SUDO gpg --batch --yes --dearmor -o /usr/share/keyrings/llvm.gpg
{
        echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-${CLANG_VERSION} main"
        echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-${CLANG_QUALIFICATION_VERSION} main"
        echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble main"
} | $SUDO tee /etc/apt/sources.list.d/llvm.list > /dev/null

curl -fsSL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x60c317803a41ba51845e371a1e9377a2ba9ef27f" \
        | $SUDO gpg --batch --yes --dearmor -o /usr/share/keyrings/ubuntu-toolchain.gpg
echo "deb [signed-by=/usr/share/keyrings/ubuntu-toolchain.gpg] https://ppa.launchpadcontent.net/ubuntu-toolchain-r/test/ubuntu noble main" \
        | $SUDO tee /etc/apt/sources.list.d/ubuntu-toolchain.list > /dev/null

$SUDO apt-get update -qq

packages=(
        "g++-${GCC_VERSION}"
        "clang-${CLANG_VERSION}"
        "clang-tidy-${CLANG_VERSION}"
        "clang-format-${CLANG_VERSION}"
        "libc++-${CLANG_VERSION}-dev"
        "libc++abi-${CLANG_VERSION}-dev"
        "g++-${GCC_QUALIFICATION_VERSION}"
        "g++-${GCC_VERSION}-multilib"
        # The test suite's range-v3 and the benchmark tree's Google Benchmark, which vcpkg.json's test feature names.
        libbenchmark-dev
        librange-v3-dev
        # What the builds below and ci-local.sh call for, beside the CMake from PyPI.
        curl
        jq
        ninja-build
)

# Each libc++ leg has a runner to itself, but apt's libc++ packages exclude one another, so libc++ stays at the
# stable rung's here; `apt-get install libc++-24-dev libc++abi-24-dev` swaps it for a trunk libc++ leg.
for version in "${CLANG_QUALIFICATION_VERSION}" "${CLANG_TRUNK_VERSION}"; do
        packages+=("clang-${version}" "clang-tidy-${version}")
done

$SUDO apt-get install -y -qq "${packages[@]}"

# GCC trunk is Jonathan Wakely's weekly binary snapshot, the one the ladders' development rung installs, under the
# same g++-17 name. Upstream publishes neither checksums nor signatures, so what arrived is recorded instead.
snapshot="$(mktemp -d)"
curl -fsSL --retry 3 https://kayari.org/gcc-latest/gcc-latest.deb -o "${snapshot}/gcc-latest.deb"
dpkg-deb -f "${snapshot}/gcc-latest.deb" Package Version
sha256sum "${snapshot}/gcc-latest.deb"
$SUDO dpkg -i "${snapshot}/gcc-latest.deb" > /dev/null || $SUDO apt-get install -f -y -qq
rm -rf "${snapshot}"
for tool in gcc g++ gcov; do
        $SUDO ln -sf "/opt/gcc-latest/bin/${tool}" "/usr/bin/${tool}-${GCC_TRUNK_VERSION}"
done
printf '%s\n' /opt/gcc-latest/lib /opt/gcc-latest/lib64 | $SUDO tee /etc/ld.so.conf.d/gcc-latest.conf > /dev/null
$SUDO ldconfig

# The format gate runs clang-format-22 from apt.llvm.org, installed above, at the stable rung alone: a formatter
# release reformats the code base. Before 22 it reads `{ a * b }` in a requires-expression as a pointer declaration.
# The same 22 also comes from PyPI as an unversioned clang-format, and CMake comes from there too, since noble's
# 3.28 is older than the 3.30 CMakeLists.txt asks for, as does the coverage job's gcovr. All three are pinned by hash
# in clang-format-requirements.txt, cmake-requirements.txt and gcovr-requirements.txt beside this script, and land in
# ~/.local/bin.
CLANG_FORMAT_REQUIREMENTS="$(dirname "${BASH_SOURCE[0]}")/clang-format-requirements.txt"
readonly CLANG_FORMAT_REQUIREMENTS
CMAKE_REQUIREMENTS="$(dirname "${BASH_SOURCE[0]}")/cmake-requirements.txt"
readonly CMAKE_REQUIREMENTS
GCOVR_REQUIREMENTS="$(dirname "${BASH_SOURCE[0]}")/gcovr-requirements.txt"
readonly GCOVR_REQUIREMENTS
for requirements in "${CLANG_FORMAT_REQUIREMENTS}" "${CMAKE_REQUIREMENTS}" "${GCOVR_REQUIREMENTS}"; do
        pip install --quiet --user --break-system-packages --require-hashes -r "${requirements}" \
                || pip3 install --quiet --user --require-hashes -r "${requirements}"
done
readonly CMAKE="${HOME:-/root}/.local/bin/cmake"

# Boost from its CMake release tarball rather than through vcpkg, whose every Boost port downloads a source archive
# that a Claude Code cloud session's proxy refuses, though it serves release assets. Boost's CMake build, unlike b2,
# installs a CMake package for each header-only library too, Boost.Hash2 among them. Only what the suite uses is
# built, by GCC 15, and a rerun skips a done install.
if [ ! -f "${BOOST_PREFIX}/lib/cmake/Boost-${BOOST_VERSION}/BoostConfig.cmake" ]; then
        boost="$(mktemp -d)"
        curl -fsSL --retry 3 "https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}-cmake.tar.xz" \
                -o "${boost}/boost.tar.xz"
        echo "${BOOST_SHA256}  ${boost}/boost.tar.xz" | sha256sum --check --quiet
        tar -xf "${boost}/boost.tar.xz" -C "${boost}"
        "${CMAKE}" -S "${boost}/boost-${BOOST_VERSION}" -B "${boost}/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_CXX_COMPILER="g++-${GCC_VERSION}" -DBUILD_SHARED_LIBS=OFF \
                -DBOOST_INCLUDE_LIBRARIES="${BOOST_LIBRARIES}" > /dev/null
        "${CMAKE}" --build "${boost}/build" --parallel "$(nproc)" > /dev/null
        $SUDO "${CMAKE}" --install "${boost}/build" --prefix "${BOOST_PREFIX}" > /dev/null
        rm -rf "${boost}"
fi

# Report what landed, but never fail a setup over a version banner: everything above has already
# installed by this point, and a caller that cannot print is not a caller that cannot build.
"g++-${GCC_VERSION}" --version | head -1 || true
"clang++-${CLANG_VERSION}" --version | head -1 || true
"${HOME:-/root}/.local/bin/clang-format" --version || true
"${CMAKE}" --version | head -1 || true
PATH="${HOME:-/root}/.local/bin:${PATH}" gcovr --version | head -1 || true
echo "Boost ${BOOST_VERSION} in ${BOOST_PREFIX}"
for compiler in "g++-${GCC_QUALIFICATION_VERSION}" "g++-${GCC_TRUNK_VERSION}" "clang++-${CLANG_QUALIFICATION_VERSION}" "clang++-${CLANG_TRUNK_VERSION}"; do
        "${compiler}" --version | head -1 || true
done
