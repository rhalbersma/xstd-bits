//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_FAST_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_FAST_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/bit_type_traits/bit_rebind.hpp>        // bit_rebind
#include <xstd/bits/detail/rebind.hpp>                     // rebind_width_v, resizable
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint16_t, uint32_t, uint8_t, uint_fast16_t, uint_fast32_t, uint_fast64_t, uint_fast8_t
#include <type_traits>                                     // conditional_t

namespace xstd {

// The fastest fixed-width block of at least N bits in one, else std::uint_fast64_t, of which N bits then take several.
template<std::size_t N>
using fast_block_t = std::conditional_t<
        (N <= bit_blocks_extent_v<std::uint8_t>), std::uint_fast8_t,
        std::conditional_t<
                (N <= bit_blocks_extent_v<std::uint16_t>), std::uint_fast16_t,
                std::conditional_t<(N <= bit_blocks_extent_v<std::uint32_t>), std::uint_fast32_t, std::uint_fast64_t>>>;

// The same bit container in the fastest block of at least its N bits, as uint_fast8_t is the fastest of at least 8.
template<class Owner>
        requires bits::detail::resizable<Owner>
using bit_fast = bit_rebind<fast_block_t<bits::detail::rebind_width_v<Owner>>, Owner>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_FAST_HPP
