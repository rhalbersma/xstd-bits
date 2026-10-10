//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_INCLUDES_HPP
#define XSTD_BITS_ALGORITHM_BIT_INCLUDES_HPP

#include <xstd/bits/detail/algorithm.hpp> // reads_keys, storage_access
#include <type_traits>                    // type_identity_t

namespace xstd {

// std::ranges::includes(s1, s2), a block at a time; s2 converts to s1's type, as a flag does to its flag set.
template<bits::detail::reads_keys S>
        requires requires (S const& s) { bits::detail::storage_access::bits(s).is_subset_of(bits::detail::storage_access::bits(s)); }
[[nodiscard]] constexpr auto bit_includes(S const& s1, std::type_identity_t<S> const& s2) noexcept
        -> bool
{
        return bits::detail::storage_access::bits(s2).is_subset_of(bits::detail::storage_access::bits(s1));
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_INCLUDES_HPP
