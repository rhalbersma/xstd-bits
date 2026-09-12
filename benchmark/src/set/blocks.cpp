//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The Block ladder: what the SAME 256 bits cost carried in 32, 16, 8, 4 or 2 blocks. bitset/ops.cpp is the
// transpose of this, varying the width at a fixed 64-bit Block; here the width is pinned and the Block varies,
// which is the axis a caller actually chooses when they write basic_bit_static_set<Block, N>.
//
// The question it answers is that the choice is a trade rather than a ranking. Iterating pays per ELEMENT: the
// scan's step is shr(block, offset) then countr_zero, and a wider block makes that pair more expensive without
// making it less frequent. Shifting pays per BLOCK: half as many blocks is half the loop. So the two halves of
// this file pull in opposite directions, and neither width wins both. [design.md#uint128-support]
//
// At 128 there are three carriers and they are not the same kind of thing: xstd::uint128 is an intrinsic the
// backend lowers to a register pair, while absl::uint128 and boost::int128::uint128 are classes whose operators
// are ordinary functions. Both classes are optional -- detected below as xstd-ints detects them -- so a build
// without either simply drops those rows.

// NOT alphabetical, and load-bearing: detail/bits/intrin calls xstd::countr_zero by a QUALIFIED name, whose
// candidates bind where that call is written rather than where it is instantiated. An adapter declaring the
// overload for an integer class must therefore be seen first; included after xstd/bits/*, such a Block
// satisfies the concept and then fails inside the body. [design.md#uint128-support]
#include <xstd/ints/cstdint/int128.hpp>                 // uint128
#if __has_include(<absl/numeric/int128.h>)
#include <xstd/ints/ext/absl/int128.hpp>                // absl::uint128
#define XSTD_BITS_BENCHMARK_HAS_ABSL_INT128
#endif
#if __has_include(<boost/int128.hpp>)
#include <xstd/ints/ext/boost/int128.hpp>               // boost::int128::uint128
#define XSTD_BITS_BENCHMARK_HAS_BOOST_INT128
#endif

#include <xstd/bits/bit_static_set.hpp>                 // basic_bit_static_set
#include <benchmark/benchmark.h>                        // DoNotOptimize, BENCHMARK_TEMPLATE, BENCHMARK_MAIN, State
#include <cstddef>                                      // size_t
#include <cstdint>                                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <vector>                                       // vector

namespace {

inline constexpr auto width = 256UZ;

template<class Block>
using set_of = xstd::basic_bit_static_set<Block, width>;

// A function of the POSITION alone and never of the Block: every row has to hold the same elements, or the
// ladder is one timing per Block of a different computation. Deterministic, so the rungs compare.
template<class T>
auto filled(std::size_t per_mille, std::size_t limit = width)
        -> T
{
        auto s = T();
        auto state = 1ULL;
        for (auto i = 0UZ; i < limit; ++i) {
                state = state * 6364136223846793005ULL + 1442695040888963407ULL;
                if ((state >> 33) % 1000UZ < per_mille) {
                        s.insert(i);
                }
        }
        return s;
}

// Everything inside the first 64 positions, so every Block wider than that carries an EMPTY high block. The
// three densities above never produce one: at 256 bits they all put something in every block, so the scan's
// cross-block step is only ever taken with a non-empty block to cross into. A clustered set is the other side
// of that step -- and a real shape besides, an occupancy board being dense at one end and empty at the other.
inline constexpr auto cluster = 64UZ;

// Forward: ++ is countr_zero within a block and a skip across the empty ones, so the Block is the unit the scan
// steps in. Three densities because the skip is the whole of what a wide block buys, and it buys most where
// most blocks are empty.
template<class T, std::size_t PerMille>
auto bm_forward(benchmark::State& state)
        -> void
{
        auto s = filled<T>(PerMille);
        for (auto _ : state) {
                benchmark::DoNotOptimize(s);
                auto sum = 0UZ;
                for (auto const x : s) {
                        sum += x;
                }
                benchmark::DoNotOptimize(sum);
        }
}

// Backward, the half of a bidirectional iterator that nothing else here measures: -- and countl_zero rather
// than ++ and countr_zero. It runs through std::reverse_iterator, so an element costs a decrement to look at
// plus the loop's own increment -- the adaptor's shape, not this benchmark's.
template<class T, std::size_t PerMille>
auto bm_reverse(benchmark::State& state)
        -> void
{
        auto s = filled<T>(PerMille);
        for (auto _ : state) {
                benchmark::DoNotOptimize(s);
                auto sum = 0UZ;
                for (auto it = s.rbegin(), last = s.rend(); it != last; ++it) {
                        sum += *it;
                }
                benchmark::DoNotOptimize(sum);
        }
}

// Forward and backward over that clustered set: the scan reaches the high block, finds it empty, and stops --
// which is the arm's second branch answered the other way.
template<class T>
auto bm_forward_clustered(benchmark::State& state)
        -> void
{
        auto s = filled<T>(400, cluster);
        for (auto _ : state) {
                benchmark::DoNotOptimize(s);
                auto sum = 0UZ;
                for (auto const x : s) {
                        sum += x;
                }
                benchmark::DoNotOptimize(sum);
        }
}

template<class T>
auto bm_reverse_clustered(benchmark::State& state)
        -> void
{
        auto s = filled<T>(400, cluster);
        for (auto _ : state) {
                benchmark::DoNotOptimize(s);
                auto sum = 0UZ;
                for (auto it = s.rbegin(), last = s.rend(); it != last; ++it) {
                        sum += *it;
                }
                benchmark::DoNotOptimize(sum);
        }
}

// Independent operands from an array rather than one restated in place. Two spellings were tried first: a
// single operand shifted in place walks itself empty within ~86 iterations, and one copied fresh each
// iteration charges the shift for a copy that a copy-only control does not price the same way. operator<<=
// branches on the COUNT and never on the content -- an all-zero array measures the same as this one, which is
// how the in-place spelling was cleared -- so an array of operands times the shift and nothing else.
template<class T>
auto operands()
        -> std::vector<T>
{
        auto v = std::vector<T>(1024);
        for (auto& e : v) {
                e = filled<T>(400);
        }
        return v;
}

template<class T>
auto bm_shift_left(benchmark::State& state)
        -> void
{
        auto v = operands<T>();
        for (auto _ : state) {
                for (auto& e : v) {
                        e <<= 3UZ;
                        benchmark::DoNotOptimize(e);
                }
        }
        state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(v.size()));
}

