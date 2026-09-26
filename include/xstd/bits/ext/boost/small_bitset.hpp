//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_SMALL_BITSET_HPP
#define XSTD_BITS_EXT_BOOST_SMALL_BITSET_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <concepts>                                      // constructible_from
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <iterator>                                      // input_iterator
#include <memory>                                        // allocator_traits
#include <string>                                        // basic_string
#include <string_view>                                   // basic_string_view
#include <type_traits>                                   // is_nothrow_move_constructible_v, type_identity_t
#include <utility>                                       // move

namespace xstd {

// The bitset reading over the small-vector column; the allocator is Boost's own, as that container defaults to it.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_small_bitset : public bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>, basic_small_bitset<Block, N, Alloc>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>, basic_small_bitset<Block, N, Alloc>>;

public:
        using typename base_type::allocator_type;
        using typename base_type::block_container_type;
        using typename base_type::size_type;

        // boost::dynamic_bitset's constructors in its documented order, over the small vector's own allocator_type.
        [[nodiscard]] constexpr basic_small_bitset() noexcept(noexcept(allocator_type()))
                : basic_small_bitset(allocator_type())
        {}

        [[nodiscard]] constexpr explicit basic_small_bitset(allocator_type const& alloc)
                : base_type(alloc)
        {}

        [[nodiscard]] constexpr explicit basic_small_bitset(size_type num_bits, unsigned long long value = 0ULL, allocator_type const& alloc = allocator_type())
                : base_type(num_bits, value, alloc)
        {}

        template<class charT, class traits, class StringAllocator>
        [[nodiscard]] constexpr explicit basic_small_bitset(
                std::basic_string<charT, traits, StringAllocator> const& str,
                std::basic_string<charT, traits, StringAllocator>::size_type pos = 0,
                std::basic_string<charT, traits, StringAllocator>::size_type n = std::basic_string<charT, traits, StringAllocator>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit basic_small_bitset(
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
        [[nodiscard]] constexpr explicit basic_small_bitset(
                charT const* str,
                std::basic_string_view<charT>::size_type n = std::basic_string_view<charT>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, n, zero, one)
        {}

        template<std::input_iterator BlockInputIterator>
                requires std::constructible_from<base_type, BlockInputIterator, BlockInputIterator, allocator_type const&>
        [[nodiscard]] constexpr basic_small_bitset(BlockInputIterator first, BlockInputIterator last, allocator_type const& alloc = allocator_type())
                : base_type(first, last, alloc)
        {}

        [[nodiscard]] basic_small_bitset(basic_small_bitset const& b) = default;
        [[nodiscard]] basic_small_bitset(basic_small_bitset&& b) = default;

        ~basic_small_bitset() = default;

        auto operator=(basic_small_bitset const& b) -> basic_small_bitset& = default;
        auto operator=(basic_small_bitset&& b) noexcept(std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value or std::allocator_traits<allocator_type>::is_always_equal::value) -> basic_small_bitset& = default;

        // Not in boost::dynamic_bitset: [container.alloc.reqmts]'s allocator-extended copy and move.
        [[nodiscard]] constexpr basic_small_bitset(basic_small_bitset const& b, std::type_identity_t<allocator_type> const& alloc)
                : base_type(b, alloc)
        {}

        [[nodiscard]] constexpr basic_small_bitset(basic_small_bitset&& b, std::type_identity_t<allocator_type> const& alloc)
                : base_type(std::move(b), alloc)
        {}

        // Not in boost::dynamic_bitset: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_small_bitset(from_bit_storage_t, block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<block_container_type>)
                : base_type(from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_small_bitset(from_bit_storage_t, block_container_type blocks, allocator_type const& alloc)
                : base_type(from_bit_storage, std::move(blocks), alloc)
        {}

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_small_bitset& x, basic_small_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using small_bitset = basic_small_bitset<std::size_t, N>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N, class Alloc>
struct hash<xstd::basic_small_bitset<Block, N, Alloc>> : hash<typename xstd::basic_small_bitset<Block, N, Alloc>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_EXT_BOOST_SMALL_BITSET_HPP
