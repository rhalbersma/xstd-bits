//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The sequence row: xstd::bit_vector against std::vector<bool>, which is the whole comparison. One reading, one
// storage, one variable -- the Standard's variable-size sequence of bool against ours. boost::dynamic_bitset is
// not on this row: it is a bitset, and it is compared against xstd::dynamic_bitset in bitset/dynamic.cpp where
// that is apples to apples. [design.md#the-sequence-ladder]
//
// The ladder differs from the bitset row's on purpose. A bitset is a bitboard question -- ALU-bound, a handful of
// words, 1 to 512. A sequence of bool is an endgame-database question: a dense flat array indexed by a ranked
// position, where a lookup costs a cache miss and nothing else. So this one runs 8 KiB to 32 MiB, through L1, L2,
// L3 and into DRAM, and reports latency per random read rather than bytes per second.

#include <benchmark/benchmark.h>        // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <xstd/bits/bit_vector.hpp>     // bit_vector
#include <algorithm>                    // count
#include <cstddef>                      // size_t
#include <cstdint>                      // int64_t, uint64_t
#include <vector>                       // vector

namespace {

inline constexpr auto bits_per_word = 64UZ;

auto bits(benchmark::State const& state)
        -> std::size_t
{
        return static_cast<std::size_t>(state.range(0)) * bits_per_word;
}

// One step of an LCG per lookup: a couple of nanoseconds against a DRAM miss, and unpredictable enough that the
// prefetcher cannot turn the random walk back into a sequential one, which is the whole point of measuring it.
constexpr auto next_index(std::uint64_t& lcg, std::size_t n)
        -> std::size_t
{
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<std::size_t>(lcg >> 33) % n;
}

template<class T>
auto filled(std::size_t n)
        -> T
{
        auto v = T(n);
        auto lcg = std::uint64_t{1};
        for (auto i = 0UZ; i < n; ++i) {
                v[i] = (next_index(lcg, 5UZ) < 2UZ);    // ~40% set, deterministic
        }
        return v;
}

}       // namespace

// The endgame-database lookup: one random read, and what it costs is a miss. Reported per item, because bytes per
// second is meaningless when a lookup touches one bit and pays for a whole line.
template<class T>
auto bm_random_read(benchmark::State& state)
        -> void
{
        auto const n = bits(state);
        auto const v = filled<T>(n);
        auto lcg = std::uint64_t{12345};
        for (auto _ : state) {
                bool b = v[next_index(lcg, n)];
                benchmark::DoNotOptimize(b);
        }
        state.SetItemsProcessed(state.iterations());
}

// The other half of a database pass: not a lookup but a sweep, where a container owning its blocks should have
// the advantage over one that does not -- and does not, which is the finding. [design.md#the-sequence-ladder]
template<class T>
auto bm_sequential_count(benchmark::State& state)
        -> void
{
        auto const n = bits(state);
        auto const v = filled<T>(n);
        for (auto _ : state) {
                auto c = std::count(v.begin(), v.end(), true);
                benchmark::DoNotOptimize(c);
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(n / 8UZ));
}

// Building a slice: the allocation and the fill, which is what a database build pays once per slice.
template<class T>
auto bm_construct(benchmark::State& state)
        -> void
{
        auto const n = bits(state);
        for (auto _ : state) {
                auto v = T(n);
                benchmark::DoNotOptimize(v);
                benchmark::ClobberMemory();
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(n / 8UZ));
}

#define BM_LADDER(fn)                                                   \
        BENCHMARK_TEMPLATE1(fn, std::vector<bool>)                      \
                ->RangeMultiplier(4)->Range(1L << 10, 1L << 22);        \
        BENCHMARK_TEMPLATE1(fn, xstd::bit_vector)                       \
                ->RangeMultiplier(4)->Range(1L << 10, 1L << 22)

BM_LADDER(bm_random_read);
BM_LADDER(bm_sequential_count);

// Construction allocates and zeroes the whole slice, so it stops four rungs short of the others rather than spend
// the run on the allocator.
#define BM_BUILD_LADDER(fn)                                             \
        BENCHMARK_TEMPLATE1(fn, std::vector<bool>)                      \
                ->RangeMultiplier(4)->Range(1L << 10, 1L << 18);        \
        BENCHMARK_TEMPLATE1(fn, xstd::bit_vector)                       \
                ->RangeMultiplier(4)->Range(1L << 10, 1L << 18)

BM_BUILD_LADDER(bm_construct);

BENCHMARK_MAIN();
