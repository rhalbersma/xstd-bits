//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_fixed_set.hpp> // bit_fixed_set
#include <xstd/bits/bit_set.hpp>       // basic_bit_set, bit_set
#include <benchmark/benchmark.h>       // DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <cstddef>                     // size_t
#include <cstdint>                     // int64_t, uint8_t, uint16_t, uint32_t
#include <opt/set/sieve.hpp>           // filter_twins, sift_primes0, sift_primes1, sift_primes_incremental, sift_primes_segmented
#include <set>                         // set
#include <version>                     // __cpp_lib_flat_set
#if defined(__cpp_lib_flat_set)

#include <flat_set> // flat_set

#endif

// A built-in wide enough to name, a library mode that will own it, and a <bit> that will take it.
#if defined(__SIZEOF_INT128__) && !defined(__STRICT_ANSI__) && !defined(_MSC_VER)

#define BENCH_HAS_UINT128
#include <xstd/ints/cstdint/int128.hpp> // uint128

#endif

// The ladder doubles rather than stepping decades: bit_set changes block count on these boundaries.
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

} // namespace

template<class X>
auto bm_sift_primes0(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(opt::sift_primes0<X>(n));
        }
        per_candidate(state);
}

template<class X>
auto bm_sift_primes1(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(opt::sift_primes1<X>(n));
        }
        per_candidate(state);
}

// The segmented sieve holds O(sqrt(n) + W), against the bounded sieve's O(n), the window sized for L1.
template<class X>
auto bm_sift_primes_segmented(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(opt::sift_primes_segmented<X, xstd::bit_fixed_set<1UZ << 15>>(n));
        }
        per_candidate(state);
}

// The incremental sieve needs no bound: one map entry per prime, and a map lookup per candidate.
template<class X>
auto bm_sift_primes_incremental(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        for (auto _ : state) {
                benchmark::DoNotOptimize(opt::sift_primes_incremental<X>(n));
        }
        per_candidate(state);
}

// The sieve is the setup, not the measurement, so it stays outside the loop.
template<class X>
auto bm_filter_twins(benchmark::State& state)
        -> void
{
        auto const n = bound(state);
        auto const primes = opt::sift_primes1<X>(n);
        for (auto _ : state) {
                benchmark::DoNotOptimize(opt::filter_twins(primes));
        }
        per_candidate(state);
}

#define BENCH_LADDER(fn, type) \
        BENCHMARK_TEMPLATE1(fn, type)->RangeMultiplier(2)->Range(lo, hi)->Unit(benchmark::kMillisecond)

// The three representations: node-based, sorted-vector, and dense bitmap, one of each.
#if defined(__cpp_lib_flat_set)

// std::flat_set stops at 2^16, and the ceiling is a measurement decision before it is a budget one.
#define BENCH_QUADRATIC(fn, type) \
        BENCHMARK_TEMPLATE1(fn, type)->RangeMultiplier(2)->Range(lo, 1L << 16)->Unit(benchmark::kMillisecond)

#define BENCH_REPRESENTATIONS(fn) \
        BENCH_QUADRATIC(fn, std::flat_set<std::size_t>); \
        BENCH_LADDER(fn, std::set<std::size_t>); \
        BENCH_LADDER(fn, xstd::bit_set)

#else

#define BENCH_REPRESENTATIONS(fn) \
        BENCH_LADDER(fn, std::set<std::size_t>); \
        BENCH_LADDER(fn, xstd::bit_set)

#endif

// The second axis, ours alone: std::set and std::flat_set have no block to choose.
#if defined(BENCH_HAS_UINT128)

#define BENCH_BLOCKS(fn) \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint8_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint16_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint32_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<xstd::uint128>)

#else

#define BENCH_BLOCKS(fn) \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint8_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint16_t>); \
        BENCH_LADDER(fn, xstd::basic_bit_set<std::uint32_t>)

#endif

BENCH_REPRESENTATIONS(bm_sift_primes0);
BENCH_REPRESENTATIONS(bm_sift_primes1);
BENCH_REPRESENTATIONS(bm_filter_twins);

// The two unbounded sieves on the dense container alone: the algorithm is priced, not the container.
BENCH_LADDER(bm_sift_primes_segmented, xstd::bit_set);
BENCHMARK_TEMPLATE1(bm_sift_primes_incremental, xstd::bit_set)
        ->RangeMultiplier(2)
        ->Range(lo, 1L << 16)
        ->Unit(benchmark::kMillisecond);

BENCH_BLOCKS(bm_sift_primes0);
BENCH_BLOCKS(bm_sift_primes1);
BENCH_BLOCKS(bm_filter_twins);

BENCHMARK_MAIN();
