//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The sequence row: xstd::bit_vector against std::vector<bool>, which is the whole comparison.

#include <xstd/bits/bit_vector.hpp> // bit_vector
#include <algorithm>                // count
#include <benchmark/benchmark.h>    // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <cstddef>                  // size_t
#include <cstdint>                  // int64_t, uint64_t
#include <vector>                   // vector

namespace {

inline constexpr auto bits_per_word = 64UZ;

auto bits(benchmark::State const& state)
        -> std::size_t
{
        return static_cast<std::size_t>(state.range(0)) * bits_per_word;
}

// One LCG step per lookup: cheap against a DRAM miss, and unpredictable enough to defeat the prefetcher.
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
                v[i] = (next_index(lcg, 5UZ) < 2UZ); // ~40% set, deterministic
        }
        return v;
}

} // namespace

// The endgame-database lookup: one random read, and what it costs is a miss.
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

// The other half of a database pass: not a lookup but a sweep -- and a row whose number belongs to the standard library rather than to either container. At 65536 bits, 40% set, medians of seven runs, ns, with the ratio ours-to-theirs:
//
//      compiler and library            vector<bool>    bit_vector      ratio
//      g++-14, libstdc++                     43690         52930       1.21x
//      clang++-20, libstdc++                 83680         19232       0.23x
//      clang++-20, libc++                       73         19146     262.27x
//
// Same two containers, same bits, and the answer runs from four times faster to two hundred and sixty times slower. libc++ SPECIALIZES std::count for its vector<bool> iterator -- __count_bool in <__algorithm/count.h>, one popcount per word -- and gets 73ns for the whole sweep. libstdc++ specializes fill for that iterator and not count, so there both sides walk bit by bit through a proxy and the remaining difference is codegen: clang turns our indexed read into something four times quicker than it manages for theirs, gcc does not.
// So this row cannot be read as a comparison of containers, and it is not a gap to close. There is no portable way to make std::count count words for a container defined outside the standard library: libc++ reaches its own iterator by overloading inside its own namespace, and neither std::count nor std::ranges::count offers a customization point that a user-defined bit container could hook. What the row does say, and what is worth keeping it for, is how much a sweep costs when it is asked through a generic algorithm.
// The row that compares the containers is bm_sequential_count_member below.
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

// The same question asked of the sequence reading rather than of a generic algorithm: one popcount per word. 91ns, 73ns and 74ns in those same three configurations -- it does not care which library or which compiler, because the loop is ours either way -- against 43690, 83680 and 73 for std::count over std::vector<bool>. Level with libc++'s specialized count at 1.01x, and some five hundred times quicker than what libstdc++ can offer for the same question.
// std::vector<bool> has no member to put beside it, which is why this rung is ours alone: what it is read against is the counterpart's rung in bm_sequential_count, that being the only way std::vector<bool> can be asked at all.
template<class T>
auto bm_sequential_count_member(benchmark::State& state)
        -> void
{
        auto const n = bits(state);
        auto const v = filled<T>(n);
        for (auto _ : state) {
                auto c = v.count();
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

// Ours alone: the count below is a walk only this reading spells, read against bm_sequential_count's rung.
#define BM_LADDER_OURS(fn) \
        BENCHMARK_TEMPLATE1(fn, xstd::bit_vector) \
                ->RangeMultiplier(4) \
                ->Range(1L << 10, 1L << 22)

#define BM_LADDER(fn) \
        BENCHMARK_TEMPLATE1(fn, std::vector<bool>) \
                ->RangeMultiplier(4) \
                ->Range(1L << 10, 1L << 22); \
        BM_LADDER_OURS(fn)

BM_LADDER(bm_random_read);
BM_LADDER(bm_sequential_count);
BM_LADDER_OURS(bm_sequential_count_member);

// Construction allocates and zeroes the whole slice, so it stops four rungs short of the others.
#define BM_BUILD_LADDER(fn) \
        BENCHMARK_TEMPLATE1(fn, std::vector<bool>) \
                ->RangeMultiplier(4) \
                ->Range(1L << 10, 1L << 18); \
        BENCHMARK_TEMPLATE1(fn, xstd::bit_vector) \
                ->RangeMultiplier(4) \
                ->Range(1L << 10, 1L << 18)

BM_BUILD_LADDER(bm_construct);

BENCHMARK_MAIN();
