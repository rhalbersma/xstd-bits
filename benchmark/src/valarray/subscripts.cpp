//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The valarray row's subscripts: xstd::bit_valarray against std::valarray<bool>, a gather and a scatter each.

#include <xstd/bits/bit_gslice.hpp>   // bit_gslice
#include <xstd/bits/bit_valarray.hpp> // bit_valarray
#include <xstd/ints/bit.hpp>          // bit_compress, bit_expand, popcount
#include <benchmark/benchmark.h>      // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE, BENCHMARK_MAIN, State
#include <cstddef>                    // size_t
#include <algorithm>                  // copy
#include <cstdint>                    // int64_t, uint64_t
#include <ranges>                     // iota
#include <utility>                    // as_const
#include <valarray>                   // gslice, slice, valarray
#include <vector>                     // vector

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

// The handwritten column: raw 64-bit words, P3104's word functions, no container, as a bitboard engine writes them.
using words = std::vector<std::uint64_t>;

auto words_of(std::size_t size, std::size_t percent, std::uint64_t seed)
        -> words
{
        auto const v = filled<std::valarray<bool>>(size, percent, seed);
        auto w       = words((size + 63UZ) / 64UZ);
        for (auto const i : std::views::iota(0UZ, size)) {
                w[i / 64UZ] |= static_cast<std::uint64_t>(v[i]) << (i % 64UZ);
        }
        return w;
}

// Bits packed in at a running offset, each word's share split over at most two output words.
inline auto append(words& out, std::size_t& pos, std::uint64_t bits, std::size_t count)
        -> void
{
        auto const index  = pos / 64UZ;
        auto const offset = pos % 64UZ;
        out[index] |= bits << offset;
        if (offset != 0UZ and offset + count > 64UZ) {
                out[index + 1UZ] |= bits >> (64UZ - offset);
        }
        pos += count;
}

// The next count bits from a running offset.
inline auto take(words const& in, std::size_t& pos, std::size_t count)
        -> std::uint64_t
{
        auto const index  = pos / 64UZ;
        auto const offset = pos % 64UZ;
        auto bits         = in[index] >> offset;
        if (offset != 0UZ and index + 1UZ < in.size()) {
                bits |= in[index + 1UZ] << (64UZ - offset);
        }
        pos += count;
        return bits;
}

// A gather returns an array of its own, as v[s] does: a clear output of the size selected.
auto hand_gather(words const& src, words const& mask, words& out)
        -> void
{
        auto pos = 0UZ;
        for (auto const k : std::views::iota(0UZ, src.size())) {
                append(out, pos, xstd::bit_compress(src[k], mask[k]), static_cast<std::size_t>(xstd::popcount(mask[k])));
        }
}

auto hand_scatter(words& dst, words const& mask, words const& in)
        -> void
{
        auto pos = 0UZ;
        for (auto const k : std::views::iota(0UZ, dst.size())) {
                auto const bits = take(in, pos, static_cast<std::size_t>(xstd::popcount(mask[k])));
                dst[k]          = (dst[k] & ~mask[k]) | xstd::bit_expand(bits, mask[k]);
        }
}

auto hand_index_gather(words const& src, std::valarray<std::size_t> const& indices, words& out)
        -> void
{
        for (auto const j : std::views::iota(0UZ, indices.size())) {
                auto const i   = indices[j];
                auto const bit = (src[i / 64UZ] >> (i % 64UZ)) & 1U;
                auto& w        = out[j / 64UZ];
                w              = (w & ~(std::uint64_t{1} << (j % 64UZ))) | (bit << (j % 64UZ));
        }
}

auto hand_index_scatter(words& dst, std::valarray<std::size_t> const& indices, words const& in)
        -> void
{
        for (auto const j : std::views::iota(0UZ, indices.size())) {
                auto const i   = indices[j];
                auto const bit = (in[j / 64UZ] >> (j % 64UZ)) & 1U;
                auto& w        = dst[i / 64UZ];
                w              = (w & ~(std::uint64_t{1} << (i % 64UZ))) | (bit << (i % 64UZ));
        }
}

