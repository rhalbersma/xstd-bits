//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_DETAIL_SET_HASH_HPP
#define XSTD_BITS_BIT_SET_DETAIL_SET_HASH_HPP

#include <xstd/bits/bit_set/detail/set_adaptor.hpp> // set_adaptor
#include <xstd/bits/detail/bit_hash.hpp>            // bit_hash
#include <xstd/bits/detail/hash.hpp>                // hash_append_keys
#include <xstd/bits/detail/ownership.hpp>           // storage
#include <boost/hash2/hash_append_fwd.hpp>          // hash_append_tag
#include <boost/hash2/xxhash.hpp>                   // xxhash_64
#include <cstddef>                                  // size_t
#include <functional>                               // hash

namespace xstd::bits::detail {

// Hash2's hook, found by ADL from a derived set too: as std::set<Key, Compare> hashes, the keys in order, their count.
template<class Provider, class Hash, class Flavor, class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const& pr, Hash& h, Flavor const& f, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const* v) noexcept
        -> void
{
        hash_append_keys(pr, h, f, *v);
}

} // namespace xstd::bits::detail

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

// Owned or viewed, as std::string_view hashes and std::set does not: the bits, by bit_hash.
template<class Bits, xstd::bits::detail::storage Store, class Derived, class Key, class KeyMapping, class Compare>
struct hash<xstd::bits::detail::set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bits::detail::set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& v) const noexcept
                -> std::size_t
        {
                return xstd::bit_hash<xstd::bits::detail::set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>, boost::hash2::xxhash_64>()(v);
        }
};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SET_DETAIL_SET_HASH_HPP
