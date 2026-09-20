//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The run-time width against boost's, on the same word ladder the static width runs in ops.cpp.

#include <xstd/bits/bit_set_view.hpp>   // bit_set_view
#include <xstd/bits/dynamic_bitset.hpp> // dynamic_bitset
#include <boost/dynamic_bitset.hpp>     // dynamic_bitset
#include <benchmark/benchmark.h>        // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <cstddef>                      // size_t
#include <cstdint>                      // int64_t, uint64_t
#include <vector>                       // vector

namespace {

inline constexpr auto bits_per_word = 64UZ;

auto words(benchmark::State const& state)
        -> std::size_t
{
        return static_cast<std::size_t>(state.range(0));
}

// A board-game density rather than a uniform one, and deterministic, so the rungs compare with each other.
template<class T>
auto filled(std::size_t n, std::uint64_t seed)
        -> T
{
        auto bits = T(n);
        auto lcg = seed | 1ULL;
        for (auto i = 0UZ; i < n; ++i) {
                lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
                if ((lcg >> 33) % 5UZ < 2UZ) { // ~40% set
                        bits.set(i);
                }
        }
        return bits;
}

// The bits as their blocks, taken out once so both halves run over the same words at the same density.
template<class T>
auto blocks_of(T const& a)
        -> std::vector<typename T::block_type>
{
        auto blocks = std::vector<typename T::block_type>(a.num_blocks());
        to_block_range(a, blocks.begin());
        return blocks;
}

auto per_byte(benchmark::State& state)
        -> void
{
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(words(state) * sizeof(std::uint64_t)));
}

} // namespace

#define BM_BINARY(name, op) \
        template<class T> \
        auto name(benchmark::State& state) \
                -> void \
        { \
                auto a = filled<T>(words(state) * bits_per_word, 1); \
                auto b = filled<T>(words(state) * bits_per_word, 2); \
                for (auto _ : state) { \
                        benchmark::DoNotOptimize(a); \
                        benchmark::DoNotOptimize(b); \
                        a op b; \
                        benchmark::ClobberMemory(); \
                } \
                per_byte(state); \
        }

BM_BINARY(bm_and, &=)
BM_BINARY(bm_or, |=)
BM_BINARY(bm_xor, ^=)

template<class T>
auto bm_shift_left(benchmark::State& state)
        -> void
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
auto bm_count(benchmark::State& state)
        -> void
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
auto bm_flip(benchmark::State& state)
        -> void
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                a.flip();
                benchmark::ClobberMemory();
        }
        per_byte(state);
}

// The scan row, like for like: both sides call find_first and find_next, which boost and this reading both answer.
template<class T>
auto bm_scan(benchmark::State& state)
        -> void
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto sum = 0UZ;
                for (auto pos = a.find_first(); pos != T::npos; pos = a.find_next(pos)) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
        per_byte(state);
}

// What the iterator costs on top of the scan: the same positions, reached as a range-for reaches them.
template<class T>
auto bm_scan_view(benchmark::State& state)
        -> void
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

// The walk an iterator cannot be: each block loaded once, then tzcnt for the position and blsr to drop it.
template<class T>
auto bm_scan_blocks(benchmark::State& state)
        -> void
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto sum = 0UZ;
                xstd::bit_set_view(a).for_each([&sum](std::size_t pos) -> void { sum += pos; });
                benchmark::DoNotOptimize(sum);
        }
        per_byte(state);
}

// The block interface, like for like: from_block_range and to_block_range are a std::copy on both sides.
template<class T>
auto bm_from_block_range(benchmark::State& state)
        -> void
{
        // Not const, though nothing writes it: DoNotOptimize's const-ref overload is deprecated upstream.
        auto blocks = blocks_of(filled<T>(words(state) * bits_per_word, 1));
        auto a = T(words(state) * bits_per_word);
        for (auto _ : state) {
                benchmark::DoNotOptimize(blocks);
                from_block_range(blocks.begin(), blocks.end(), a);
                benchmark::ClobberMemory();
        }
        per_byte(state);
}

// The way back out, into a contiguous output iterator over a buffer that already exists.
template<class T>
auto bm_to_block_range(benchmark::State& state)
        -> void
{
        auto a = filled<T>(words(state) * bits_per_word, 1);
        auto blocks = std::vector<typename T::block_type>(a.num_blocks());
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                to_block_range(a, blocks.begin());
                benchmark::ClobberMemory();
        }
        per_byte(state);
}

// A run-time width takes the ladder as a Range; the rungs are the same 1, 2, 4 ... 512 words as the static one.
#define BM_LADDER_OURS(fn) \
        BENCHMARK_TEMPLATE1(fn, xstd::dynamic_bitset) \
                ->RangeMultiplier(2) \
                ->Range(1, 512)

#define BM_LADDER(fn) \
        BENCHMARK_TEMPLATE1(fn, boost::dynamic_bitset<std::uint64_t>) \
                ->RangeMultiplier(2) \
                ->Range(1, 512); \
        BM_LADDER_OURS(fn)

BM_LADDER(bm_and);
BM_LADDER(bm_or);
BM_LADDER(bm_xor);
BM_LADDER(bm_shift_left);
BM_LADDER(bm_count);
BM_LADDER(bm_flip);
BM_LADDER(bm_scan);
BM_LADDER_OURS(bm_scan_view);
BM_LADDER_OURS(bm_scan_blocks);
BM_LADDER(bm_from_block_range);
BM_LADDER(bm_to_block_range);

BENCHMARK_MAIN();