// A slice's positions as a mask word per word, made once outside the loop as a handwritten engine would.
auto slice_mask(std::size_t stride)
        -> words
{
        auto m = words(n / 64UZ);
        for (auto i = 0UZ; i < n; i += stride) {
                m[i / 64UZ] |= std::uint64_t{1} << (i % 64UZ);
        }
        return m;
}

} // namespace

// The handwritten slice: a stride-one gather is a copy, any other a compress against a mask built beforehand.
auto bm_slice_gather_hand(benchmark::State& state)
        -> void
{
        auto const stride = static_cast<std::size_t>(state.range(0));
        auto const v      = words_of(n, 50UZ, 1U);
        auto const mask   = slice_mask(stride);
        for (auto _ : state) {
                auto out = words(((n / stride) + 63UZ) / 64UZ);
                if (stride == 1UZ) {
                        std::ranges::copy(v, out.begin());
                } else {
                        hand_gather(v, mask, out);
                }
                benchmark::DoNotOptimize(out.data());
                benchmark::ClobberMemory();
        }
        set_counters(state, n / stride);
}

auto bm_slice_scatter_hand(benchmark::State& state)
        -> void
{
        auto const stride = static_cast<std::size_t>(state.range(0));
        auto v            = words_of(n, 50UZ, 1U);
        auto const mask   = slice_mask(stride);
        auto const w      = words_of(n / stride, 50UZ, 2U);
        for (auto _ : state) {
                if (stride == 1UZ) {
                        std::ranges::copy(w, v.begin());
                } else {
                        hand_scatter(v, mask, w);
                }
                benchmark::ClobberMemory();
        }
        set_counters(state, n / stride);
}

auto bm_mask_gather_hand(benchmark::State& state)
        -> void
{
        auto const v    = words_of(n, 50UZ, 1U);
        auto const mask = words_of(n, static_cast<std::size_t>(state.range(0)), 3U);
        auto size       = 0UZ;
        for (auto _ : state) {
                // The size first, as a gather returning its own array must know it.
                size = 0UZ;
                for (auto const m : mask) {
                        size += static_cast<std::size_t>(xstd::popcount(m));
                }
                auto out = words((size + 63UZ) / 64UZ);
                hand_gather(v, mask, out);
                benchmark::DoNotOptimize(out.data());
                benchmark::ClobberMemory();
        }
        set_counters(state, size);
}

auto bm_mask_scatter_hand(benchmark::State& state)
        -> void
{
        auto v          = words_of(n, 50UZ, 1U);
        auto const mask = words_of(n, static_cast<std::size_t>(state.range(0)), 3U);
        auto size       = 0UZ;
        for (auto const m : mask) {
                size += static_cast<std::size_t>(xstd::popcount(m));
        }
        auto const w = words_of(size, 50UZ, 2U);
        for (auto _ : state) {
                hand_scatter(v, mask, w);
                benchmark::ClobberMemory();
        }
        set_counters(state, size);
}

