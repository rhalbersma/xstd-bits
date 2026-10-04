//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_STATIC_BLOCK_CAPACITY_HPP
#define XSTD_BITS_DETAIL_STATIC_BLOCK_CAPACITY_HPP

#include <cstddef>     // size_t
#include <span>        // dynamic_extent
#include <type_traits> // integral_constant, is_pointer_v

namespace xstd::bits::detail {

// In blocks, the capacity the type answers without an object, else dynamic_extent.
template<class Bits>
consteval auto static_block_capacity() noexcept
        -> std::size_t
{
        // A capacity() usable as a constant, as std::inplace_vector's is.
        if constexpr (requires { typename std::integral_constant<std::size_t, Bits::capacity()>; }) {
                return Bits::capacity();
        } else if constexpr (requires { requires std::is_pointer_v<decltype(&Bits::capacity)>; typename std::integral_constant<std::size_t, Bits::static_capacity>; }) {
                // A static run-time capacity(), as boost::container::static_vector's, names static_capacity too.
                return Bits::static_capacity;
        } else {
                return std::dynamic_extent;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_STATIC_BLOCK_CAPACITY_HPP
