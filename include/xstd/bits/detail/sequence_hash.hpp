//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SEQUENCE_HASH_HPP
#define XSTD_BITS_DETAIL_SEQUENCE_HASH_HPP

#include <xstd/bits/detail/bit_hash.hpp>         // bit_hash
#include <xstd/bits/detail/ownership.hpp>        // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp> // sequence_adaptor
#include <boost/hash2/xxhash.hpp>                // xxhash_64
#include <cstddef>                               // size_t
#include <functional>                            // hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

// The owner's bits, by bit_hash, where std::vector<bool> has a hash; a view no more than std::span has.
template<class Bits, xstd::bits::detail::window W, class Derived>
struct hash<xstd::bits::detail::sequence_adaptor<Bits, xstd::bits::detail::storage::owned, W, Derived>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bits::detail::sequence_adaptor<Bits, xstd::bits::detail::storage::owned, W, Derived> const& v) const noexcept
                -> std::size_t
        {
                return xstd::bit_hash<xstd::bits::detail::sequence_adaptor<Bits, xstd::bits::detail::storage::owned, W, Derived>, boost::hash2::xxhash_64>()(v);
        }
};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_DETAIL_SEQUENCE_HASH_HPP
