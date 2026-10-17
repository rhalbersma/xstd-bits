//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The valarray row's subscripts: xstd::bit_valarray against std::valarray<bool>, a gather and a scatter each.

#include <xstd/bits/bit_valarray/bit_gslice.hpp>   // bit_gslice
#include <xstd/bits/bit_valarray/bit_valarray.hpp> // bit_valarray
#include <xstd/bits/detail/ownership.hpp>          // storage_access
#include <xstd/ints/bit.hpp>                       // bit_compress, bit_expand, bit_repeat
#include <benchmark/benchmark.h>                   // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE, BENCHMARK_MAIN, State
#include <algorithm>                               // copy
#include <array>                                   // array
#include <bit>                                     // popcount
#include <cstddef>                                 // size_t
#include <cstdint>                                 // int64_t, uint64_t
#include <numeric>                                 // gcd
#include <ranges>                                  // iota
#include <span>                                    // span
#include <valarray>                                // gslice, slice, valarray

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

// The handwritten column: P3104's word functions over 64-bit words, as a bitboard engine writes them. The words are
// the blocks of bit_valarrays filled as the library's are, and each gather's result is a bit_valarray too, so both
// columns run on the same allocator, alignment and fill, and differ only in the code between them.
using word = std::size_t;

auto blocks(xstd::bit_valarray& v)
        -> std::span<word>
{
        return xstd::bits::detail::storage_access::bits(v).blocks();
}

auto blocks(xstd::bit_valarray const& v)
        -> std::span<word const>
{
        return xstd::bits::detail::storage_access::bits(v).blocks();
}

// The packed bits of each word gathered in a register, each output word stored once, full.
struct packer
{
        std::span<word> out;
        std::size_t index = 0UZ;
        std::size_t fill  = 0UZ;
        word acc          = 0U;

        auto put(word bits, std::size_t count)
                -> void
        {
                acc |= bits << fill;
                auto const before = fill;
                fill += count;
                if (fill >= 64UZ) {
                        out[index++] = acc;
                        fill -= 64UZ;
                        acc = (bits >> 1U) >> (63UZ - before);
                }
        }

        auto flush()
                -> void
        {
                if (fill != 0UZ) {
                        out[index] = acc;
                }
        }
};

// The source bits a scatter deposits, a register holding those not yet taken.
struct unpacker
{
        std::span<word const> in;
        std::size_t next  = 0UZ;
        std::size_t avail = 0UZ;
        word buffer       = 0U;

        auto take(std::size_t count)
                -> word
        {
                if (count <= avail) {
                        auto const bits = buffer;
                        buffer          = (buffer >> (count / 2UZ)) >> (count - (count / 2UZ));
                        avail -= count;
                        return bits;
                }
                auto const word_in = in[next++];
                auto const bits    = buffer | (word_in << avail);
                auto const used    = count - avail;
                buffer             = (word_in >> (used / 2UZ)) >> (used - (used / 2UZ));
                avail              = 64UZ - used;
                return bits;
        }
};

// A slice's masks as a handwritten engine sets them up: the first and last word cut, and the middle words' masks over
// whole periods of stride / gcd(stride, 64) words, with their popcounts, so the loop over the words asks for neither.
struct slice_setup
{
        std::size_t first;
        std::size_t last;
        std::size_t span;
        word first_mask;
        word last_mask;
        std::array<word, 64> masks{};
        std::array<std::size_t, 64> counts{};

        slice_setup(std::size_t start, std::size_t size, std::size_t stride)
                : first(start / 64UZ)
                , last(((start + ((size - 1UZ) * stride)) / 64UZ) + 1UZ)
                , span((64UZ / (stride / std::gcd(stride, 64UZ))) * (stride / std::gcd(stride, 64UZ)))
                , first_mask()
                , last_mask()
        {
                auto const ones  = xstd::bit_repeat(word{1}, static_cast<int>(stride));
                auto const carry = 64UZ % stride;
                auto const final = start + ((size - 1UZ) * stride);
                auto const tail  = final % 64UZ == 63UZ ? ~word{0} : (word{1} << ((final % 64UZ) + 1UZ)) - 1U;
                auto offset      = (start % 64UZ) % stride;
                first_mask       = (ones << offset) & ~((word{1} << (start % 64UZ)) - 1U);
                if (first + 1UZ == last) {
                        first_mask &= tail;
                }
                offset = offset >= carry ? offset - carry : offset + stride - carry;
                for (auto const p : std::views::iota(0UZ, span)) {
                        masks[p]  = ones << offset;
                        counts[p] = static_cast<std::size_t>(std::popcount(masks[p]));
                        offset    = offset >= carry ? offset - carry : offset + stride - carry;
                }
                last_mask = masks[(last - first - 2UZ) % span] & tail;
        }

