#!/usr/bin/env bash
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Runs the GitHub Actions ladders' Linux legs on what setup-toolchain.sh installs, so a change is green here before it
# waits on CI's runners. Each leg does what its job does: configure, build and test the suite on one compiler, or run
# clang-tidy, the coverage gate, the format gate or the audits. Each leg works under build/ci-local/<leg>, logs to
# build/ci-local/<leg>.log, and the run ends in one line per leg.
#
#   tools/ci-local.sh                     every leg, one after another, each on every core
#   tools/ci-local.sh gcc-17 tidy-24      the legs named
#   tools/ci-local.sh --list              the legs
#
# CI_LOCAL_BUILD_TYPE picks the suite legs' build type, Debug by default; CI runs both Debug and Release. A suite
# leg's build tree, some 3 GB, is removed once the leg passes, and kept when it fails; CI_LOCAL_KEEP=1 keeps it either
# way.
#
# Not here, having no Linux equivalent: MSVC, clang-cl, MinGW, Apple Clang and Emscripten. Nor the sanitizer,
# CodeQL, consumption, fuzzing and mutation jobs, whose own commands CONTRIBUTING.md gives.

set -uo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly root
readonly out="${root}/build/ci-local"
readonly build_type="${CI_LOCAL_BUILD_TYPE:-Debug}"
readonly keep="${CI_LOCAL_KEEP:-0}"
jobs="$(nproc)"
readonly jobs
cmake="${HOME:-/root}/.local/bin/cmake"
[ -x "${cmake}" ] || cmake="cmake"
readonly cmake

readonly all_legs=(gcc-15 gcc-16 gcc-17 cxx26-16 cxx26-17 clang-22 clang-23 clang-24 libcxx-22 i686
                   hardened-gcc hardened-libcxx tidy-22 tidy-23 tidy-24 coverage format audit)

# Each clang uses the libstdc++ of its own rung, as on the runners, rather than the newest GCC installed.
gcc_for_clang() {
        case "$1" in
                22) echo "--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15" ;;
                23) echo "--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/16" ;;
                24) echo "--gcc-toolchain=/opt/gcc-latest" ;;
        esac
}

# Configure, build and test the suite as the ladders' jobs do: <leg> <compiler> <flags> <build type> [CMake args...]
suite() {
        local leg="$1" cxx="$2" flags="$3" type="$4"
        shift 4
        local dir="${out}/${leg}"
        "${cmake}" -S "${root}" -B "${dir}" -G Ninja -DCMAKE_BUILD_TYPE="${type}" -DCMAKE_CXX_COMPILER="${cxx}" \
                -DCMAKE_CXX_FLAGS="${flags}" -DCMAKE_CXX_FLAGS_DEBUG=-g1 -DCMAKE_CXX_SCAN_FOR_MODULES=OFF "$@" \
                && "${cmake}" --build "${dir}" --parallel "${jobs}" \
                && ctest --test-dir "${dir}" --output-on-failure --parallel "${jobs}" \
                && { [ "${keep}" = 1 ] || rm -rf "${dir}"; }
}

# A suite leg on another standard library or the trunk libstdc++ links a Boost.Test built for it.
boost_test_for() {
        local name="$1"
        shift
        "${root}/tools/boost-test.sh" "${out}/boost-test-${name}" "$@"
}

# The clang-tidy job's passes over the compile database, and its check of each public header on its own, with the
# directories the job names as adapted_dirs.
tidy() {
        local version="$1" dir="${out}/tidy-$1" flags std status=0
        flags="$(gcc_for_clang "${version}")"
        "${cmake}" -S "${root}" -B "${dir}" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="clang++-${version}" \
                -DCMAKE_CXX_FLAGS="${flags}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_SCAN_FOR_MODULES=OFF || return 1
        std="$(jq -r '.[0].command // (.[0].arguments | join(" "))' "${dir}/compile_commands.json" | grep -o -- '-std=[^ ]*' | head -1)"
        "run-clang-tidy-${version}" -quiet -p "${dir}" "${dir}/test/header_self_sufficiency/.*" || status=1
        while IFS= read -r -d '' header; do
                # shellcheck disable=SC2086 # flags is one option or none
                "clang-tidy-${version}" --quiet "${header}" -- -x c++-header "${std}" -I"${root}/include" ${flags} \
                        -isystem "${dir}/_deps/xstd-ints-src/include" -isystem "${dir}/_deps/xstd-misc-src/include" || status=1
        done < <(find "${root}/include" -type f -name '*.hpp' -print0)
        "run-clang-tidy-${version}" -quiet -p "${dir}" "${root}/test/src/.*" || status=1
        return "${status}"
}

