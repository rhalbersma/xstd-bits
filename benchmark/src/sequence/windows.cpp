//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_algorithm/bit_copy.hpp>  // bit_copy
#include <xstd/bits/bit_algorithm/bit_count.hpp> // bit_count
#include <xstd/bits/bit_algorithm/bit_fill.hpp>  // bit_fill
#include <xstd/bits/bit_algorithm/bit_find.hpp>  // bit_find
#include <xstd/bits/bit_span/bit_span.hpp>       // bit_span
#include <xstd/bits/bit_vector.hpp>              // bit_vector
#include <benchmark/benchmark.h>                 // ClobberMemory, DoNotOptimize, BENCHMARK, BENCHMARK_MAIN, State
#include <cstddef>                               // ptrdiff_t, size_t
#include <cstdint>                               // int64_t

namespace {

inline constexpr auto bits_per_block = 64UZ;

// A window of range(0) blocks' worth of positions, from offset range(1) of a vector with a block to spare either side.
auto width(benchmark::State const& state)
        -> std::size_t
{
        return static_cast<std::size_t>(state.range(0)) * bits_per_block;
}

auto offset(benchmark::State const& state, int i)
        -> std::size_t
{
        return static_cast<std::size_t>(state.range(i));
}

auto striped(std::size_t n)
        -> xstd::bit_vector
{
        auto v = xstd::bit_vector(n);
        for (auto i = 0UZ; i < n; i += 3UZ) {
                v[i] = true;
        }
        return v;
}

} // namespace

// A window copied into another vector at the offset range(2): aligned, at equal offsets, and at two different ones.
auto bm_window_copy(benchmark::State& state)
        -> void
{
        auto const n   = width(state);
        auto const src = striped(n + (2UZ * bits_per_block));
        auto dst       = xstd::bit_vector(n + (2UZ * bits_per_block));
        auto const w   = xstd::bit_span(src).subspan(offset(state, 1), n);
        auto const out = dst.begin() + static_cast<std::ptrdiff_t>(offset(state, 2));
        for (auto _ : state) {
                xstd::ranges::bit_copy(w, out);
                benchmark::ClobberMemory();
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(n / 8UZ));
}

// A window filled, alternately true and false so that no fill is a repeat of the one before.
auto bm_window_fill(benchmark::State& state)
        -> void
{
        auto const n = width(state);
        auto v       = xstd::bit_vector(n + (2UZ * bits_per_block));
        auto const w = xstd::bit_span(v).subspan(offset(state, 1), n);
        auto value   = false;
        for (auto _ : state) {
                xstd::ranges::bit_fill(w, value);
                value = not value;
                benchmark::ClobberMemory();
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(n / 8UZ));
}

// A window's ones counted.
auto bm_window_count(benchmark::State& state)
        -> void
{
        auto const n = width(state);
        auto const v = striped(n + (2UZ * bits_per_block));
        auto const w = xstd::bit_span(v).subspan(offset(state, 1), n);
        for (auto _ : state) {
                auto c = xstd::ranges::bit_count(w, true);
                benchmark::DoNotOptimize(c);
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(n / 8UZ));
}

// A window searched for its one set position, at its end.
auto bm_window_find(benchmark::State& state)
        -> void
{
        auto const n = width(state);
        auto v       = xstd::bit_vector(n + (2UZ * bits_per_block));
        auto const s = offset(state, 1);
        v[s + n - 1UZ] = true;
        auto const w   = xstd::bit_span(v).subspan(s, n);
        for (auto _ : state) {
                auto it = xstd::ranges::bit_find(w, true);
                benchmark::DoNotOptimize(it);
        }
        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(n / 8UZ));
}

// A megabit, aligned and three positions in, and for the copy, at equal and at different offsets either side.
BENCHMARK(bm_window_copy)->Args({16384, 0, 0})->Args({16384, 3, 3})->Args({16384, 3, 10});
BENCHMARK(bm_window_fill)->Args({16384, 0})->Args({16384, 3});
BENCHMARK(bm_window_count)->Args({16384, 0})->Args({16384, 3});
BENCHMARK(bm_window_find)->Args({16384, 0})->Args({16384, 3});

BENCHMARK_MAIN();
