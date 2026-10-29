//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_HAS_SINGLE_BIT_HPP
#define XSTD_BITS_BIT_HAS_SINGLE_BIT_HPP

#include <xstd/bits/detail/ownership.hpp>   // owner, storage_access
#include <xstd/ints/bit/has_single_bit.hpp> // IWYU pragma: export; has_single_bit
#include <algorithm>                        // find_if, none_of
#include <iterator>                         // end, next

// [bit.pow.two]'s has_single_bit over one owner, of any width, beside xstd-ints' over one integer.
namespace xstd {

// Exactly one position set: the first block holding any is a power of two, and every block after it is nought.
template<class T>
        requires bits::detail::owner<T>
[[nodiscard]] constexpr auto has_single_bit(T const& x) noexcept
        -> bool
{
        auto const blocks  = bits::detail::storage_access::bits(x).blocks();
        auto const any_set = []<class Block>(Block block) -> bool { return block != Block{}; };
        auto const first   = std::ranges::find_if(blocks, any_set);
        return first != std::ranges::end(blocks) and xstd::has_single_bit(*first) and std::ranges::none_of(std::ranges::next(first), std::ranges::end(blocks), any_set);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_HAS_SINGLE_BIT_HPP
