//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_HASH_HPP
#define XSTD_BITS_DETAIL_HASH_HPP

#include <xstd/ints/ext/boost/hash2.hpp> // hash_append_int
#include <xstd/misc/ext/boost/hash2.hpp> // hash, long_hash, short_hash
#include <boost/hash2/flavor.hpp>        // default_flavor
#include <cstddef>                       // size_t
#include <cstdint>                       // uint64_t
#include <limits>                        // numeric_limits
#include <type_traits>                   // conditional_t

namespace xstd::bits::detail {

// The value: the blocks and the width. Every storage reads by block, so equal values hash equal whatever holds them.
template<class Provider, class Hash, class Flavor, class Bits>
constexpr auto hash_append_bits(Provider const& pr, Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        using block_type  = Bits::block_type;
        auto const blocks = c.blocks();
        if constexpr (std::numeric_limits<block_type>::digits > std::numeric_limits<std::uint64_t>::digits) {
                // Wider than any integer Hash2 writes: hash_append_int puts each block in as 64-bit words, low first.
                for (auto const b : blocks) {
                        xstd::hash_append_int(h, f, b);
                }
        } else {
                // Contiguous whole blocks, padding clear: one update writes the bytes a block at a time would.
                pr.hash_append_range(h, f, blocks.data(), blocks.data() + blocks.size());
        }
        pr.hash_append_size(h, f, c.size());
}

// The set reading at a run-time width: the positions held and their count, since equal sets need not share a width.
template<class Provider, class Hash, class Flavor, class Bits>
constexpr auto hash_append_positions(Provider const& pr, Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        for (auto n = c.find_first(); n != c.size(); n = c.exclusive_find_next(n)) {
                // As the flavor writes a size, so a fixed size_type gives one message on 32- and 64-bit targets.
                pr.hash_append(h, f, static_cast<Flavor::size_type>(n));
        }
        pr.hash_append_size(h, f, c.count());
}

// One word: FNV-1a has no setup to amortize over it, and xxHash, taking the input in stripes, wins from two on.
inline constexpr auto short_message_size = sizeof(std::uint64_t);

// What std::hash folds the value to under an algorithm: the public hasher of the value's own type, unseeded.
template<class Hash, class T>
[[nodiscard]] constexpr auto std_hash(T const& v) noexcept
        -> std::size_t
{
        auto const hasher = xstd::hash<T, Hash>();
        return hasher(v);
}

// The default, by the bytes appended ahead of the width or count, which equal values share whatever holds them.
template<class T>
[[nodiscard]] constexpr auto std_hash_by_size(T const& v, std::size_t message_size) noexcept
        -> std::size_t
{
        if (message_size <= short_message_size) {
                return std_hash<xstd::short_hash>(v);
        }
        return std_hash<xstd::long_hash>(v);
}

// The bit reading's default, by its blocks' bytes, chosen at compile time where the width is static.
template<class T, class Bits>
[[nodiscard]] constexpr auto std_hash_bits(T const& v, Bits const& c) noexcept
        -> std::size_t
{
        constexpr auto block_size = sizeof(typename Bits::block_type);
        if constexpr (Bits::has_static_size) {
                using hash_type = std::conditional_t<(Bits::blocks_for(Bits::extent) * block_size <= short_message_size), xstd::short_hash, xstd::long_hash>;
                return std_hash<hash_type>(v);
        } else {
                return std_hash_by_size(v, c.num_blocks() * block_size);
        }
}

// The position reading's default, by the bytes its positions take under the default flavor, which xstd::hash uses.
template<class T, class Bits>
[[nodiscard]] constexpr auto std_hash_positions(T const& v, Bits const& c) noexcept
        -> std::size_t
{
        return std_hash_by_size(v, c.count() * sizeof(boost::hash2::default_flavor::size_type));
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_HASH_HPP
