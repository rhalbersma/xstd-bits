//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_ALGORITHM_SEQUENCES_HPP
#define TEST_ALGORITHM_SEQUENCES_HPP

#ifdef _MSC_VER

// xstd::bit_array<0> gives bogus "unreachable code" warnings, ahead of the containers this header includes
#pragma warning(disable : 4702)

#endif

#include <test/sequence/factory.hpp> // limit_v, make_sequence, static_width, stripes
#include <test/spec/sequence.hpp>    // dynamic, fixed, inplace, small
#include <xstd/bits/bit_span.hpp>    // bit_span
#include <xstd/bits/bit_vector.hpp>  // basic_bit_vector
#include <algorithm>                 // min
#include <array>                     // array
#include <cstddef>                   // ptrdiff_t, size_t
#include <ranges>                    // begin, end, iota, size, subrange
#include <tuple>                     // tuple_cat
#include <utility>                   // declval
#include <vector>                    // vector

// Patterned sequences and their windows, for the algorithms to be checked against std::ranges over the same bools.
namespace test::algorithm {

// A pattern with no period a block boundary could hide behind.
[[nodiscard]] constexpr auto in_pattern(std::size_t i) noexcept
        -> bool
{
        return ((i * 7UZ) + 3UZ) % 5UZ < 2UZ;
}

// A copy of s with position i set to in_pattern(i).
template<class S>
[[nodiscard]] constexpr auto patterned(S s)
        -> S
{
        for (auto const i : std::views::iota(0UZ, std::ranges::size(s))) {
                s[i] = in_pattern(i);
        }
        return s;
}

// The bools a sequence holds, in a constant expression too: what every effect is stated over.
template<class R>
[[nodiscard]] constexpr auto bools_of(R const& r)
        -> std::vector<bool>
{
        return std::vector<bool>(std::ranges::begin(r), std::ranges::end(r));
}

// A copy of s whose positions from k on are set and those before it clear: sorted, and partitioned the other way.
template<class S>
[[nodiscard]] constexpr auto sorted(S s, std::size_t k)
        -> S
{
        for (auto const i : std::views::iota(0UZ, std::ranges::size(s))) {
                s[i] = k <= i;
        }
        return s;
}

// f(w) for every window w = span.subspan(lo, hi - lo), the empty ones included.
template<class Span, class F>
constexpr auto for_each_window(Span const& span, F f)
        -> void
{
        auto const n = std::ranges::size(span);
        for (auto const lo : std::views::iota(0UZ, n + 1UZ)) {
                for (auto const hi : std::views::iota(lo, n + 1UZ)) {
                        f(span.subspan(lo, hi - lo));
                }
        }
}

// Whether f, writing through every window of a copy of a, does what g does to the same positions of its bools.
template<class A, class F, class G>
[[nodiscard]] auto writes_as_model(A const& a, F f, G g)
        -> int
{
        auto const n = std::ranges::size(a);
        auto wrong   = 0;
        for (auto const lo : std::views::iota(0UZ, n + 1UZ)) {
                for (auto const hi : std::views::iota(lo, n + 1UZ)) {
                        auto b     = a;
                        auto model = bools_of(a);
                        f(xstd::bit_span(b).subspan(lo, hi - lo));
                        g(std::ranges::subrange(model.begin() + static_cast<std::ptrdiff_t>(lo), model.begin() + static_cast<std::ptrdiff_t>(hi)));
                        wrong += static_cast<int>(bools_of(b) != model);
                }
        }
        return wrong;
}

// A destination of n bools over Block, striped so a write past its range or short of it shows.
template<class Block>
[[nodiscard]] auto striped_vector(std::size_t n)
        -> xstd::basic_bit_vector<Block>
{
        return test::sequence::make_sequence<xstd::basic_bit_vector<Block>>(n, [](std::size_t i) -> bool { return test::sequence::stripes(i); });
}

// The offsets a destination is written at: aligned, inside a narrow block, and either side of a wide one's boundary.
inline constexpr auto offsets = std::array{0UZ, 3UZ, 13UZ, 63UZ, 64UZ};

// Every column of ours the sequence clauses sweep, their standard models left out: blocks, widths and storages.
using sequences = decltype(std::tuple_cat(std::declval<test::spec::sequence::fixed>(), std::declval<test::spec::sequence::dynamic>(), std::declval<test::spec::sequence::small>(), std::declval<test::spec::sequence::inplace>()));

// The widest run-time width an input takes, past two blocks of the widest builtin Block.
inline constexpr auto sequence_limit = 130UZ;

// The widths an input of X takes: its own where the type fixes it, else the edges of the narrow and wide blocks.
template<class X>
[[nodiscard]] constexpr auto widths()
        -> std::array<std::size_t, 10>
{
        constexpr auto limit = test::sequence::limit_v<X, sequence_limit>;
        if constexpr (test::sequence::static_width<X>) {
                return {limit, limit, limit, limit, limit, limit, limit, limit, limit, limit};
        } else {
                auto nrv = std::array{0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, 63UZ, 64UZ, 65UZ, 130UZ};
                for (auto& n : nrv) {
                        n = std::ranges::min(n, limit);
                }
                return nrv;
        }
}

// The patterns an input holds at width n: empty and full, striped either way, one bool set or clear, and sorted runs.
template<class F>
constexpr auto for_each_pattern(std::size_t n, F f)
        -> void
{
        f([](std::size_t) -> bool { return false; });
        f([](std::size_t) -> bool { return true; });
        f([](std::size_t i) -> bool { return test::sequence::stripes(i); });
        f([](std::size_t i) -> bool { return not test::sequence::stripes(i); });
        for (auto const p : {0UZ, n / 2UZ, n - 1UZ}) {
                f([p](std::size_t i) -> bool { return i == p; });
                f([p](std::size_t i) -> bool { return i != p; });
        }
        for (auto const p : {n / 3UZ, n - (n / 3UZ)}) {
                f([p](std::size_t i) -> bool { return p <= i; });
                f([p](std::size_t i) -> bool { return i < p; });
        }
}

// f(x) for every input of X, each width once over a static width, under every pattern.
template<class X, class F>
auto for_each_sequence(F f)
        -> void
{
        constexpr auto ns       = widths<X>();
        constexpr auto distinct = test::sequence::static_width<X> ? 1UZ : ns.size();
        for (auto const k : std::views::iota(0UZ, distinct)) {
                for_each_pattern(ns[k], [&](auto pred) -> void { f(test::sequence::make_sequence<X>(ns[k], pred)); });
        }
}

} // namespace test::algorithm

#endif // TEST_ALGORITHM_SEQUENCES_HPP
