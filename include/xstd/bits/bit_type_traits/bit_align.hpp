//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_ALIGN_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_ALIGN_HPP

#include <xstd/bits/bit_type_traits/bit_resize.hpp> // bit_resize
#include <xstd/bits/detail/num_blocks.hpp>          // whole_blocks_width_v
#include <xstd/bits/detail/rebind.hpp>              // rebind_block_t, rebind_width_v, resizable

namespace xstd {

// The same bit container with N rounded up to a whole number of its blocks, so that no block carries an unused tail.
template<class Owner>
        requires bits::detail::resizable<Owner>
using bit_align = bit_resize<bits::detail::whole_blocks_width_v<bits::detail::rebind_block_t<Owner>, bits::detail::rebind_width_v<Owner>>, Owner>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_ALIGN_HPP
