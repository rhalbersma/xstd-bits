//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_ALL_OF_HPP
#define XSTD_BITS_ALGORITHM_BIT_ALL_OF_HPP

#include <xstd/bits/detail/algorithm.hpp> // all_true, reads_bools

namespace xstd {

// std::ranges::all_of(r, std::identity()), a block at a time: true on an empty sequence.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_all_of(R const& r) noexcept
        -> bool
{
        return bits::detail::all_true(r);
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_ALL_OF_HPP
