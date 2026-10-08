//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_LEAST_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_LEAST_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/rebind.hpp>                     // rebind_t, rebind_width_v, rebindable
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint16_t, uint32_t, uint64_t, uint8_t
#include <type_traits>                                     // conditional_t

namespace xstd {

// The narrowest fixed-width block holding N bits in one, else std::uint64_t, of which N bits then take several.
template<std::size_t N>
using least_block_t = std::conditional_t<
        (N <= bit_blocks_extent_v<std::uint8_t>), std::uint8_t,
        std::conditional_t<
                (N <= bit_blocks_extent_v<std::uint16_t>), std::uint16_t,
                std::conditional_t<(N <= bit_blocks_extent_v<std::uint32_t>), std::uint32_t, std::uint64_t>>>;

// The same bit container in the smallest block that holds its N bits, as uint_least8_t is the smallest of at least 8.
template<class Owner>
        requires bits::detail::rebindable<Owner>
using bit_least = bits::detail::rebind_t<Owner, least_block_t<bits::detail::rebind_width_v<Owner>>, bits::detail::rebind_width_v<Owner>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_LEAST_HPP
