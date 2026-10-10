//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_ROTATE_HPP
#define XSTD_BITS_ALGORITHM_BIT_ROTATE_HPP

#include <xstd/bits/detail/algorithm.hpp> // storage_access, whole_sequence
#include <cassert>                        // assert
#include <cstddef>                        // size_t
#include <ranges>                         // begin, borrowed_subrange_t, end, iterator_t, size

namespace xstd {

// std::ranges::rotate(r, middle), a block at a time: bit i takes bit (i + n) % size(), for n = middle - begin.
template<bits::detail::whole_sequence R>
        requires requires (R& r, std::size_t n) { bits::detail::storage_access::bits(r).rotate(n); }
constexpr auto bit_rotate(R&& r, std::ranges::iterator_t<R> middle) noexcept
        -> std::ranges::borrowed_subrange_t<R>
{
        auto const first = std::ranges::begin(r);
        auto const last  = std::ranges::end(r);
        assert(first <= middle and middle <= last);
        bits::detail::storage_access::bits(r).rotate(static_cast<std::size_t>(middle - first));
        return std::ranges::borrowed_subrange_t<R>(first + (last - middle), last);
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_ROTATE_HPP
