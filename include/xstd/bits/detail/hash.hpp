//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_HASH_HPP
#define XSTD_BITS_DETAIL_HASH_HPP

#include <boost/hash2/fnv1a.hpp>              // fnv1a_64
#include <boost/hash2/get_integral_result.hpp> // get_integral_result
#include <boost/hash2/hash_append.hpp>        // hash_append
#include <xstd/bits/bit_traits.hpp>           // block_readable, count, find_first, find_next
#include <cstddef>                            // size_t
#include <cstdint>                            // uint64_t
#include <limits>                             // numeric_limits
#include <ranges>                             // iota

namespace xstd::detail::bits {

// A block wider than Hash2 writes, the 128-bit one, goes in as its two halves, low first.
template<class Hash, class Flavor, class Block>
constexpr auto hash_append_block(Hash& h, Flavor const& f, Block b)
        -> void
{
        constexpr auto half = static_cast<unsigned>(std::numeric_limits<std::uint64_t>::digits);
        if constexpr (std::numeric_limits<Block>::digits > std::numeric_limits<std::uint64_t>::digits) {
                boost::hash2::hash_append(h, f, static_cast<std::uint64_t>(b));
                boost::hash2::hash_append(h, f, static_cast<std::uint64_t>(b >> half));
        } else {
                boost::hash2::hash_append(h, f, b);
        }
}

// The value through the trait: the blocks and the width where the storage reads by block, every position and the width otherwise. Equal values hash equal whatever holds them, so no storage's own hook is asked. [design.md#the-hashing-invariant]
template<class Traits, class Hash, class Flavor, class Bits>
constexpr auto hash_append_bits(Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        if constexpr (block_readable<Traits, Bits>) {
                for (auto const i : std::views::iota(0UZ, Traits::num_blocks(c))) {
                        hash_append_block(h, f, Traits::block(c, i));
                }
        } else {
                for (auto const i : std::views::iota(0UZ, Traits::size(c))) {
                        boost::hash2::hash_append(h, f, Traits::at(c, i));
                }
        }
        boost::hash2::hash_append(h, f, Traits::size(c));
}

// The set reading at a run-time width: the positions held and their count, since equal sets need not share a width. [design.md#width-is-capacity]
template<class Traits, class Hash, class Flavor, class Bits>
constexpr auto hash_append_positions(Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        for (auto n = find_first<Traits>(c); n != Traits::size(c); n = find_next<Traits>(c, n)) {
                boost::hash2::hash_append(h, f, n);
        }
        boost::hash2::hash_append(h, f, count<Traits>(c));
}

// The one place std::hash chooses an algorithm; a caller wanting another brings it through hash_append. [design.md#the-hashing-invariant]
template<class T>
[[nodiscard]] constexpr auto std_hash(T const& v) noexcept
        -> std::size_t
{
        boost::hash2::fnv1a_64 h;
        boost::hash2::hash_append(h, {}, v);
        return boost::hash2::get_integral_result<std::size_t>(h);
}

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_HASH_HPP
