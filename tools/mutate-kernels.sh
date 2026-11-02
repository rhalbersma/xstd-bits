#!/usr/bin/env bash
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Mutation testing over the block kernels under include/xstd/bits/detail/, as the Mutation workflow runs it.
#
#   tools/mutate-kernels.sh [jobs]
#
# The kernels branch on the instructions they are compiled for, so the tests are built for a fixed level rather than for
# the machine at hand: x86-64-v3, with PDEP and PEXT and without AVX-512, compiles in every path but the portable one,
# and x86-64 compiles in that one. Each header is run against the tests that exercise it, not against every test that
# includes it: a kernel header reaches some 200 tests, most of which never call it. A group's tests are those that
# killed one of its mutants; a survivor there is a gap or an equivalent mutant, and doc/mutation.md lists the latter.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
jobs="${1:-$(nproc)}"
cmake="$(command -v "${HOME}/.local/bin/cmake" || command -v cmake)"
detail=xstd/bits/detail
status=0

headers() {
        local h
        for h in "$@"; do printf '%s/%s.hpp\n' "${detail}" "${h}"; done
}

# $1 the -march level; the rest the test targets to build.
build() {
        local arch="$1" dir="${root}/build/mutate-$1"
        shift
        # Clang 22 on GCC 15's libstdc++, its own rung's, rather than the newest GCC the toolchain installs.
        "${cmake}" -S "${root}" -B "${dir}" -G Ninja -DCMAKE_CXX_COMPILER=clang++-22 -DCMAKE_BUILD_TYPE=Debug \
                -DCMAKE_CXX_FLAGS=--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15 -DCMAKE_CXX_FLAGS_DEBUG=-O0 \
                -DBUILD_TESTING=ON -DXSTD_BITS_WARNINGS_AS_ERRORS=OFF -DXSTD_BITS_BUILD_BENCHMARKS=OFF \
                -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXSTD_BITS_TEST_ARCH="${arch}" > /dev/null
        "${cmake}" --build "${dir}" -j "${jobs}" --target "$@"
}

# $1 the -march level, $2 the test targets as a list, the rest the headers.
mutate() {
        local arch="$1" tests="$2"
        shift 2
        local pattern
        pattern="^($(tr ' ' '|' <<< "${tests}"))\$"
        echo "== ${arch}: $*"
        python3 "${root}/tools/mutate.py" "$@" --build "${root}/build/mutate-${arch}" --jobs "${jobs}" --max-targets 0 --tests "${pattern//./\\.}" || status=1
}

# The valarray's gathers, scatters and masks.
valarray="test.bits.detail.gather_scatter test.bits.bit_valarray.bit_valarray test.bits.bit_valarray.bit_fixed_valarray.subscripts test.bits.bit_valarray.bit_inplace_valarray test.bits.bit_valarray.bit_slice_array"
# The block walks, shifts and intrinsics, and the hash that expands a block into bools.
blocks="test.bits.common_vocabulary test.bits.detail.bit_layout test.bits.detail.bit_block_container test.bits.bit_set.bit_fixed_set test.bits.detail.hash"
# The comparisons.
compare="test.bits.bit_algorithm.bit_equal test.bits.bit_algorithm.bit_mismatch test.bits.bit_algorithm.bit_lexicographical_compare_three_way test.bits.bit_functional.bit_equal_to"
# The portable compress and expand, which a full mask skips.
portable="test.bits.detail.gather_scatter"

# shellcheck disable=SC2086 # each list is the targets, one word apiece
build x86-64-v3 ${valarray} ${blocks} ${compare}
mapfile -t v3_valarray < <(headers bit_at bit_stream block_width compress_blocks expand_blocks expand_indices expand_run fill_blocks fill_indices fill_run gather_indices gather_run scatter slice_masks stored_masks word_masks)
mutate x86-64-v3 "${valarray}" "${v3_valarray[@]}"
mapfile -t v3_blocks < <(headers block_intersects intrinsic shift walk_block)
mutate x86-64-v3 "${blocks}" "${v3_blocks[@]}"
mapfile -t v3_compare < <(headers first_mismatch storage_compare)
mutate x86-64-v3 "${compare}" "${v3_compare[@]}"

# shellcheck disable=SC2086
build x86-64 ${portable}
mapfile -t x86_64 < <(headers compressed expanded)
mutate x86-64 "${portable}" "${x86_64[@]}"

exit "${status}"
