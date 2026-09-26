//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DYNAMIC_BITSET_HPP
#define XSTD_BITS_DYNAMIC_BITSET_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <concepts>                                      // constructible_from
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <iterator>                                      // input_iterator
#include <memory>                                        // allocator, allocator_traits
#include <string>                                        // basic_string
#include <string_view>                                   // basic_string_view
#include <type_traits>                                   // type_identity_t
#include <utility>                                       // move
#include <vector>                                        // vector

namespace xstd {

// The bitset reading over a heap of blocks, boost::dynamic_bitset being its counterpart.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_dynamic_bitset : public bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<std::vector<Block, Allocator>>, basic_dynamic_bitset<Block, Allocator>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<std::vector<Block, Allocator>>, basic_dynamic_bitset<Block, Allocator>>;

public:
        using typename base_type::size_type;

        // boost::dynamic_bitset's constructors in its documented order.
        [[nodiscard]] constexpr basic_dynamic_bitset() noexcept(noexcept(Allocator()))
                : basic_dynamic_bitset(Allocator())
        {}

        [[nodiscard]] constexpr explicit basic_dynamic_bitset(Allocator const& alloc) noexcept
                : base_type(alloc)
        {}

        [[nodiscard]] constexpr explicit basic_dynamic_bitset(size_type num_bits, unsigned long long value = 0ULL, Allocator const& alloc = Allocator())
                : base_type(num_bits, value, alloc)
        {}

        template<class charT, class traits, class StringAllocator>
        [[nodiscard]] constexpr explicit basic_dynamic_bitset(
                std::basic_string<charT, traits, StringAllocator> const& str,
                std::basic_string<charT, traits, StringAllocator>::size_type pos = 0,
                std::basic_string<charT, traits, StringAllocator>::size_type n = std::basic_string<charT, traits, StringAllocator>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit basic_dynamic_bitset(
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
        [[nodiscard]] constexpr explicit basic_dynamic_bitset(
                charT const* str,
                std::size_t n = std::basic_string_view<charT>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, n, zero, one)
        {}

        template<std::input_iterator BlockInputIterator>
                requires std::constructible_from<base_type, BlockInputIterator, BlockInputIterator, Allocator const&>
        [[nodiscard]] constexpr basic_dynamic_bitset(BlockInputIterator first, BlockInputIterator last, Allocator const& alloc = Allocator())
                : base_type(first, last, alloc)
        {}

        [[nodiscard]] basic_dynamic_bitset(basic_dynamic_bitset const& b) = default;
        [[nodiscard]] basic_dynamic_bitset(basic_dynamic_bitset&& b) noexcept = default;

        ~basic_dynamic_bitset() = default;

        auto operator=(basic_dynamic_bitset const& b) -> basic_dynamic_bitset& = default;
        auto operator=(basic_dynamic_bitset&& b) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value or std::allocator_traits<Allocator>::is_always_equal::value) -> basic_dynamic_bitset& = default;

        // Not in boost::dynamic_bitset: [container.alloc.reqmts]'s allocator-extended copy and move.
        [[nodiscard]] constexpr basic_dynamic_bitset(basic_dynamic_bitset const& b, std::type_identity_t<Allocator> const& alloc)
                : base_type(b, alloc)
        {}

        [[nodiscard]] constexpr basic_dynamic_bitset(basic_dynamic_bitset&& b, std::type_identity_t<Allocator> const& alloc)
                : base_type(std::move(b), alloc)
        {}

        // Not in boost::dynamic_bitset: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_dynamic_bitset(from_bit_storage_t, std::vector<Block, Allocator> blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_dynamic_bitset(from_bit_storage_t, std::vector<Block, Allocator> blocks, Allocator const& alloc)
                : base_type(from_bit_storage, std::move(blocks), alloc)
        {}

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_dynamic_bitset& x, basic_dynamic_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using dynamic_bitset = basic_dynamic_bitset<std::size_t>;

// The blocks adopted name the block and the allocator both.
template<xstd::unsigned_integer Block, class Allocator>
basic_dynamic_bitset(from_bit_storage_t, std::vector<Block, Allocator>) -> basic_dynamic_bitset<Block, Allocator>;

template<xstd::unsigned_integer Block, class Allocator>
basic_dynamic_bitset(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> basic_dynamic_bitset<Block, Allocator>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, class Allocator>
struct hash<xstd::basic_dynamic_bitset<Block, Allocator>> : hash<typename xstd::basic_dynamic_bitset<Block, Allocator>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_DYNAMIC_BITSET_HPP
