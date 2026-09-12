//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The static-width ladder: what a block of bits costs at 1, 2, 4, ... 1024 words, ours against std::bitset. [design.md#two-block-case]

#include <xstd/bits/bit_set_view.hpp>   // bit_set_view
#include <xstd/bits/bitset.hpp>         // aligned::bitset, bitset
#include <xstd/bits/ext/std/bitset.hpp> // bit_traits over std::bitset
#include <benchmark/benchmark.h>        // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <bitset>                       // bitset
#include <cstddef>                      // size_t
#include <cstdint>                      // uint64_t

namespace {

inline constexpr auto bits_per_word = 64UZ;

// A board-game density rather than a uniform one: an occupancy bitboard is neither empty nor full, and find_next on a 1%-set bitset is a different benchmark from one on a 90%-set bitset.
template<class T>
auto filled(std::size_t n, std::uint64_t seed)
        -> T
{
        auto bits = T();
        auto state = seed | 1ULL;
        for (auto i = 0UZ; i < n; ++i) {
                state = state * 6364136223846793005ULL + 1442695040888963407ULL;
                if ((state >> 33) % 5UZ < 2UZ) {        // ~40% set
                        bits.set(i);
                }
        }
        return bits;
}

}       // namespace

// The operand escapes and memory is clobbered on every iteration: an unrolled two-word AND over a stack operand is exactly the shape a compiler deletes, and measuring nothing at the rung the bench exists for would be the one failure that still looks like a result.
#define BM_BINARY(name, op)                                                     \
        template<class T, std::size_t N>                                        \
        auto name(benchmark::State& state)                                      \
                -> void                                                         \
        {                                                                       \
                auto a = filled<T>(N, 1);                                       \
                auto b = filled<T>(N, 2);                                       \
                for (auto _ : state) {                                          \
                        benchmark::DoNotOptimize(a);                            \
                        benchmark::DoNotOptimize(b);                            \
                        a op b;                                                 \
                        benchmark::ClobberMemory();                             \
                }                                                               \
                state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(N / 8UZ)); \
        }

BM_BINARY(bm_and, &=)
BM_BINARY(bm_or,  |=)
BM_BINARY(bm_xor, ^=)

template<class T, std::size_t N>
auto bm_shift_left(benchmark::State& state)
        -> void
{
        auto a = filled<T>(N, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                a <<= 3UZ;
                benchmark::ClobberMemory();
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(N / 8UZ));
}

template<class T, std::size_t N>
auto bm_count(benchmark::State& state)
        -> void
{
        auto a = filled<T>(N, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto n = a.count();
                benchmark::DoNotOptimize(n);
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(N / 8UZ));
}

// all() is where the unused tail shows: an unaligned width compares the last block against a mask, an aligned one against all-ones. [design.md#padding]
template<class T, std::size_t N>
auto bm_all(benchmark::State& state)
        -> void
{
        auto a = filled<T>(N, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto b = a.all();
                benchmark::DoNotOptimize(b);
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(N / 8UZ));
}

template<class T, std::size_t N>
auto bm_flip(benchmark::State& state)
        -> void
{
        auto a = filled<T>(N, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                a.flip();
                benchmark::ClobberMemory();
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(N / 8UZ));
}

// The scan is what the two-block arm is actually about: design.md#two-block-case argues the general walk costs more than the whole scan is worth at this width, so the tail is a named block rather than a range. [design.md#the-blit]
template<class T, std::size_t N>
auto bm_scan(benchmark::State& state)
        -> void
{
        auto a = filled<T>(N, 1);
        for (auto _ : state) {
                benchmark::DoNotOptimize(a);
                auto sum = 0UZ;
                for (auto const pos : xstd::bit_set_view(a)) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(N / 8UZ));
}

// N is a template argument on both sides, so the ladder is a compile-time list rather than a Range.
#define BM_RUNG(fn, words)                                                                      \
        BENCHMARK_TEMPLATE(fn, std::bitset <words * bits_per_word>, words * bits_per_word);     \
        BENCHMARK_TEMPLATE(fn, xstd::bitset<words * bits_per_word>, words * bits_per_word)

// Three words breaks the doubling on purpose: it is the first width nobody has an unrolled arm for, and so the control that says a two-word result is the specialization rather than noise. [design.md#two-block-case]
#define BM_LADDER(fn)    \
        BM_RUNG(fn,   1); \
        BM_RUNG(fn,   2); \
        BM_RUNG(fn,   3); \
        BM_RUNG(fn,   4); \
        BM_RUNG(fn,   8); \
        BM_RUNG(fn,  16); \
        BM_RUNG(fn,  32); \
        BM_RUNG(fn,  64); \
        BM_RUNG(fn, 128); \
        BM_RUNG(fn, 256); \
        BM_RUNG(fn, 512)

BM_LADDER(bm_and);
BM_LADDER(bm_or);
BM_LADDER(bm_xor);
BM_LADDER(bm_shift_left);
BM_LADDER(bm_count);
BM_LADDER(bm_all);
BM_LADDER(bm_flip);
BM_LADDER(bm_scan);

// The alignment A/B, at the width a padded board actually lands on. [design.md#padding]
#define BM_ALIGNMENT(fn)                                                        \
        BENCHMARK_TEMPLATE(fn, std::bitset<120>,           120);                \
        BENCHMARK_TEMPLATE(fn, xstd::bitset<120>,          120);                \
        BENCHMARK_TEMPLATE(fn, xstd::aligned::bitset<120>, 120);                \
        BENCHMARK_TEMPLATE(fn, std::bitset<128>,           128);                \
        BENCHMARK_TEMPLATE(fn, xstd::bitset<128>,          128)

BM_ALIGNMENT(bm_flip);
BM_ALIGNMENT(bm_shift_left);
BM_ALIGNMENT(bm_all);
BM_ALIGNMENT(bm_scan);

BENCHMARK_MAIN();