        // The words in order, with their masks and counts: the same loop the library's slice runs.
        template<class F>
        auto run(F f) const
                -> void
        {
                f(first, first_mask, static_cast<std::size_t>(std::popcount(first_mask)));
                if (first + 1UZ == last) {
                        return;
                }
                auto k = first + 1UZ;
                if (span == 64UZ) {
                        for (; k + 1UZ < last; ++k) {
                                f(k, masks[0], counts[0]);
                        }
                } else {
                        for (; k + span < last; k += span) {
                                for (auto const p : std::views::iota(0UZ, span)) {
                                        f(k + p, masks[p], counts[p]);
                                }
                        }
                        for (auto p = 0UZ; k + 1UZ < last; ++k, ++p) {
                                f(k, masks[p], counts[p]);
                        }
                }
                f(last - 1UZ, last_mask, static_cast<std::size_t>(std::popcount(last_mask)));
        }
};

// A gather returns an array of its own, as v[s] does: a clear output of the size selected.
auto hand_gather(std::span<word const> src, std::span<word const> mask, std::span<word> out)
        -> void
{
        auto p = packer{out};
        for (auto const k : std::views::iota(0UZ, src.size())) {
                p.put(xstd::bit_compress(src[k], mask[k]), static_cast<std::size_t>(std::popcount(mask[k])));
        }
        p.flush();
}

auto hand_scatter(std::span<word> dst, std::span<word const> mask, std::span<word const> in)
        -> void
{
        auto u = unpacker{in};
        for (auto const k : std::views::iota(0UZ, dst.size())) {
                auto const bits = u.take(static_cast<std::size_t>(std::popcount(mask[k])));
                dst[k]          = (dst[k] & ~mask[k]) | xstd::bit_expand(bits, mask[k]);
        }
}

template<class Position>
auto hand_index_gather(std::span<word const> src, std::size_t count, Position position, std::span<word> out)
        -> void
{
        auto acc = word{0};
        for (auto const j : std::views::iota(0UZ, count)) {
                auto const i = position(j);
                acc |= ((src[i / 64UZ] >> (i % 64UZ)) & 1U) << (j % 64UZ);
                if (j % 64UZ == 63UZ) {
                        out[j / 64UZ] = acc;
                        acc           = 0U;
                }
        }
        if (count % 64UZ != 0UZ) {
                out[count / 64UZ] = acc;
        }
}

template<class Position>
auto hand_index_scatter(std::span<word> dst, std::size_t count, Position position, std::span<word const> in)
        -> void
{
        for (auto const j : std::views::iota(0UZ, count)) {
                auto const i   = position(j);
                auto const bit = (in[j / 64UZ] >> (j % 64UZ)) & 1U;
                auto& w        = dst[i / 64UZ];
                w              = (w & ~(word{1} << (i % 64UZ))) | (bit << (i % 64UZ));
        }
}

// A selection as a mask over n bits, made once outside the loop as a handwritten engine would.
template<class Selects>
auto mask_of(Selects selects)
        -> xstd::bit_valarray
{
        auto m = xstd::bit_valarray(n);
        for (auto const i : std::views::iota(0UZ, n)) {
                m[i] = selects(i);
        }
        return m;
}

// The handwritten slice: a stride-one gather is a copy, any other the same set-up and loop as the library's.
auto bm_slice_gather_hand(benchmark::State& state)
        -> void
{
        auto const stride = static_cast<std::size_t>(state.range(0));
        auto const v      = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        for (auto _ : state) {
                auto out = xstd::bit_valarray(n / stride);
                if (stride == 1UZ) {
                        std::ranges::copy(blocks(v), blocks(out).begin());
                } else if (stride >= 64UZ) {
                        // A word or more apart, at most one bit a word: one position at a time, as the library does.
                        hand_index_gather(blocks(v), n / stride, [=](std::size_t j) -> std::size_t { return j * stride; }, blocks(out));
                } else {
                        auto const src = blocks(v);
                        auto p         = packer{blocks(out)};
                        slice_setup(0UZ, n / stride, stride).run([&](std::size_t k, word mask, std::size_t count) -> void { p.put(xstd::bit_compress(src[k], mask), count); });
                        p.flush();
                }
                benchmark::DoNotOptimize(out);
                benchmark::ClobberMemory();
        }
        set_counters(state, n / stride);
}

