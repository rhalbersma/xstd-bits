//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_HASH_HPP
#define XSTD_BITS_DETAIL_HASH_HPP

#include <xstd/bits/detail/bit_layout.hpp> // bits_per_byte, block_digits, blocks_copy_as_bytes, byte_count, bytes_per_block
#include <xstd/bits/detail/intrin.hpp>     // bools_per_block, expand_block
#include <xstd/bits/detail/ownership.hpp>  // owner, set_reading_tag, storage_access
#include <xstd/bits/detail/shift.hpp>      // shr
#include <boost/hash2/hash_append.hpp>     // hash_append, hash_append_size
#include <algorithm>                       // min
#include <array>                           // array
#include <concepts>                        // derived_from, same_as
#include <cstddef>                         // size_t
#include <cstdint>                         // uint64_t
#include <ranges>                          // contiguous_range, data, iota, range_value_t, size, sized_range
#include <span>                            // dynamic_extent, span
#include <type_traits>                     // is_class_v, remove_cvref_t
#include <utility>                         // declval

namespace xstd::bits::detail {

// Byte j of the bits, positions [8j, 8j + 8), read by shifts at every block width and byte order.
template<class Blocks>
[[nodiscard]] constexpr auto string_byte(Blocks const& blocks, std::size_t j) noexcept
        -> unsigned char
{
        return static_cast<unsigned char>(shr(blocks[j / bytes_per_block<Blocks>], bits_per_byte * (j % bytes_per_block<Blocks>)));
}

// The model's message, which the hash_append hooks write.

// Block k of the bit string, positions [64k, 64k + 64), zero past the last block: what the bools are expanded from.
template<class Blocks>
[[nodiscard]] constexpr auto string_block(Blocks const& blocks, std::size_t k) noexcept
        -> std::uint64_t
{
        if constexpr (block_digits<Blocks> % bools_per_block == 0UZ) {
                constexpr auto string_blocks_per_block = block_digits<Blocks> / bools_per_block;
                return static_cast<std::uint64_t>(shr(blocks[k / string_blocks_per_block], bools_per_block * (k % string_blocks_per_block)));
        } else {
                // Narrower blocks, or a width such as 24 dividing no 64-bit block: a byte at a time, value bits only.
                constexpr auto bytes_per_string_block = bools_per_block / bits_per_byte;
                auto const first                      = k * bytes_per_string_block;
                auto const last                       = std::ranges::min(first + bytes_per_string_block, std::ranges::size(blocks) * bytes_per_block<Blocks>);
                auto block                            = std::uint64_t{0};
                for (auto const j : std::views::iota(first, last)) {
                        block |= static_cast<std::uint64_t>(string_byte(blocks, j)) << (bits_per_byte * (j - first));
                }
                return block;
        }
}

// The n bools a std::vector<bool> of the same value holds, one byte of 0 or 1 each, eight 64-bit blocks an update.
template<class Hash, class Blocks>
constexpr auto update_bools(Hash& h, Blocks const& blocks, std::size_t n)
        -> void
{
        constexpr auto string_blocks_per_update = 8UZ;
        auto buffer                             = std::array<unsigned char, string_blocks_per_update * bools_per_block>();
        auto const string_blocks                = (n + bools_per_block - 1UZ) / bools_per_block;
        for (auto first = 0UZ; first < string_blocks; first += string_blocks_per_update) {
                auto const last = std::ranges::min(first + string_blocks_per_update, string_blocks);
                for (auto const k : std::views::iota(first, last)) {
                        expand_block(string_block(blocks, k), std::span(buffer).subspan((k - first) * bools_per_block).template first<bools_per_block>());
                }
                // Up to position n and no further: the zeros past it are no bool the model holds.
                h.update(buffer.data(), std::ranges::min(n, last * bools_per_block) - (first * bools_per_block));
        }
}

// std::array<bool, N> as Hash2 writes it, one '\x00' when empty, and std::vector<bool> or std::inplace_vector<bool, N>.
template<class Provider, class Hash, class Flavor, class Bits>
constexpr auto hash_append_bools(Provider const& pr, Hash& h, Flavor const& f, Bits const& c)
        -> void
{
        if constexpr (Bits::extent == 0UZ) {
                pr.hash_append(h, f, '\x00');
        } else {
                update_bools(h, c.blocks(), c.size());
                if constexpr (Bits::extent == std::dynamic_extent) {
                        pr.hash_append_size(h, f, c.size());
                }
        }
}

// std::set<Key, Compare> as Hash2 writes it: each key in the order the set iterates, then their count.
template<class Provider, class Hash, class Flavor, class Set>
constexpr auto hash_append_keys(Provider const& pr, Hash& h, Flavor const& f, Set const& s)
        -> void
{
        for (auto const key : s) {
                pr.hash_append(h, f, static_cast<Set::value_type>(key));
        }
        pr.hash_append_size(h, f, s.size());
}

// The canonical byte string, which bit_hash_append writes: bit i at bit i % 8 of byte i / 8, unused high bits zero.

// Blocks whose storage is the string: little-endian, every bit a value bit, and a scalar with the language's layout.
template<class Block>
inline constexpr bool hashes_storage_bytes = blocks_copy_as_bytes<std::span<Block const>> and not std::is_class_v<Block>;

// The most bytes assembled ahead of an update where the storage is not the string: up to it, a static width takes one.
inline constexpr auto assembly_bytes = 256UZ;

template<class Hash, class Blocks>
constexpr auto assemble_bit_bytes(Hash& h, Blocks const& blocks, std::size_t n)
        -> void
{
        auto buffer = std::array<unsigned char, assembly_bytes>();
        for (auto first = 0UZ; first < n; first += assembly_bytes) {
                auto const count = std::ranges::min(assembly_bytes, n - first);
                for (auto const j : std::views::iota(0UZ, count)) {
                        buffer[j] = string_byte(blocks, first + j);
                }
                h.update(buffer.data(), count);
        }
}

// The first n bytes of the string: one update over the storage where it is the string, assembled where it is not.
template<class Hash, class Blocks>
constexpr auto update_bit_bytes(Hash& h, Blocks const& blocks, std::size_t n)
        -> void
{
        if consteval {
                assemble_bit_bytes(h, blocks, n);
        } else {
                if constexpr (hashes_storage_bytes<std::ranges::range_value_t<Blocks>>) {
                        // No update of nothing: an empty storage need not point at an object.
                        if (n != 0UZ) {
                                h.update(std::ranges::data(blocks), n);
                        }
                } else {
                        assemble_bit_bytes(h, blocks, n);
                }
        }
}

// The storage under a reading, as the free functions reach it.
template<class T>
using hashed_bits_t = std::remove_cvref_t<decltype(storage_access::bits(std::declval<T const&>()))>;

template<class T>
concept set_reading = std::same_as<typename T::reads_as, set_reading_tag>;

// Every set, a view among them, and every sequence that owns its bits, as std::span hashes nothing.
template<class T>
concept bit_hashable = std::derived_from<T, typename T::adaptor_type> and (set_reading<T> or T::owns_storage);

template<class T>
concept static_bit_hashable = bit_hashable<T> and hashed_bits_t<T>::extent != std::dynamic_extent;

// The string's length: ceil(N / 8) bytes of a width, and of a run-time set those up to the byte of its last key.
template<bit_hashable T>
[[nodiscard]] constexpr auto bit_byte_count(T const& x) noexcept
        -> std::size_t
{
        auto const& c = storage_access::bits(x);
        if constexpr (set_reading<T> and hashed_bits_t<T>::extent == std::dynamic_extent) {
                // Equal sets need not share a width, so no byte past the last key is part of the value.
                auto const last = c.total_find_prev(c.size());
                if (last == c.size()) {
                        return 0UZ;
                }
                return (last / bits_per_byte) + 1UZ;
        } else {
                return (c.size() + bits_per_byte - 1UZ) / bits_per_byte;
        }
}

// The string, then at a run-time width its length: a set's in bytes, a sequence's in bits.
template<class Hash, class Flavor, bit_hashable T>
constexpr auto hash_append_bit_string(Hash& h, Flavor const& f, T const& x)
        -> void
{
        if constexpr (hashed_bits_t<T>::extent == 0UZ) {
                // Hash2 has every append update, an empty std::array with the one byte this writes.
                boost::hash2::hash_append(h, f, '\x00');
        } else {
                auto const n = bit_byte_count(x);
                update_bit_bytes(h, storage_access::bits(x).blocks(), n);
                if constexpr (hashed_bits_t<T>::extent == std::dynamic_extent) {
                        if constexpr (set_reading<T>) {
                                boost::hash2::hash_append_size(h, f, n);
                        } else {
                                boost::hash2::hash_append_size(h, f, storage_access::bits(x).size());
                        }
                }
        }
}

// The count a range's type fixes, as Hash2 asks of a contiguous range, else dynamic_extent.
template<class R>
inline constexpr std::size_t static_range_extent_v = std::dynamic_extent;

template<class T, std::size_t K>
inline constexpr std::size_t static_range_extent_v<T[K]> = K; // NOLINT(modernize-avoid-c-arrays): what a built-in array is

template<class T, std::size_t K>
inline constexpr std::size_t static_range_extent_v<std::array<T, K>> = K;

template<class T, std::size_t E>
inline constexpr std::size_t static_range_extent_v<std::span<T, E>> = E;

// One plane of static width, or a built-in or std::array of planes, nested to any depth; dense if no byte is between.
template<class T>
struct planes;

template<static_bit_hashable T>
struct planes<T>
{
        using plane_type = T;