template<class T>
auto bm_shift_right(benchmark::State& state)
        -> void
{
        auto v = operands<T>();
        for (auto _ : state) {
                for (auto& e : v) {
                        e >>= 3UZ;
                        benchmark::DoNotOptimize(e);
                }
        }
        state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(v.size()));
}

// A count the compiler cannot fold, which is what a caller shifting by a variable gets. The constant rows above
// let the whole sequence be specialized; this one keeps the general path, where a variable-count shift of a
// 128-bit block is a multi-instruction sequence rather than one shrx.
template<class T>
auto bm_shift_left_runtime(benchmark::State& state)
        -> void
{
        auto v = operands<T>();
        auto n = 3UZ;
        benchmark::DoNotOptimize(n);
        for (auto _ : state) {
                for (auto& e : v) {
                        e <<= n;
                        benchmark::DoNotOptimize(e);
                }
        }
        state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(v.size()));
}

}       // namespace

// The two classes are optional, so the ladder is built rung by rung rather than as one list.
#define BM_BUILTIN_RUNGS(fn)                                    \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint8_t >);          \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint16_t>);          \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint32_t>);          \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint64_t>);          \
        BENCHMARK_TEMPLATE(fn, set_of<xstd::uint128>)

#define BM_BUILTIN_RUNGS_D(fn, d)                               \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint8_t >, d);       \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint16_t>, d);       \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint32_t>, d);       \
        BENCHMARK_TEMPLATE(fn, set_of<std::uint64_t>, d);       \
        BENCHMARK_TEMPLATE(fn, set_of<xstd::uint128>, d)

#ifdef XSTD_BITS_BENCHMARK_HAS_ABSL_INT128
#define BM_ABSL_RUNG(fn)       BENCHMARK_TEMPLATE(fn, set_of<absl::uint128>)
#define BM_ABSL_RUNG_D(fn, d)  BENCHMARK_TEMPLATE(fn, set_of<absl::uint128>, d)
#else
#define BM_ABSL_RUNG(fn)
#define BM_ABSL_RUNG_D(fn, d)
#endif

#ifdef XSTD_BITS_BENCHMARK_HAS_BOOST_INT128
#define BM_BOOST_RUNG(fn)      BENCHMARK_TEMPLATE(fn, set_of<boost::int128::uint128>)
#define BM_BOOST_RUNG_D(fn, d) BENCHMARK_TEMPLATE(fn, set_of<boost::int128::uint128>, d)
#else
#define BM_BOOST_RUNG(fn)
#define BM_BOOST_RUNG_D(fn, d)
#endif

#define BM_LADDER(fn)           \
        BM_BUILTIN_RUNGS(fn);   \
        BM_ABSL_RUNG(fn);       \
        BM_BOOST_RUNG(fn)

#define BM_LADDER_D(fn, d)              \
        BM_BUILTIN_RUNGS_D(fn, d);      \
        BM_ABSL_RUNG_D(fn, d);          \
        BM_BOOST_RUNG_D(fn, d)

// Sparse, the density bitset/ops.cpp uses, and dense: the first is where skipping empty blocks pays and the
// last is where nothing is skipped and every step is the per-element primitive.
BM_LADDER_D(bm_forward,  50);
BM_LADDER_D(bm_forward, 400);
BM_LADDER_D(bm_forward, 900);
BM_LADDER_D(bm_reverse,  50);
BM_LADDER_D(bm_reverse, 400);
BM_LADDER_D(bm_reverse, 900);

BM_LADDER(bm_forward_clustered);
BM_LADDER(bm_reverse_clustered);

BM_LADDER(bm_shift_left);
BM_LADDER(bm_shift_right);
BM_LADDER(bm_shift_left_runtime);

BENCHMARK_MAIN();
