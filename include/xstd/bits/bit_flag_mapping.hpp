//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_MAPPING_HPP
#define XSTD_BITS_BIT_FLAG_MAPPING_HPP

#include <xstd/bits/detail/flag_word.hpp> // flag_mask, flag_width_v, flag_word_t, from_word, to_word
#include <xstd/bits/detail/shift.hpp>     // shl
#include <bit>                            // countr_zero, has_single_bit
#include <cassert>                        // assert
#include <cstddef>                        // size_t

// A bitmask type keyed on its own one-bit values, so a set of its flags needs no enumeration of ranks.
namespace xstd {

// A one-bit value ranks at the position of its bit, and rank i is the value 1 << i: an enumeration or a std::bitset.
template<class Key, std::size_t N = bits::detail::flag_width_v<Key>>
        requires bits::detail::flag_mask<Key>
struct bit_flag_mapping
{
private:
        using word_type = bits::detail::flag_word_t<Key>;

        static_assert(N <= bits::detail::flag_width_v<Key>);

public:
        static constexpr std::size_t size = N;

        // Exactly one bit set, below N: zero and a value of several bits name no position, and are no key.
        [[nodiscard]] static constexpr auto is_key(Key const& key) noexcept
                -> bool
        {
                auto const word = bits::detail::to_word(key);
                return std::has_single_bit(word) and static_cast<std::size_t>(std::countr_zero(word)) < N;
        }

        // The key has exactly one bit set; one at or above N ranks at size or above.
        [[nodiscard]] static constexpr auto to_index(Key const& key) noexcept
                -> std::size_t
        {
                auto const word = bits::detail::to_word(key);
                assert(std::has_single_bit(word));
                return static_cast<std::size_t>(std::countr_zero(word));
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                assert(index < N);
                return bits::detail::from_word<Key>(bits::detail::shl(word_type{1}, index));
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FLAG_MAPPING_HPP