        static constexpr bool dense = true;
};

template<class T, std::size_t K>
        requires requires { typename planes<T>::plane_type; }
struct planes<T[K]> // NOLINT(modernize-avoid-c-arrays): what a built-in array is
{
        using plane_type = planes<T>::plane_type;

        static constexpr bool dense = planes<T>::dense;
};

template<class T, std::size_t K>
        requires requires { typename planes<T>::plane_type; }
struct planes<std::array<T, K>>
{
        using plane_type = planes<T>::plane_type;

        // Measured, not assumed: std::array may carry more than its elements, and holds no element at K = 0.
        static constexpr bool dense = planes<T>::dense and K != 0UZ and sizeof(std::array<T, K>) == K * sizeof(T);
};

// A plane whose object is its string: an owner, with blocks that are the string, and no byte past ceil(N / 8).
template<class T>
concept plane_is_its_string =
        owner<T> and
        hashes_storage_bytes<typename hashed_bits_t<T>::block_type> and
        sizeof(T) == byte_count<hashed_bits_t<T>::extent>;

template<class T>
concept dense_planes = planes<T>::dense and plane_is_its_string<typename planes<T>::plane_type>;

template<class R>
concept bit_hashable_range =
        std::ranges::contiguous_range<R const> and
        std::ranges::sized_range<R const> and
        requires { typename planes<std::ranges::range_value_t<R>>::plane_type; };

template<class Hash, class Flavor, bit_hashable_range R>
constexpr auto hash_append_bit_planes(Hash& h, Flavor const& f, R const& r) -> void;

template<class Hash, class Flavor, class T>
constexpr auto hash_append_bit_element(Hash& h, Flavor const& f, T const& x)
        -> void
{
        if constexpr (bit_hashable<T>) {
                hash_append_bit_string(h, f, x);
        } else {
                hash_append_bit_planes(h, f, x);
        }
}

// The planes' strings back to back, as Hash2 writes a range, and its count where the type fixes none.
template<class Hash, class Flavor, bit_hashable_range R>
constexpr auto hash_append_bit_planes(Hash& h, Flavor const& f, R const& r)
        -> void
{
        using element_type    = std::ranges::range_value_t<R>;
        constexpr auto extent = static_range_extent_v<R>;
        if constexpr (extent == 0UZ) {
                boost::hash2::hash_append(h, f, '\x00');
        } else {
                auto const append_each = [&] -> void {
                        for (auto const& x : r) {
                                hash_append_bit_element(h, f, x);
                        }
                };
                if consteval {
                        append_each();
                } else {
                        if constexpr (dense_planes<element_type>) {
                                // The planes' objects are their strings back to back, and one update takes them all.
                                auto const size_in_bytes = std::ranges::size(r) * sizeof(element_type);
                                if constexpr (extent == std::dynamic_extent) {
                                        if (size_in_bytes != 0UZ) {
                                                h.update(std::ranges::data(r), size_in_bytes);
                                        }
                                } else {
                                        h.update(std::ranges::data(r), size_in_bytes);
                                }
                        } else {
                                append_each();
                        }
                }
                if constexpr (extent == std::dynamic_extent) {
                        boost::hash2::hash_append_size(h, f, std::ranges::size(r));
                }
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_HASH_HPP
