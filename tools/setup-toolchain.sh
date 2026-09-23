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

# A cloud setup script runs as root; a developer's shell does not.
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
        SUDO="sudo"
fi

readonly CLANG_VERSION=22
readonly GCC_VERSION=15

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
        libboost-dev
        libboost-test-dev
)

if [[ "${XSTD_TOOLCHAIN_FULL:-0}" == "1" ]]; then
        packages+=("g++-16")
fi

$SUDO apt-get install -y -qq "${packages[@]}"

# The format gate pins 22, and before 22 clang-format reads `{ a * b }` in a requires-expression as a pointer
# declaration. apt has no clang-format-22 for noble, so it comes from PyPI, pinned by hash in
# clang-format-requirements.txt beside this script, and lands in ~/.local/bin.
CLANG_FORMAT_REQUIREMENTS="$(dirname "${BASH_SOURCE[0]}")/clang-format-requirements.txt"
readonly CLANG_FORMAT_REQUIREMENTS
pip install --quiet --user --break-system-packages --require-hashes -r "${CLANG_FORMAT_REQUIREMENTS}" \
        || pip3 install --quiet --user --require-hashes -r "${CLANG_FORMAT_REQUIREMENTS}"

# Report what landed, but never fail a setup over a version banner: everything above has already
# installed by this point, and a caller that cannot print is not a caller that cannot build.
"g++-${GCC_VERSION}" --version | head -1 || true
"clang++-${CLANG_VERSION}" --version | head -1 || true
"${HOME:-/root}/.local/bin/clang-format" --version || true
