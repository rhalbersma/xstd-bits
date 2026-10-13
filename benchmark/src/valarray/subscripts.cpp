//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The valarray row's subscripts: xstd::bit_valarray against std::valarray<bool>, a gather and a scatter each.

#include <xstd/bits/bit_valarray.hpp> // bit_valarray
#include <benchmark/benchmark.h>      // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE, BENCHMARK_MAIN, State
#include <cstddef>                    // size_t
#include <cstdint>                    // int64_t, uint64_t
#include <ranges>                     // iota
#include <utility>                    // as_const
#include <valarray>                   // gslice, slice, valarray

namespace {

inline constexpr auto n = 1UZ << 16U;

// One LCG step per element, so a density is met on average and no pattern lines up with a block.
constexpr auto next_index(std::uint64_t& lcg, std::size_t bound)
        -> std::size_t
{
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<std::size_t>(lcg >> 33U) % bound;
}

// Each element set with probability percent / 100, deterministically.
template<class T>
auto filled(std::size_t size, std::size_t percent, std::uint64_t seed)
        -> T
{
        auto v   = T(size);
        auto lcg = seed;
        for (auto const i : std::views::iota(0UZ, size)) {
                v[i] = next_index(lcg, 100UZ) < percent;
        }
        return v;
}

// The positions a mask selects, as the index array the indirect subscript takes.
template<class T>
auto indices_of(T const& mask)
        -> std::valarray<std::size_t>
{
        auto count = 0UZ;
        for (auto const i : std::views::iota(0UZ, mask.size())) {
                count += mask[i] ? 1UZ : 0UZ;
        }
        auto result = std::valarray<std::size_t>(count);
        auto j      = 0UZ;
        for (auto const i : std::views::iota(0UZ, mask.size())) {
                if (mask[i]) {
                        result[j++] = i;
                }
        }
        return result;
}

auto set_counters(benchmark::State& state, std::size_t selected)
        -> void
{
        state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(selected));
}

} // namespace

// v[slice(0, n / stride, stride)] read into an array of its own.
template<class T>
auto bm_slice_gather(benchmark::State& state)
        -> void
{
        auto const stride = static_cast<std::size_t>(state.range(0));
        auto const v      = filled<T>(n, 50UZ, 1U);
        auto const s      = std::slice(0UZ, n / stride, stride);
        for (auto _ : state) {
                T w = v[s];
                benchmark::DoNotOptimize(w);
        }
        set_counters(state, s.size());
}

// v[slice(0, n / stride, stride)] = w, through the proxy.
template<class T>
auto bm_slice_scatter(benchmark::State& state)
        -> void
{
        auto const stride = static_cast<std::size_t>(state.range(0));
        auto v            = filled<T>(n, 50UZ, 1U);
        auto const s      = std::slice(0UZ, n / stride, stride);
        auto const w      = filled<T>(s.size(), 50UZ, 2U);
        for (auto _ : state) {
                v[s] = w;
                benchmark::ClobberMemory();
        }
        set_counters(state, s.size());
}

// A board of 8 by 8 squares in each 64: its 8 ranks, files 0 to 3 of each.
auto board()
        -> std::gslice
{
        return {0UZ, std::valarray<std::size_t>{n / 64UZ, 8UZ, 4UZ}, std::valarray<std::size_t>{64UZ, 8UZ, 1UZ}};
}

template<class T>
auto bm_gslice_gather(benchmark::State& state)
        -> void
{
        auto const v = filled<T>(n, 50UZ, 1U);
        auto const g = board();
        for (auto _ : state) {
                T w = v[g];
                benchmark::DoNotOptimize(w);
        }
        set_counters(state, n / 2UZ);
}

template<class T>
auto bm_gslice_scatter(benchmark::State& state)
        -> void
{
        auto v       = filled<T>(n, 50UZ, 1U);
        auto const w = filled<T>(n / 2UZ, 50UZ, 2U);
        auto const g = board();
        for (auto _ : state) {
                v[g] = w;
                benchmark::ClobberMemory();
        }
        set_counters(state, n / 2UZ);
}

