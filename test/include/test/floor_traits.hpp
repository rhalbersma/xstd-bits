//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_FLOOR_TRAITS_HPP
#define TEST_FLOOR_TRAITS_HPP

#include <xstd/bits/bit_traits.hpp> // bit_traits
#include <cstddef>                  // size_t

namespace test {

// The floor and nothing more, over storage whose own trait answers everything natively: one variable, two tiers. [design.md#the-trait-is-a-parameter]
template<class Bits>
struct floor_traits
{
        static constexpr std::size_t extent = xstd::bit_traits<Bits>::extent;

        [[nodiscard]] static constexpr auto size(Bits const& c) noexcept -> std::size_t { return xstd::bit_traits<Bits>::size(c); }
        [[nodiscard]] static constexpr auto at(Bits const& c, std::size_t n) noexcept -> bool { return xstd::bit_traits<Bits>::at(c, n); }
};

} // namespace test

#endif // TEST_FLOOR_TRAITS_HPP
