//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_ROTATE_COPY_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_ROTATE_COPY_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // bool_output_iterator, first_position, reads_bools
#include <xstd/bits/bit_algorithm/detail/blit.hpp>      // copy_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <algorithm>                                    // rotate_copy_result
#include <cassert>                                      // assert
#include <cstddef>                                      // ptrdiff_t, size_t
#include <ranges>                                       // begin, borrowed_iterator_t, end, iterator_t, size

namespace xstd {

// std::ranges::rotate_copy(r, middle, out): [middle, end) then [begin, middle), each copied a block at a time.
template<bits::detail::reads_bools R, bits::detail::bool_output_iterator O>
constexpr auto bit_rotate_copy(R&& r, std::ranges::iterator_t<R> middle, O out) noexcept
        -> std::ranges::rotate_copy_result<std::ranges::borrowed_iterator_t<R>, O>
{
        using bits::detail::storage_access;
        auto const first = std::ranges::begin(r);
        assert(first <= middle and middle <= std::ranges::end(r));
        auto const n = std::ranges::size(r);
        auto const m = static_cast<std::size_t>(middle - first);
        auto const s = bits::detail::first_position(r);
        auto&& dst   = storage_access::iterated_bits(out);
        auto const d = storage_access::position(out);
        bits::detail::copy_bools(dst, d, storage_access::bits(r), s + m, n - m);
        bits::detail::copy_bools(dst, d + n - m, storage_access::bits(r), s, m);
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), out + static_cast<std::ptrdiff_t>(n)};
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_ROTATE_COPY_HPP
