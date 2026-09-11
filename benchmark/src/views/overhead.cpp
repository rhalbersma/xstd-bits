//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// What a reading costs when it is a view rather than a container, on the SAME backend block sequence.
//
// Three variants per reading, so the answer decomposes instead of arriving as one number:
//
//   owner            set_adaptor / sequence_adaptor owning the block_sequence      -- the baseline
//   view_of_storage  a view holding a POINTER to that same block_sequence          -- adds indirection
//   view_of_bitset   a view over the bitset_adaptor wrapping it                    -- adds trait forwarding
//
// So (view_of_storage - owner) is what the pointer costs, and (view_of_bitset - view_of_storage) is what
// bit_traits<bitset_adaptor> costs, which is the layer that forwards each entry to the storage's own trait.
// Every subject is passed through DoNotOptimize before the loop, including the owners. Without that the owner
// is a local the compiler can constant-fold straight through -- a count() folded at compile time measures 0.16 ns,
// half a cycle, which is not a faster reading but no reading at all -- while a view's pointer blocks the same
// folding. Comparing those two measures the folding, not the indirection. [design.md#a-bitset-reads-as-its-storage]

#include <xstd/bits/bit_array.hpp>          // bit_array
#include <xstd/bits/bit_set_view.hpp>       // bit_set_view
#include <xstd/bits/bit_span.hpp>           // bit_span
#include <xstd/bits/bit_static_set.hpp>     // bit_static_set
#include <xstd/bits/bitset.hpp>             // bitset
#include <xstd/bits/detail/block_array.hpp> // block_array
#include <benchmark/benchmark.h>            // ClobberMemory, DoNotOptimize, BENCHMARK_TEMPLATE, BENCHMARK_MAIN, State
#include <cstddef>                          // size_t
#include <cstdint>                          // uint64_t