# The coverage job: the stable rung's GCC with --coverage, the stub's arguments and excludes, and the workflow's gcovr
# filters, failing below 100% of lines or branches. gcovr reads only this leg's tree, not the other legs' beside it.
coverage() {
        local dir="${out}/coverage"
        local gcovr="${HOME:-/root}/.local/bin/gcovr"
        [ -x "${gcovr}" ] || gcovr="gcovr"
        "${cmake}" -S "${root}" -B "${dir}" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++-15 \
                -DCMAKE_CXX_FLAGS="--coverage -O0" -DCMAKE_EXE_LINKER_FLAGS=--coverage -DCMAKE_CXX_FLAGS_DEBUG=-g1 \
                -DCMAKE_CXX_SCAN_FOR_MODULES=OFF -DXSTD_BITS_BUILD_BENCHMARKS=OFF -DXSTD_BITS_BUILD_HARDENED_TESTS=OFF \
                && "${cmake}" --build "${dir}" --parallel "${jobs}" \
                && ctest --test-dir "${dir}" --output-on-failure --parallel "${jobs}" \
                && "${gcovr}" --root "${root}" "${dir}" --gcov-executable gcov-15 --merge-lines \
                        --exclude 'test/.*' --exclude 'benchmark/.*' --exclude 'build/.*' \
                        --exclude-lines-by-pattern '^\s*(assert|[A-Z][A-Z0-9_]*_ASSERT)\(' \
                        --exclude-lines-by-pattern '=\s*default;' \
                        --exclude-lines-by-pattern '\s*\}\s*$' \
                        --exclude-branches-by-pattern '^\s*(assert|[A-Z][A-Z0-9_]*_ASSERT)\(' \
                        --exclude-branches-by-pattern '^\s*.*=\s*default;' \
                        --exclude-throw-branches --exclude-unreachable-branches \
                        --print-summary --fail-under-line 100 --fail-under-branch 100 --txt "${out}/coverage.txt" \
                && { [ "${keep}" = 1 ] || rm -rf "${dir}"; }
}

# The format gate: the stable rung's clang-format over the paths its job names, reporting rather than rewriting.
format() {
        find "${root}/examples" "${root}/include" "${root}/test" "${root}/benchmark" -regex '.*\.\(hpp\|cpp\)' -print0 \
                | xargs -0 clang-format-22 --dry-run --Werror
}

# The Audit job: the paragraph inventories against the citations, and the include comments against the code.
audit() {
        python3 "${root}/tools/audit.py" && python3 "${root}/tools/include_comments.py"
}

run_leg() {
        local boost_test
        case "$1" in
                # The one ladder that builds the benchmark tree, whose Release legs run it as a smoke test.
                gcc-1[56]) suite "$1" "g++-${1#gcc-}" "" "${build_type}" -DXSTD_BITS_BUILD_BENCHMARKS=ON ;;
                # The trunk libstdc++ is newer than the GCC 15 that built the installed Boost.Test.
                gcc-17)
                        boost_test="$(boost_test_for gcc-17 g++-17)" || return 1
                        suite "$1" g++-17 "" "${build_type}" -DXSTD_BITS_BUILD_BENCHMARKS=ON "${boost_test}" ;;
                # The inplace column over std::inplace_vector, on the two rungs whose libstdc++ has it.
                cxx26-16) suite "$1" g++-16 "" "${build_type}" -DXSTD_BITS_CXX_STANDARD=26 ;;
                cxx26-17)
                        boost_test="$(boost_test_for gcc-17 g++-17)" || return 1
                        suite "$1" g++-17 "" "${build_type}" -DXSTD_BITS_CXX_STANDARD=26 "${boost_test}" ;;
                clang-2[23]) suite "$1" "clang++-${1#clang-}" "$(gcc_for_clang "${1#clang-}")" "${build_type}" ;;
                clang-24)
                        boost_test="$(boost_test_for clang-24 clang++-24 "$(gcc_for_clang 24)")" || return 1
                        suite "$1" clang++-24 "$(gcc_for_clang 24)" "${build_type}" "${boost_test}" ;;
                libcxx-22)
                        boost_test="$(boost_test_for libcxx-22 clang++-22 -stdlib=libc++)" || return 1
                        suite "$1" clang++-22 "-stdlib=libc++" "${build_type}" -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++ "${boost_test}" ;;
                # One rung in Debug, as the i686 job runs it, against the Boost setup-toolchain.sh builds at -m32.
                i686) suite "$1" g++-15 "-m32" Debug -DCMAKE_PREFIX_PATH=/usr/local/i686 ;;
                # One rung in Release against each hardened standard library, as the hardened jobs run them.
                hardened-gcc) suite "$1" g++-15 "-D_GLIBCXX_ASSERTIONS" Release ;;
                hardened-libcxx)
                        boost_test="$(boost_test_for libcxx-22 clang++-22 -stdlib=libc++)" || return 1
                        suite "$1" clang++-22 "-stdlib=libc++ -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST" Release \
                                -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++ "${boost_test}" ;;
                tidy-2[234]) tidy "${1#tidy-}" ;;
                coverage) coverage ;;
                format) format ;;
                audit) audit ;;
                *) echo "no leg named $1; tools/ci-local.sh --list names them" >&2; return 2 ;;
        esac
}

if [ "${1:-}" = "--list" ]; then
        printf '%s\n' "${all_legs[@]}"
        exit 0
fi
legs=("$@")
[ "${#legs[@]}" -gt 0 ] || legs=("${all_legs[@]}")

mkdir -p "${out}"
results=()
failed=0
for leg in "${legs[@]}"; do
        start="$(date +%s)"
        echo "== ${leg}"
        if run_leg "${leg}" > "${out}/${leg}.log" 2>&1; then
                verdict="pass"
        else
                verdict="FAIL"
                failed=1
        fi
        results+=("$(printf '%-4s %-15s %5ss  %s' "${verdict}" "${leg}" "$(( $(date +%s) - start ))" "${out}/${leg}.log")")
done
printf '%s\n' "${results[@]}"
exit "${failed}"