// The board as a handwritten engine has it: one constant mask in every word.
auto bm_gslice_gather_hand(benchmark::State& state)
        -> void
{
        auto const v    = words_of(n, 50UZ, 1U);
        auto const mask = words(n / 64UZ, 0x0F0F'0F0F'0F0F'0F0FULL);
        for (auto _ : state) {
                auto out = words(n / 128UZ);
                hand_gather(v, mask, out);
                benchmark::DoNotOptimize(out.data());
                benchmark::ClobberMemory();
        }
        set_counters(state, n / 2UZ);
}

auto bm_gslice_scatter_hand(benchmark::State& state)
        -> void
{
        auto v          = words_of(n, 50UZ, 1U);
        auto const mask = words(n / 64UZ, 0x0F0F'0F0F'0F0F'0F0FULL);
        auto const w    = words_of(n / 2UZ, 50UZ, 2U);
        for (auto _ : state) {
                hand_scatter(v, mask, w);
                benchmark::ClobberMemory();
        }
        set_counters(state, n / 2UZ);
}

auto bm_indirect_gather_hand(benchmark::State& state)
        -> void
{
        auto const v       = words_of(n, 50UZ, 1U);
        auto const indices = indices_of(filled<std::valarray<bool>>(n, static_cast<std::size_t>(state.range(0)), 3U));
        for (auto _ : state) {
                auto out = words((indices.size() + 63UZ) / 64UZ);
                hand_index_gather(v, indices, out);
                benchmark::DoNotOptimize(out.data());
                benchmark::ClobberMemory();
        }
        set_counters(state, indices.size());
}

auto bm_indirect_scatter_hand(benchmark::State& state)
        -> void
{
        auto v             = words_of(n, 50UZ, 1U);
        auto const indices = indices_of(filled<std::valarray<bool>>(n, static_cast<std::size_t>(state.range(0)), 3U));
        auto const w       = words_of(indices.size(), 50UZ, 2U);
        for (auto _ : state) {
                hand_index_scatter(v, indices, w);
                benchmark::ClobberMemory();
        }
        set_counters(state, indices.size());
}

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

// The board through a bit_gslice made once, as std::gslice makes its positions once.
auto bm_bit_gslice_gather(benchmark::State& state)
        -> void
{
        auto const v = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const g = xstd::bit_gslice(board());
        for (auto _ : state) {
                xstd::bit_valarray w = v[g];
                benchmark::DoNotOptimize(w);
        }
        set_counters(state, n / 2UZ);
}

auto bm_bit_gslice_scatter(benchmark::State& state)
        -> void
{
        auto v       = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const w = filled<xstd::bit_valarray>(n / 2UZ, 50UZ, 2U);
        auto const g = xstd::bit_gslice(board());
        for (auto _ : state) {
                v[g] = w;
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
BENCHMARK(bm_slice_gather_hand)->Arg(1)->Arg(2)->Arg(3)->Arg(8)->Arg(64)->Arg(100);
BENCHMARK(bm_slice_scatter_hand)->Arg(1)->Arg(2)->Arg(3)->Arg(8)->Arg(64)->Arg(100);

BENCHMARK_TEMPLATE(bm_gslice_gather, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_gather, xstd::bit_valarray);
BENCHMARK_TEMPLATE(bm_gslice_scatter, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_scatter, xstd::bit_valarray);
BENCHMARK_TEMPLATE(bm_gslice_gather_with_setup, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_gather_with_setup, xstd::bit_valarray);
BENCHMARK_TEMPLATE(bm_gslice_scatter_with_setup, std::valarray<bool>);
BENCHMARK_TEMPLATE(bm_gslice_scatter_with_setup, xstd::bit_valarray);
BENCHMARK(bm_bit_gslice_gather);
BENCHMARK(bm_bit_gslice_scatter);
BENCHMARK(bm_gslice_gather_hand);
BENCHMARK(bm_gslice_scatter_hand);

BENCHMARK_TEMPLATE(bm_mask_gather, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_mask_gather, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_mask_scatter, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_mask_scatter, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK(bm_mask_gather_hand)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK(bm_mask_scatter_hand)->Arg(5)->Arg(50)->Arg(95);

BENCHMARK_TEMPLATE(bm_indirect_gather, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_indirect_gather, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_indirect_scatter, std::valarray<bool>)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK_TEMPLATE(bm_indirect_scatter, xstd::bit_valarray)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK(bm_indirect_gather_hand)->Arg(5)->Arg(50)->Arg(95);
BENCHMARK(bm_indirect_scatter_hand)->Arg(5)->Arg(50)->Arg(95);

BENCHMARK_MAIN();
