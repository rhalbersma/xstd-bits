//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_MAPPING_HPP
#define XSTD_BITS_BIT_FLAG_MAPPING_HPP

#include <xstd/bits/detail/flag_block.hpp> // default_flag_width_v, flag_block_t, flag_mask, flag_width_v, from_block, to_block, word_bit, word_count, word_countr_zero
#include <cassert>                         // assert
#include <cstddef>                         // size_t

// A bitmask type keyed on its own one-bit values, so a set of its flags needs no enumeration of ranks.
namespace xstd {

// A one-bit value ranks at the position of its bit, and rank i is the value 1 << i: an enumeration, integer or bitset.
template<class Key, std::size_t N = bits::detail::default_flag_width_v<Key>>
        requires bits::detail::flag_mask<Key>
struct bit_flag_mapping
{
private:
        static_assert(N <= bits::detail::flag_width_v<Key>);

public:
        // The unsigned word a mask is read and written as, a block or an array of them, each bit at its own position.
        using block_type = bits::detail::flag_block_t<Key>;

        static constexpr std::size_t size = N;

        // Exactly one bit set, below N: zero and a value of several bits name no position, and are no key.
        [[nodiscard]] static constexpr auto is_key(Key const& key) noexcept
                -> bool
        {
                auto const block = bits::detail::to_block(key);
                return bits::detail::word_count(block) == 1UZ and bits::detail::word_countr_zero(block) < N;
        }

        // The key has exactly one bit set; one at or above N ranks at size or above.
        [[nodiscard]] static constexpr auto to_index(Key const& key) noexcept
                -> std::size_t
        {
                auto const block = bits::detail::to_block(key);
                assert(bits::detail::word_count(block) == 1UZ);
                return bits::detail::word_countr_zero(block);
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                assert(index < N);
                return bits::detail::from_block<Key>(bits::detail::word_bit<block_type>(index));
        }

        // Any value of the mask, one-bit or not, as the block holding its bits; from_block reads it back.
        [[nodiscard]] static constexpr auto to_block(Key const& mask) noexcept
                -> block_type
        {
                return bits::detail::to_block(mask);
        }

        [[nodiscard]] static constexpr auto from_block(block_type const& block) noexcept
                -> Key
        {
                return bits::detail::from_block<Key>(block);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FLAG_MAPPING_HPP
