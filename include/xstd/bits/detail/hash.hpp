//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_HASH_HPP
#define XSTD_BITS_DETAIL_HASH_HPP

#include <xstd/bits/detail/shift.hpp>          // shr
#include <boost/hash2/fnv1a.hpp>               // fnv1a_64
#include <boost/hash2/get_integral_result.hpp> // get_integral_result
#include <boost/hash2/hash_append.hpp>         // hash_append, hash_append_range, hash_append_size
#include <cstddef>                             // size_t
#include <cstdint>                             // uint64_t
#include <limits>                              // numeric_limits

namespace xstd::bits::detail {

// The value: the blocks and the width. Every storage reads by block, so equal values hash equal whatever holds them.
template<class Hash, class Flavor, class Bits>
constexpr auto hash_append_bits(Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        using block_type  = Bits::block_type;
        auto const blocks = c.blocks();
        if constexpr (std::numeric_limits<block_type>::digits > std::numeric_limits<std::uint64_t>::digits) {
                // A block wider than Hash2 writes, the 128-bit one, goes in as its two halves, low first.
                constexpr auto half = static_cast<unsigned>(std::numeric_limits<std::uint64_t>::digits);
                for (auto const b : blocks) {
                        boost::hash2::hash_append(h, f, static_cast<std::uint64_t>(b));
                        boost::hash2::hash_append(h, f, static_cast<std::uint64_t>(shr(b, half)));
                }
        } else {
                // Contiguous whole blocks, padding clear: one update writes the bytes a block at a time would.
                boost::hash2::hash_append_range(h, f, blocks.data(), blocks.data() + blocks.size());
        }
        boost::hash2::hash_append_size(h, f, c.size());
}

// The set reading at a run-time width: the positions held and their count, since equal sets need not share a width.
template<class Hash, class Flavor, class Bits>
constexpr auto hash_append_positions(Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        for (auto n = c.find_first(); n != c.size(); n = c.exclusive_find_next(n)) {
                // As the flavor writes a size, so a fixed size_type gives one message on 32- and 64-bit targets.
                boost::hash2::hash_append(h, f, static_cast<Flavor::size_type>(n));
        }
        boost::hash2::hash_append_size(h, f, c.count());
}

// The one place std::hash chooses an algorithm: fnv1a_64 is a default, and the parameter lets it be overridden.
template<class T, class Hash = boost::hash2::fnv1a_64>
[[nodiscard]] constexpr auto std_hash(T const& v, Hash h = {}) noexcept
        -> std::size_t
{
        boost::hash2::hash_append(h, {}, v);
        return boost::hash2::get_integral_result<std::size_t>(h);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_HASH_HPP
