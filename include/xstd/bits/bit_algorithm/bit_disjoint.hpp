//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_DISJOINT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_DISJOINT_HPP

#include <xstd/bits/bit_algorithm/bit_intersects.hpp> // bit_intersects
#include <xstd/bits/detail/ownership.hpp>             // reads_keys
#include <type_traits>                                // type_identity_t

namespace xstd::ranges {

// Whether std::ranges::set_intersection(s1, s2) would be empty, a block at a time; s2 converts to s1's type.
template<bits::detail::reads_keys S>
[[nodiscard]] constexpr auto bit_disjoint(S const& s1, std::type_identity_t<S> const& s2) noexcept
        -> bool
{
        return not xstd::ranges::bit_intersects(s1, s2);
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_DISJOINT_HPP
