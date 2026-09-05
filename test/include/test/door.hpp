//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_DOOR_HPP
#define TEST_DOOR_HPP

#include <xstd/bits/bit_traits.hpp> // bit_traits, bit_storage, block_readable, static_bit_extent
#include <compare>                  // strong_ordering
#include <cstddef>                  // size_t
#include <vector>                   // vector

// Every entry the readings will ask for, driven once against a model, so a forwarding specialization cannot silently drop one.
namespace test {

// Disagreements counted rather than asserted, so one call sites reports the width that broke. [design.md#counted-not-asserted]
template<class T>
[[nodiscard]] auto door_disagreements() -> int
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        auto disagreed = 0;
        auto const differs = [&](bool wrong) -> void { disagreed += static_cast<int>(wrong); };

        auto c = T();
        differs(traits::size(c) != N);
        differs(traits::count(c) != 0UZ);
        differs(traits::num_blocks(c) == 0UZ);
        differs(traits::find_first(c) != N);
        differs(traits::find_last(c) != N);

        // fill both ways, which is what the set reading's clear() and the sequence reading's fill() become.
        traits::fill(c, true);
        differs(traits::count(c) != N);
        traits::fill(c, false);
        differs(traits::count(c) != 0UZ);

        return disagreed;
}

// The position-dependent entries, which a zero width has none to drive. [design.md#per-instantiation-slots]
template<class T>
[[nodiscard]] auto door_disagreements_at_positions() -> int
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        auto disagreed = 0;
        auto const differs = [&](bool wrong) -> void { disagreed += static_cast<int>(wrong); };

        for (auto i = 0UZ; i < N; ++i) {
                auto c = T();
                traits::insert(c, i);
                differs(not traits::at(c, i));
                differs(traits::count(c) != 1UZ);
                differs(traits::find_first(c) != i);
                differs(traits::find_prev(c, N) != i);
                differs(traits::find_next(c, i) != N);
                differs(xstd::detail::bits::scan_count<traits>(c) != 1UZ);

                traits::assign(c, i, false);
                differs(traits::at(c, i));
        }
        return disagreed;
}

// Both orderings, on a pair that differs at one position: whoever holds it is greater under either reading.
template<class T>
[[nodiscard]] auto door_ordering_disagreements() -> int
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        if constexpr (N == 0) {
                return 0;
        } else {
                auto disagreed = 0;
                auto held = T();
                traits::insert(held, N - 1UZ);
                auto const empty = T();

                disagreed += static_cast<int>(traits::set_three_way(held, empty)      != std::strong_ordering::greater);
                disagreed += static_cast<int>(traits::sequence_three_way(held, empty) != std::strong_ordering::greater);
                disagreed += static_cast<int>(traits::set_three_way(empty, empty)     != std::strong_ordering::equal);
                return disagreed;
        }
}

} // namespace test

#endif // TEST_DOOR_HPP
