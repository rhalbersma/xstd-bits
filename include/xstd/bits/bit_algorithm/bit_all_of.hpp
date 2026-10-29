//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_ALL_OF_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_ALL_OF_HPP

#include <xstd/bits/bit_algorithm/detail/count.hpp> // all_true
#include <xstd/bits/detail/ownership.hpp>           // reads_bools

namespace xstd::ranges {

// std::ranges::all_of(r, std::identity()), a block at a time: true on an empty sequence.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_all_of(R const& r) noexcept
        -> bool
{
        return bits::detail::all_true(r);
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_ALL_OF_HPP
