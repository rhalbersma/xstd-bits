//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SEQUENCE_HASH_HPP
#define XSTD_BITS_DETAIL_SEQUENCE_HASH_HPP

#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/bit_hash.hpp>            // bit_hash
#include <xstd/bits/detail/hash.hpp>                // hash_append_bools
#include <xstd/bits/detail/ownership.hpp>           // storage, storage_access, window
#include <xstd/bits/detail/sequence_adaptor.hpp>    // sequence_adaptor
#include <boost/hash2/hash_append_fwd.hpp>          // hash_append_tag
#include <boost/hash2/xxhash.hpp>                   // xxhash_64
#include <cstddef>                                  // size_t
#include <functional>                               // hash

namespace xstd::bits::detail {

// Hash2's hooks, found by ADL from a derived container too: the blocks, as std::array or std::vector writes them.
template<class Provider, class Hash, class Flavor, class Blocks, std::size_t N, bool Counted>
constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const& pr, Hash& h, Flavor const& f, bit_block_container<Blocks, N, Counted> const* v) noexcept
        -> void
{
        pr.hash_append(h, f, v->blocks());
}

// The owner's value as its model hashes it, bools then a run-time size; a view follows span and hashes none.
template<class Provider, class Hash, class Flavor, class Bits, window W, class Derived, std::size_t E>
constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const& pr, Hash& h, Flavor const& f, sequence_adaptor<Bits, storage::owned, W, Derived, E> const* v) noexcept
        -> void
{
        hash_append_bools(pr, h, f, storage_access::bits(*v));
}

} // namespace xstd::bits::detail

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
