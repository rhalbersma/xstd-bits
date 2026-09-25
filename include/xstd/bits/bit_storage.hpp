//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STORAGE_HPP
#define XSTD_BITS_BIT_STORAGE_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <ranges>                                  // contiguous_range, range_size_t, range_value_t, sized_range
#include <type_traits>                             // remove_const_t

// What every container and view here presents a packed interface over: bits in contiguous unsigned words.
namespace xstd {

// One unsigned word, or a sized contiguous range of them that subscripts; const where a view only reads.
template<class Bits>
concept bit_storage =
        xstd::unsigned_integer<std::remove_const_t<Bits>> or
        (std::ranges::sized_range<Bits> and std::ranges::contiguous_range<Bits> and
         xstd::unsigned_integer<std::remove_const_t<std::ranges::range_value_t<Bits>>> and
         requires (Bits& bits, std::ranges::range_size_t<Bits> n) { bits[n]; });

} // namespace xstd

#endif // XSTD_BITS_BIT_STORAGE_HPP