namespace {

inline constexpr auto bits_per_word = 64UZ;

// The same bit pattern in every subject, so the three variants differ only in how the bits are reached.
constexpr auto is_set(std::size_t i)
        -> bool
{
        return (i % 5UZ) < 2UZ;         // ~40% set, deterministic
}

// One filler for all four subjects, because each reading spells "put a bit in" its own way: a bitset and a
// block_sequence take set(n), an ordered set takes insert(n), and a sequence of bool assigns through v[i].
template<std::size_t N, class T>
auto filled()
        -> T
{
        auto c = T();
        for (auto i = 0UZ; i < N; ++i) {
                if (!is_set(i)) {
                        continue;
                }
                if constexpr (requires { c.set(i); }) {
                        c.set(i);
                } else if constexpr (requires { c.insert(i); }) {
                        c.insert(i);
                } else {
                        c[i] = true;
                }
        }
        return c;
}

// ---------------------------------------------------------------------------------------------------------
// The set reading: iteration, which is the trait-heaviest operation there is -- every step is a find_next.
// ---------------------------------------------------------------------------------------------------------

template<std::size_t N>
auto set_iterate_owner(benchmark::State& state)
        -> void
{
        auto s = filled<N, xstd::bit_static_set<N>>();
        benchmark::DoNotOptimize(&s);
        for (auto _ : state) {
                auto sum = 0UZ;
                for (auto const pos : s) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
}

// The control: byte-identical to set_iterate_owner above. Any gap between the two is not a difference in
// what the code does, because there is none -- it is where the loop landed. [design.md#what-a-view-costs]
template<std::size_t N>
auto set_iterate_owner_twin(benchmark::State& state)
        -> void
{
        auto s = filled<N, xstd::bit_static_set<N>>();
        benchmark::DoNotOptimize(&s);
        for (auto _ : state) {
                auto sum = 0UZ;
                for (auto const pos : s) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
}

template<std::size_t N>
auto set_iterate_view_of_storage(benchmark::State& state)
        -> void
{
        auto blocks = filled<N, xstd::detail::bits::block_array<std::size_t, N>>();
        benchmark::DoNotOptimize(&blocks);
        auto const s = xstd::bit_set_view(blocks);
        for (auto _ : state) {
                auto sum = 0UZ;
                for (auto const pos : s) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
}

template<std::size_t N>
auto set_iterate_view_of_bitset(benchmark::State& state)
        -> void
{
        auto bits = filled<N, xstd::bitset<N>>();
        benchmark::DoNotOptimize(&bits);
        auto const s = xstd::bit_set_view<xstd::bitset<N>>(bits);
        for (auto _ : state) {
                auto sum = 0UZ;
                for (auto const pos : s) {
                        sum += pos;
                }
                benchmark::DoNotOptimize(sum);
        }
}

// ---------------------------------------------------------------------------------------------------------
// The sequence reading: count, which goes through num_blocks and block, the entries a forwarder most risks
// losing -- lose them and this walk silently becomes one bool at a time.
// ---------------------------------------------------------------------------------------------------------

template<std::size_t N>
auto sequence_count_owner(benchmark::State& state)
        -> void
{
        auto v = filled<N, xstd::bit_array<N>>();
        benchmark::DoNotOptimize(&v);
        for (auto _ : state) {
                benchmark::DoNotOptimize(v.count());
        }
}

template<std::size_t N>
auto sequence_count_view_of_storage(benchmark::State& state)
        -> void
{
        auto blocks = filled<N, xstd::detail::bits::block_array<std::size_t, N>>();
        benchmark::DoNotOptimize(&blocks);
        auto const v = xstd::bit_span(blocks);
        for (auto _ : state) {
                benchmark::DoNotOptimize(v.count());
        }
}

template<std::size_t N>
auto sequence_count_view_of_bitset(benchmark::State& state)
        -> void
{
        auto bits = filled<N, xstd::bitset<N>>();
        benchmark::DoNotOptimize(&bits);
        auto const v = xstd::bit_span<xstd::bitset<N>>(bits);
        for (auto _ : state) {
                benchmark::DoNotOptimize(v.count());
        }
}

// ---------------------------------------------------------------------------------------------------------
// The sequence reading again, element-wise: a random read, where the trait's at() is on the hot path and
// there is no word-parallelism to hide behind.
// ---------------------------------------------------------------------------------------------------------

constexpr auto next_index(std::uint64_t& lcg, std::size_t n)
        -> std::size_t
{
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<std::size_t>(lcg >> 33) % n;
}

template<std::size_t N>
auto sequence_read_owner(benchmark::State& state)
        -> void
{
        auto v = filled<N, xstd::bit_array<N>>();
        benchmark::DoNotOptimize(&v);
        auto lcg = std::uint64_t{1};
        for (auto _ : state) {
                benchmark::DoNotOptimize(static_cast<bool>(v[next_index(lcg, N)]));
        }
}

template<std::size_t N>
auto sequence_read_view_of_storage(benchmark::State& state)
        -> void
{
        auto blocks = filled<N, xstd::detail::bits::block_array<std::size_t, N>>();
        benchmark::DoNotOptimize(&blocks);
        auto const v = xstd::bit_span(blocks);
        auto lcg = std::uint64_t{1};
        for (auto _ : state) {
                benchmark::DoNotOptimize(static_cast<bool>(v[next_index(lcg, N)]));
        }
}

template<std::size_t N>
auto sequence_read_view_of_bitset(benchmark::State& state)
        -> void
{
        auto bits = filled<N, xstd::bitset<N>>();
        benchmark::DoNotOptimize(&bits);
        auto const v = xstd::bit_span<xstd::bitset<N>>(bits);
        auto lcg = std::uint64_t{1};
        for (auto _ : state) {
                benchmark::DoNotOptimize(static_cast<bool>(v[next_index(lcg, N)]));
        }
}

}       // namespace

// From four words up. One word is deliberately absent: a single popcount is one cycle, so a one-cycle
// difference between two variants reads as +100% and says nothing. [design.md#what-a-view-costs]
#define LADDER(fn)                                              \
        BENCHMARK_TEMPLATE(fn, 4UZ   * bits_per_word);          \
        BENCHMARK_TEMPLATE(fn, 16UZ  * bits_per_word);          \
        BENCHMARK_TEMPLATE(fn, 64UZ  * bits_per_word);          \
        BENCHMARK_TEMPLATE(fn, 256UZ * bits_per_word)

LADDER(set_iterate_owner);
LADDER(set_iterate_owner_twin);
LADDER(set_iterate_view_of_storage);
LADDER(set_iterate_view_of_bitset);

LADDER(sequence_count_owner);
LADDER(sequence_count_view_of_storage);
LADDER(sequence_count_view_of_bitset);

LADDER(sequence_read_owner);
LADDER(sequence_read_view_of_storage);
LADDER(sequence_read_view_of_bitset);

BENCHMARK_MAIN();
