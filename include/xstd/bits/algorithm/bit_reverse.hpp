//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_REVERSE_HPP
#define XSTD_BITS_ALGORITHM_BIT_REVERSE_HPP

#include <xstd/bits/detail/algorithm.hpp> // storage_access, whole_sequence
#include <ranges>                         // borrowed_iterator_t, end

namespace xstd {

// std::ranges::reverse(r), a block at a time: the blocks end for end, each block's bits too, then the padding shifted out.
template<bits::detail::whole_sequence R>
        requires requires (R& r) { bits::detail::storage_access::bits(r).reverse(); }
constexpr auto bit_reverse(R&& r) noexcept
        -> std::ranges::borrowed_iterator_t<R>
{
        bits::detail::storage_access::bits(r).reverse();
        return std::ranges::borrowed_iterator_t<R>(std::ranges::end(r));
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_REVERSE_HPP
