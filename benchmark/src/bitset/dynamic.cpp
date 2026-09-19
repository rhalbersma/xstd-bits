//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The run-time width against boost's, on the same word ladder the static width runs in ops.cpp.

#include <xstd/bits/bit_set_view.hpp>             // bit_set_view
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <benchmark/benchmark.h>                  // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE1, BENCHMARK_MAIN, State
#include <cstddef>                                // size_t
#include <cstdint>                                // int64_t, uint64_t
#include <vector>                                 // vector

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
                if ((lcg >> 33) % 5UZ < 2UZ) {          // ~40% set
                        bits.set(i);
                }
        }
        return bits;
}

// The bits as their blocks, taken out once so both halves of the interface below run over the same words at the same density.
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

}       // namespace

#define BM_BINARY(name, op)                                                     \
        template<class T>                                                       \
        auto name(benchmark::State& state)                                      \
                -> void                                                         \
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

// Like for like, which this row was not. Boost answers find_first/find_next natively and so does the bitset reading here, so both rungs now make the SAME two calls. What stood here walked ours through bit_set_view's iterator and gave boost a bit-by-bit test(pos) loop instead -- the loop a std::bitset user writes, std::bitset having no scan to call, and not the loop a boost user writes.
// It did not flatter us, it flattered boost, which is the harder way to be wrong. At the top rung, 32768 bits and 40% set, a per-bit loop over boost costs 28.3us and boost's own find_next costs 61.9us: testing every position is a predictable branch per bit, where a scan re-enters the block it was given and carries a dependency from one position to the next. So the old row read ours 1.8x SLOWER than boost by comparing our scan against its per-bit loop; the same two calls on both sides put ours at 53.0us against 61.9us, which is 0.86x.
// Those are medians of seven runs, and worth reading as such: the two re-entrant scans are latency-bound and move by a tenth between runs on a shared machine, where the block walk below lands within 3% every time.
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

// What the iterator costs on top of that scan: the same positions, reached the way a range-for reaches them rather than by naming the two calls. Ours alone, boost having no view over its bits -- the rung it is read against is its own in bm_scan, which is now the same walk.
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

// The walk an iterator cannot be, and the reason to have both: a scan restarts from the position it was given, so it re-reads that position's block on every step and cannot start the next step until this one answers, where this loads each block ONCE and then spends tzcnt for the position and blsr to drop it. Ours alone again, and it is the row worth reading -- 7.0us against the 53.0us scan at the top rung, some seven times, and four times the per-bit loop that beat the scan.
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

// The block interface, and the row where boost stands on exactly the same footing by having the same thing: from_block_range and to_block_range are boost's too, each of them a std::copy over its own vector, so this is a like-for-like pair rather than one of the ours-only walks above.
// Both sides take the same words through the same contiguous iterators, and everything is allocated before the loop. That last part is the row rather than an aside about it: build the destination inside the timed region instead and the same call reads about twice what it reads here, which is the allocator measured in place of the copy.
// At the top rung, 32768 bits and medians of seven with each side timed in both positions, the two land together: 35ns in against boost's 35ns, 36ns out against boost's 36ns, on a memcpy of the same words at 35ns. Both are one bulk copy by then and neither sits above the floor -- which is the thing this row exists to keep checking rather than assume, the loop that used to stand on our side of it having run some 3.4x the floor.
// Ours also does one thing boost does not, and it is inside those numbers: erase_unused() masks the tail once the blocks have landed, where boost keeps whatever the caller's last block held. One word at every width, which is what the stronger guarantee costs.
template<class T>
auto bm_from_block_range(benchmark::State& state)
        -> void
{
        auto const blocks = blocks_of(filled<T>(words(state) * bits_per_word, 1));
        auto a = T(words(state) * bits_per_word);
        for (auto _ : state) {
                benchmark::DoNotOptimize(blocks);
                from_block_range(blocks.begin(), blocks.end(), a);
                benchmark::ClobberMemory();
        }
        per_byte(state);
}

// The way back out, into a contiguous output iterator, which is what a caller reaches for when the blocks are going into a buffer that already exists.
// A back_inserter is the other shape and gets no rung of its own: it costs a push_back per block, 320ns against this row's 36ns at the top rung, and what that measures is a vector growing rather than the interface answering.
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

// A run-time width takes the ladder as a Range where the static one needs a template list; the rungs are the same 1, 2, 4 ... 512 words, so the two files' rows can be read against each other.
// Ours first and alone, because two of the rows below are walks only ours has a spelling for. Those take no counterpart rung of their own: what they are read against is boost's rung in BM_LADDER(bm_scan), which measures the same bits by the only walk boost offers.
#define BM_LADDER_OURS(fn)                                                              \
        BENCHMARK_TEMPLATE1(fn, xstd::dynamic_bitset)                                   \
                ->RangeMultiplier(2)->Range(1, 512)

#define BM_LADDER(fn)                                                                   \
        BENCHMARK_TEMPLATE1(fn, boost::dynamic_bitset<std::uint64_t>)                   \
                ->RangeMultiplier(2)->Range(1, 512);                                    \
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
