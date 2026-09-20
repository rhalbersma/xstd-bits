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
#   GCC 15, clang 22, libc++ 22, clang-format 22, and Boost's headers with Boost.Test.
#
# Set XSTD_TOOLCHAIN_FULL=1 to add GCC 16, the qualification rung. Its libstdc++ is the oldest carrying
# <inplace_vector>, so a repository with a C++26 leg needs it to reach that leg locally.
#
# This gets syntax-only checks, clang-tidy and the format gate, and it is what a comment or style sweep needs.
# It is not a test run: vcpkg.json's dependencies come through vcpkg, and CMake fetches any sibling xstd
# repository on top, which is a build worth starting deliberately rather than on every container start.

set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

readonly CLANG_VERSION=22
readonly GCC_VERSION=15
readonly CLANG_FORMAT_VERSION=22.1.8

# apt.llvm.org for clang, the toolchain PPA for a GCC newer than noble's. The PPA signs with RSA-1024, so apt
# warns about a weak algorithm on every update; that is the archive's key, not a fault in this script.
curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --batch --yes --dearmor -o /usr/share/keyrings/llvm.gpg
echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-${CLANG_VERSION} main" \
        > /etc/apt/sources.list.d/llvm.list

curl -fsSL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x60c317803a41ba51845e371a1e9377a2ba9ef27f" \
        | gpg --batch --yes --dearmor -o /usr/share/keyrings/ubuntu-toolchain.gpg
echo "deb [signed-by=/usr/share/keyrings/ubuntu-toolchain.gpg] https://ppa.launchpadcontent.net/ubuntu-toolchain-r/test/ubuntu noble main" \
        > /etc/apt/sources.list.d/ubuntu-toolchain.list

apt-get update -qq

packages=(
        "g++-${GCC_VERSION}"
        "clang-${CLANG_VERSION}"
        "clang-tidy-${CLANG_VERSION}"
        "libc++-${CLANG_VERSION}-dev"
        "libc++abi-${CLANG_VERSION}-dev"
        libboost-dev
        libboost-test-dev
)

if [[ "${XSTD_TOOLCHAIN_FULL:-0}" == "1" ]]; then
        packages+=("g++-16")
fi

apt-get install -y -qq "${packages[@]}"

# The format gate pins 22, and before 22 clang-format reads `{ a * b }` in a requires-expression as a pointer
# declaration. apt has no clang-format-22 for noble, so it comes from PyPI and lands in ~/.local/bin.
pip install --quiet --user --break-system-packages "clang-format==${CLANG_FORMAT_VERSION}"

"g++-${GCC_VERSION}" --version | head -1
"clang++-${CLANG_VERSION}" --version | head -1
"${HOME}/.local/bin/clang-format" --version
