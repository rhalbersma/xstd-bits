#!/bin/bash -eu
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Boost with the builder's compiler and flags, so its one compiled library shares the fuzzers' libc++ and sanitizer.
cmake -S "$SRC/boost" -B "$WORK/boost-build" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
        -DBOOST_INCLUDE_LIBRARIES="container;dynamic_bitset;hash2" -DCMAKE_INSTALL_PREFIX="$WORK/boost"
cmake --build "$WORK/boost-build" --parallel "$(nproc)"
cmake --install "$WORK/boost-build"

# The sanitizer and the coverage instrumentation arrive in CXXFLAGS, and the engine in LIB_FUZZING_ENGINE. No build
# type, so CXXFLAGS alone sets the optimization and NDEBUG stays undefined: the library's asserts are findings too.
cmake -S "$SRC/xstd-bits" -B "$WORK/build" -DBUILD_TESTING=OFF \
        -DCMAKE_PREFIX_PATH="$WORK/boost" \
        -DXSTD_BITS_BUILD_FUZZERS=ON \
        -DXSTD_BITS_FUZZ_COMPILE_OPTIONS="" \
        -DXSTD_BITS_FUZZ_LINK_OPTIONS="$LIB_FUZZING_ENGINE"
cmake --build "$WORK/build" --parallel "$(nproc)" --target fuzz_bitset fuzz_set fuzz_vector

for target in fuzz_bitset fuzz_set fuzz_vector; do
        cp "$WORK/build/fuzz/$target" "$OUT/"
        # The checked-in seeds, which ClusterFuzzLite unpacks beside the corpus it keeps.
        (cd "$SRC/xstd-bits/fuzz/corpus/$target" && zip -q -j "$OUT/${target}_seed_corpus.zip" ./*)
done