// The same with the gslice built in the loop: std::gslice computes its positions in its constructor, and ours per subscript.
template<class T>
auto bm_gslice_gather_with_setup(benchmark::State& state)
        -> void
{
        auto const v = filled<T>(n, 50UZ, 1U);
        for (auto _ : state) {
                T w = v[board()];
                benchmark::DoNotOptimize(w);
        }
        set_counters(state, n / 2UZ);
}

template<class T>
auto bm_gslice_scatter_with_setup(benchmark::State& state)
        -> void
{
        auto v       = filled<T>(n, 50UZ, 1U);
        auto const w = filled<T>(n / 2UZ, 50UZ, 2U);
        for (auto _ : state) {
                v[board()] = w;
                benchmark::ClobberMemory();
        }
        set_counters(state, n / 2UZ);
}

// v[mask] at a density of selected positions given in percent.
template<class T>
auto bm_mask_gather(benchmark::State& state)
        -> void
{
        auto const v    = filled<T>(n, 50UZ, 1U);
        auto const mask = filled<T>(n, static_cast<std::size_t>(state.range(0)), 3U);
        auto const size = indices_of(mask).size();
        for (auto _ : state) {
                T w = v[mask];
                benchmark::DoNotOptimize(w);
        }
        set_counters(state, size);
}

template<class T>
auto bm_mask_scatter(benchmark::State& state)
        -> void
{
        auto v          = filled<T>(n, 50UZ, 1U);
        auto const mask = filled<T>(n, static_cast<std::size_t>(state.range(0)), 3U);
        auto const size = indices_of(mask).size();
        auto const w    = filled<T>(size, 50UZ, 2U);
        for (auto _ : state) {
                v[mask] = w;
                benchmark::ClobberMemory();
        }
        set_counters(state, size);
}

// The same positions as the mask, named one by one: the index path against the mask path, at the same densities.
template<class T>
auto bm_indirect_gather(benchmark::State& state)
        -> void
{
        auto const v       = filled<T>(n, 50UZ, 1U);
        auto const indices = indices_of(filled<T>(n, static_cast<std::size_t>(state.range(0)), 3U));
        for (auto _ : state) {
                T w = v[indices];
                benchmark::DoNotOptimize(w);
        }
        set_counters(state, indices.size());
}

template<class T>
auto bm_indirect_scatter(benchmark::State& state)
        -> void
{
        auto v             = filled<T>(n, 50UZ, 1U);
        auto const indices = indices_of(filled<T>(n, static_cast<std::size_t>(state.range(0)), 3U));
        auto const w       = filled<T>(indices.size(), 50UZ, 2U);
        for (auto _ : state) {
                v[indices] = w;
                benchmark::ClobberMemory();
        }
        set_counters(state, indices.size());
}

// Strides below a block take the mask path, one of a block or more the index path.
BENCHMARK_TEMPLATE(bm_slice_gather, std::valarray<bool>)->Arg(1)->Arg(2)->Arg(3)->Arg(8)->Arg(64)->Arg(100);
BENCHMARK_TEMPLATE(bm_slice_gather, xstd::bit_valarray)->Arg(1)->Arg(2)->Arg(3)->Arg(8)->Arg(64)->Arg(100);
BENCHMARK_TEMPLATE(bm_slice_scatter, std::valarray<bool>)->Arg(1)->Arg(2)->Arg(3)->Arg(8)->Arg(64)->Arg(100);
BENCHMARK_TEMPLATE(bm_slice_scatter, xstd::bit_valarray)->Arg(1)->Arg(2)->Arg(3)->Arg(8)->Arg(64)->Arg(100);

BENCHMARK_TEMPLATE(bm_gslice_gather, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_gather, xstd::bit_valarray);
BENCHMARK_TEMPLATE(bm_gslice_scatter, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_scatter, xstd::bit_valarray);
BENCHMARK_TEMPLATE(bm_gslice_gather_with_setup, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_gather_with_setup, xstd::bit_valarray);
BENCHMARK_TEMPLATE(bm_gslice_scatter_with_setup, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_scatter_with_setup, xstd::bit_valarray);

BENCHMARK_TEMPLATE(bm_mask_gather, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_mask_gather, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_mask_scatter, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_mask_scatter, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);

BENCHMARK_TEMPLATE(bm_indirect_gather, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_indirect_gather, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_indirect_scatter, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_indirect_scatter, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);

BENCHMARK_MAIN();
