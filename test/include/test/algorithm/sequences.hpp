//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_ALGORITHM_SEQUENCES_HPP
#define TEST_ALGORITHM_SEQUENCES_HPP

#include <cstddef> // size_t
#include <ranges>  // iota, size

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

} // namespace test::algorithm

#endif // TEST_ALGORITHM_SEQUENCES_HPP
