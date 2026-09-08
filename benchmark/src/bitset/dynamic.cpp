//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The run-time width against boost's, on the same word ladder the static width runs in ops.cpp. Boost has no
// static column at all -- a std::vector behind every operation -- so where ops.cpp asks what a compile-time
// width buys over std::bitset, this asks what our storage buys over the one everybody reaches for.
// [design.md#a-strict-extension]

#include <benchmark/benchmark.h>                  // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <xstd/bits/bit_set_view.hpp>             // bit_set_view
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_traits over boost::dynamic_bitset
#include <cstddef>                                // size_t
#include <cstdint>                                // int64_t, uint64_t

namespace {

inline constexpr auto bits_per_word = 64UZ;

auto words(benchmark::State const& state) -> std::size_t
{
        return static_cast<std::size_t>(state.range(0));
}

// A board-game density rather than a uniform one, and deterministic, so the rungs compare with each other.
template<class T>
auto filled(std::size_t n, std::uint64_t seed) -> T
{
        auto bits = T(n);
        auto lcg = seed | 1ULL;
        for (auto i = 0UZ; i < n; ++i) {
                lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
                if ((lcg >> 33) % 5UZ < 2UZ) {          // ~40% set
                        bits.set(i);
                }
        }
        return bits;
}

void per_byte(benchmark::State& state)
{
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(words(state) * sizeof(std::uint64_t)));
}

}       // namespace

#define BM_BINARY(name, op)                                                     \
        template<class T>                                                       \
        void name(benchmark::State& state)                                      \
        {                                                                       \
                auto a = filled<T>(words(state) * bits_per_word, 1);            \
                auto b = filled<T>(words(state) * bits_per_word, 2);            \
                for (auto _ : state) {                                          \
                        benchmark::DoNotOptimize(a);                            \
                        benchmark::DoNotOptimize(b);                            \
                        a op b;                                                 \
                        benchmark::ClobberMemory();                             \
                }                                                               \
                per_byte(state);                                                \
        }

BM_BINARY(bm_and, &=)
BM_BINARY(bm_or,  |=)
BM_BINARY(bm_xor, ^=)

template<class T>
void bm_shift_left(benchmark::State& state)
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                a <<= 3UZ;
                benchmark::ClobberMemory();
        }
        per_byte(state);
}

template<class T>
void bm_count(benchmark::State& state)
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto n = a.count();
                benchmark::DoNotOptimize(n);
        }
        per_byte(state);
}

template<class T>
void bm_flip(benchmark::State& state)
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                a.flip();
                benchmark::ClobberMemory();
        }
        per_byte(state);
}

// Boost answers find_first/find_next natively and ours answers its own scan; bit_set_view is what puts the two
// behind one expression. [design.md#the-blit]
template<class T>
void bm_scan(benchmark::State& state)
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto sum = 0UZ;
                for (auto const pos : xstd::bit_set_view(a)) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
        per_byte(state);
}

// A run-time width takes the ladder as a Range where the static one needs a template list; the rungs are the same
// 1, 2, 4 ... 512 words, so the two files' rows can be read against each other.
#define BM_LADDER(fn)                                                                   \
        BENCHMARK_TEMPLATE1(fn, boost::dynamic_bitset<std::uint64_t>)                   \
                ->RangeMultiplier(2)->Range(1, 512);                                    \
        BENCHMARK_TEMPLATE1(fn, xstd::dynamic_bitset)                                   \
                ->RangeMultiplier(2)->Range(1, 512)

BM_LADDER(bm_and);
BM_LADDER(bm_or);
BM_LADDER(bm_xor);
BM_LADDER(bm_shift_left);
BM_LADDER(bm_count);
BM_LADDER(bm_flip);
BM_LADDER(bm_scan);

BENCHMARK_MAIN();