auto bm_slice_scatter_hand(benchmark::State& state)
        -> void
{
        auto const stride = static_cast<std::size_t>(state.range(0));
        auto v            = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const w      = filled<xstd::bit_valarray>(n / stride, 50UZ, 2U);
        for (auto _ : state) {
                if (stride == 1UZ) {
                        std::ranges::copy(blocks(w), blocks(v).begin());
                } else if (stride >= 64UZ) {
                        hand_index_scatter(blocks(v), n / stride, [=](std::size_t j) -> std::size_t { return j * stride; }, blocks(w));
                } else {
                        auto const dst = blocks(v);
                        auto u         = unpacker{blocks(w)};
                        slice_setup(0UZ, n / stride, stride).run([&](std::size_t k, word mask, std::size_t count) -> void { dst[k] = (dst[k] & ~mask) | xstd::bit_expand(u.take(count), mask); });
                }
                benchmark::ClobberMemory();
        }
        set_counters(state, n / stride);
}

auto bm_mask_gather_hand(benchmark::State& state)
        -> void
{
        auto const v    = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const mask = filled<xstd::bit_valarray>(n, static_cast<std::size_t>(state.range(0)), 3U);
        auto size       = 0UZ;
        for (auto _ : state) {
                // The size first, as a gather returning its own array must know it.
                size = 0UZ;
                for (auto const m : blocks(mask)) {
                        size += static_cast<std::size_t>(xstd::popcount(m));
                }
                auto out = xstd::bit_valarray(size);
                hand_gather(blocks(v), blocks(mask), blocks(out));
                benchmark::DoNotOptimize(out);
                benchmark::ClobberMemory();
        }
        set_counters(state, size);
}

auto bm_mask_scatter_hand(benchmark::State& state)
        -> void
{
        auto v          = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const mask = filled<xstd::bit_valarray>(n, static_cast<std::size_t>(state.range(0)), 3U);
        auto size       = 0UZ;
        for (auto const m : blocks(mask)) {
                size += static_cast<std::size_t>(xstd::popcount(m));
        }
        auto const w = filled<xstd::bit_valarray>(size, 50UZ, 2U);
        for (auto _ : state) {
                hand_scatter(blocks(v), blocks(mask), blocks(w));
                benchmark::ClobberMemory();
        }
        set_counters(state, size);
}

// The board as a handwritten engine has it: one constant mask in every word.
auto bm_gslice_gather_hand(benchmark::State& state)
        -> void
{
        auto const v    = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const mask = mask_of([](std::size_t i) -> bool { return i % 8UZ < 4UZ; });
        for (auto _ : state) {
                auto out = xstd::bit_valarray(n / 2UZ);
                hand_gather(blocks(v), blocks(mask), blocks(out));
                benchmark::DoNotOptimize(out);
                benchmark::ClobberMemory();
        }
        set_counters(state, n / 2UZ);
}

auto bm_gslice_scatter_hand(benchmark::State& state)
        -> void
{
        auto v          = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const mask = mask_of([](std::size_t i) -> bool { return i % 8UZ < 4UZ; });
        auto const w    = filled<xstd::bit_valarray>(n / 2UZ, 50UZ, 2U);
        for (auto _ : state) {
                hand_scatter(blocks(v), blocks(mask), blocks(w));
                benchmark::ClobberMemory();
        }
        set_counters(state, n / 2UZ);
}

auto bm_indirect_gather_hand(benchmark::State& state)
        -> void
{
        auto const v       = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const indices = indices_of(filled<std::valarray<bool>>(n, static_cast<std::size_t>(state.range(0)), 3U));
        for (auto _ : state) {
                auto out = xstd::bit_valarray(indices.size());
                hand_index_gather(blocks(v), indices.size(), [&](std::size_t j) -> std::size_t { return indices[j]; }, blocks(out));
                benchmark::DoNotOptimize(out);
                benchmark::ClobberMemory();
        }
        set_counters(state, indices.size());
}

auto bm_indirect_scatter_hand(benchmark::State& state)
        -> void
{
        auto v             = filled<xstd::bit_valarray>(n, 50UZ, 1U);
        auto const indices = indices_of(filled<std::valarray<bool>>(n, static_cast<std::size_t>(state.range(0)), 3U));
        auto const w       = filled<xstd::bit_valarray>(indices.size(), 50UZ, 2U);
        for (auto _ : state) {
                hand_index_scatter(blocks(v), indices.size(), [&](std::size_t j) -> std::size_t { return indices[j]; }, blocks(w));
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

// The gslice built in the loop: std::gslice finds its positions when built, a bit_valarray per subscript.
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

} // namespace

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
