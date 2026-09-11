//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set.hpp>        // basic_bit_set, bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <benchmark/benchmark.h>        // DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <opt/set/sieve.hpp>            // filter_twins, sift_primes0, sift_primes1, sift_primes_incremental, sift_primes_segmented
#include <cstddef>                      // size_t
#include <cstdint>                      // int64_t, uint8_t, uint16_t, uint32_t
#include <set>                          // set
#include <version>                      // __cpp_lib_flat_set
#if defined(__cpp_lib_flat_set)
#include <flat_set> // flat_set
#endif

// A built-in wide enough to name, a library mode that will own it, and a <bit> that will take it. [design.md#uint128-support]
#if defined(__SIZEOF_INT128__) && !defined(__STRICT_ANSI__) && !defined(_MSC_VER)
#define BENCH_HAS_UINT128
#include <xstd/ints/cstdint/int128.hpp> // uint128
#endif

// The ladder doubles rather than stepping decades: bit_set changes block count on these boundaries, so a
// doubling walks whole blocks, and a cache knee reads as a knee. [design.md#the-sieve]
inline constexpr auto lo = 1L << 10;
inline constexpr auto hi = 1L << 20;

namespace {

auto bound(benchmark::State const& state)
        -> std::size_t
{
        return static_cast<std::size_t>(state.range(0));
}

// Reported per candidate rather than per run, so the rungs are comparable down the ladder and not only across it.
auto per_candidate(benchmark::State& state)
        -> void
{
        state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(bound(state)));
}

}       // namespace

template<class X>
auto bm_sift_primes0(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(xstd::sift_primes0<X>(n));
        }
        per_candidate(state);
}

template<class X>
auto bm_sift_primes1(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(xstd::sift_primes1<X>(n));
        }
        per_candidate(state);
}

// The segmented sieve holds O(sqrt(n) + W) whatever n is, against the bounded sieve's O(n), and the window is a
// compile-time width chosen to sit in L1. What the ladder is asked here is what that costs in time.
// [design.md#the-unbounded-sieves]
template<class X>
auto bm_sift_primes_segmented(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(xstd::sift_primes_segmented<X, xstd::bit_static_set<1UZ << 15>>(n));
        }
        per_candidate(state);
}

// The incremental sieve is the price of needing no bound at all: one map entry per prime found, and a map lookup
// per candidate where the array sieve has a strided write. O'Neill's own point is that it is slower; this is the
// measurement of how much. [design.md#the-unbounded-sieves]
template<class X>
auto bm_sift_primes_incremental(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(xstd::sift_primes_incremental<X>(n));
        }
        per_candidate(state);
}

// The sieve is the setup, not the measurement, so it stays outside the loop; at the top rung it costs more than
// the twins pass it feeds.
template<class X>
auto bm_filter_twins(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        auto const primes = xstd::sift_primes1<X>(n);
        for (auto _ : state) {
                benchmark::DoNotOptimize(xstd::filter_twins(primes));
        }
        per_candidate(state);
}

#define BENCH_LADDER(fn, type) \
        BENCHMARK_TEMPLATE1(fn, type)->RangeMultiplier(2)->Range(lo, hi)->Unit(benchmark::kMillisecond)

// The three representations: node-based, sorted-vector, and dense bitmap, one of each. Dynamic containers only,
// so the bench compares like with like: a set sized for its universe is not measuring what a growing one is.
// [design.md#the-sieve]
#if defined(__cpp_lib_flat_set)

// std::flat_set stops at 2^16, and the ceiling is a measurement decision before it is a budget one. Its sift is
// quadratic -- erase on a sorted vector is linear and the sieve does about n log log n of them -- so each rung
// past that costs five times the last and establishes nothing the curve has not already shown. Carrying it to
// 2^20 would spend six minutes of every Release ctest run to re-derive a slope visible four rungs earlier.
// Defined inside the guard rather than beside BENCH_LADDER: where there is no <flat_set> there is no user, and
// -Weverything answers -Wunused-macros.
#define BENCH_QUADRATIC(fn, type) \
        BENCHMARK_TEMPLATE1(fn, type)->RangeMultiplier(2)->Range(lo, 1L << 16)->Unit(benchmark::kMillisecond)

#define BENCH_REPRESENTATIONS(fn)                          \
        BENCH_QUADRATIC(fn, std::flat_set<std::size_t>);   \
        BENCH_LADDER(fn, std::set<std::size_t>);           \
        BENCH_LADDER(fn, xstd::bit_set)
#else
#define BENCH_REPRESENTATIONS(fn)                          \
        BENCH_LADDER(fn, std::set<std::size_t>);           \
        BENCH_LADDER(fn, xstd::bit_set)
#endif

// The second axis, ours alone: std::set and std::flat_set have no block to choose. The footprint at a given n is
// the same count of bits whatever the block, so what this varies is the block count against the cost per block --
// the sift is a strided write and near width-indifferent, the scans are not. bit_set is the size_t rung already.
#if defined(BENCH_HAS_UINT128)
#define BENCH_BLOCKS(fn)                                   \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint8_t >); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint16_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint32_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<xstd::uint128>)
#else
#define BENCH_BLOCKS(fn)                                   \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint8_t >); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint16_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint32_t>)
#endif

BENCH_REPRESENTATIONS(bm_sift_primes0);
BENCH_REPRESENTATIONS(bm_sift_primes1);
BENCH_REPRESENTATIONS(bm_filter_twins);

// The two unbounded sieves, on the dense container alone: what is being priced is the algorithm against
// sift_primes1 on the same row, not one container against another. The segmented one runs the full ladder, being
// linearithmic and cheap; the incremental one stops at 2^16 for the same reason std::flat_set does -- a map lookup
// per candidate is a large constant, and the curve is decided long before the top rung.
// [design.md#the-unbounded-sieves]
BENCH_LADDER(bm_sift_primes_segmented, xstd::bit_set);
BENCHMARK_TEMPLATE1(bm_sift_primes_incremental, xstd::bit_set)
        ->RangeMultiplier(2)->Range(lo, 1L << 16)->Unit(benchmark::kMillisecond);

BENCH_BLOCKS(bm_sift_primes0);
BENCH_BLOCKS(bm_sift_primes1);
BENCH_BLOCKS(bm_filter_twins);

BENCHMARK_MAIN();
