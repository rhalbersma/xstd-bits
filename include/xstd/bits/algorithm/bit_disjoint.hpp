//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_DISJOINT_HPP
#define XSTD_BITS_ALGORITHM_BIT_DISJOINT_HPP

#include <xstd/bits/detail/algorithm.hpp> // reads_keys, storage_access
#include <type_traits>                    // type_identity_t

namespace xstd {

// Whether std::ranges::set_intersection(s1, s2) would be empty, a block at a time; s2 converts to s1's type.
template<bits::detail::reads_keys S>
        requires requires (S const& s) { bits::detail::storage_access::bits(s).intersects(bits::detail::storage_access::bits(s)); }
[[nodiscard]] constexpr auto bit_disjoint(S const& s1, std::type_identity_t<S> const& s2) noexcept
        -> bool
{
        return not bits::detail::storage_access::bits(s1).intersects(bits::detail::storage_access::bits(s2));
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_DISJOINT_HPP
