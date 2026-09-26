//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BOUNDED_BITSET_HPP
#define XSTD_BITS_BOUNDED_BITSET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bit_storage.hpp>                     // bit_storage_extent_v
#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/memory.hpp>                          // align_up
#include <concepts>                                      // constructible_from
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <inplace_vector>                                // inplace_vector
#include <iterator>                                      // input_iterator
#include <limits>                                        // numeric_limits
#include <string>                                        // basic_string
#include <string_view>                                   // basic_string_view
#include <utility>                                       // move

namespace xstd {

// A resizable bitset that never allocates; no bit_ prefix, bitset already carrying the word.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bounded_bitset : public bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>, basic_bounded_bitset<Block, N>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>, basic_bounded_bitset<Block, N>>;

public:
        using typename base_type::block_container_type;
        using typename base_type::size_type;

        // boost::dynamic_bitset's constructors in its documented order, less its allocator forms.
        [[nodiscard]] basic_bounded_bitset() noexcept = default;

        [[nodiscard]] constexpr explicit basic_bounded_bitset(size_type num_bits, unsigned long long value = 0ULL)
                : base_type(num_bits, value)
        {}

        template<class charT, class traits, class Allocator>
        [[nodiscard]] constexpr explicit basic_bounded_bitset(
                std::basic_string<charT, traits, Allocator> const& str,
                std::basic_string<charT, traits, Allocator>::size_type pos = 0,
                std::basic_string<charT, traits, Allocator>::size_type n = std::basic_string<charT, traits, Allocator>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit basic_bounded_bitset(
                std::basic_string_view<charT, traits> str,
                std::basic_string_view<charT, traits>::size_type pos = 0,
                std::basic_string_view<charT, traits>::size_type n = std::basic_string_view<charT, traits>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, pos, n, zero, one)
        {}

        template<class charT>
                requires std::constructible_from<base_type, charT const*>
        [[nodiscard]] constexpr explicit basic_bounded_bitset(
                charT const* str,
                std::size_t n = std::basic_string_view<charT>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, n, zero, one)
        {}

        template<std::input_iterator BlockInputIterator>
                requires std::constructible_from<base_type, BlockInputIterator, BlockInputIterator>
        [[nodiscard]] constexpr basic_bounded_bitset(BlockInputIterator first, BlockInputIterator last)
                : base_type(first, last)
        {}

        [[nodiscard]] basic_bounded_bitset(basic_bounded_bitset const& b) = default;
        [[nodiscard]] basic_bounded_bitset(basic_bounded_bitset&& b) noexcept = default;

        ~basic_bounded_bitset() = default;

        auto operator=(basic_bounded_bitset const& b) -> basic_bounded_bitset& = default;
        auto operator=(basic_bounded_bitset&& b) noexcept -> basic_bounded_bitset& = default;

        // Not in boost::dynamic_bitset: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_bounded_bitset(from_bit_storage_t, block_container_type blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bounded_bitset& x, basic_bounded_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bounded_bitset = basic_bounded_bitset<std::size_t, N>;

// Every bit of the blocks a position, so the capacity is theirs, rounded to whole blocks.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bounded_bitset(from_bit_storage_t, std::inplace_vector<Block, K>) -> basic_bounded_bitset<Block, bit_storage_extent_v<Block> * K>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bounded_bitset = xstd::basic_bounded_bitset<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bounded_bitset = basic_bounded_bitset<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N>
struct hash<xstd::basic_bounded_bitset<Block, N>> : hash<typename xstd::basic_bounded_bitset<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BOUNDED_BITSET_HPP
