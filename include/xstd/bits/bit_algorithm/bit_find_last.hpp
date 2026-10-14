//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_FIND_LAST_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_FIND_LAST_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // first_position, reads_bools
#include <xstd/bits/bit_algorithm/detail/find.hpp>      // find_last_value
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <cstddef>                                      // ptrdiff_t
#include <ranges>                                       // begin, borrowed_subrange_t, end, size

namespace xstd {

// std::ranges::find_last(r, value), a block at a time from the top: the last position holding value, to the end.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_find_last(R&& r, bool value) noexcept
        -> std::ranges::borrowed_subrange_t<R>
{
        auto const i = bits::detail::find_last_value(bits::detail::storage_access::bits(r), bits::detail::first_position(r), std::ranges::size(r), value);
        return std::ranges::borrowed_subrange_t<R>(std::ranges::begin(r) + static_cast<std::ptrdiff_t>(i), std::ranges::end(r));
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_FIND_LAST_HPP
