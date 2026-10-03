//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DYNAMIC_BITSET_HPP
#define XSTD_BITS_DYNAMIC_BITSET_HPP

#include <xstd/bits/detail/bit_container.hpp>      // bit_container
#include <xstd/bits/detail/bitset_adaptor.hpp>     // bitset_adaptor
#include <xstd/bits/from_bit_storage.hpp>          // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/misc/concepts/simple_allocator.hpp> // simple_allocator
#include <concepts>                                // constructible_from
#include <cstddef>                                 // size_t
#include <functional>                              // hash
#include <iterator>                                // input_iterator
#include <memory>                                  // allocator
#include <string>                                  // basic_string
#include <string_view>                             // basic_string_view
#include <type_traits>                             // type_identity_t
#include <utility>                                 // move
#include <vector>                                  // vector

namespace xstd {

// The bitset reading over a heap of blocks, boost::dynamic_bitset being its counterpart.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_dynamic_bitset : public bits::detail::bitset_adaptor<bits::detail::bit_container<std::vector<Block, Allocator>>, basic_dynamic_bitset<Block, Allocator>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::bit_container<std::vector<Block, Allocator>>, basic_dynamic_bitset<Block, Allocator>>;

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

// boost::dynamic_bitset's defaults as guides: Block from the allocator, std::size_t by default.
basic_dynamic_bitset() -> basic_dynamic_bitset<std::size_t>;

template<class Allocator>
        requires xstd::simple_allocator<Allocator>
explicit basic_dynamic_bitset(Allocator) -> basic_dynamic_bitset<typename Allocator::value_type, Allocator>;

explicit basic_dynamic_bitset(std::size_t, unsigned long long = 0ULL) -> basic_dynamic_bitset<std::size_t>;

template<class Allocator>
        requires xstd::simple_allocator<Allocator>
explicit basic_dynamic_bitset(std::size_t, unsigned long long, Allocator) -> basic_dynamic_bitset<typename Allocator::value_type, Allocator>;

// The digits are not the block's argument, so zero and one take part in no deduction.
template<class charT, class traits, class StringAllocator>
explicit basic_dynamic_bitset(std::basic_string<charT, traits, StringAllocator>, std::size_t = 0, std::size_t = std::basic_string<charT, traits, StringAllocator>::npos, std::type_identity_t<charT> = static_cast<charT>('0'), std::type_identity_t<charT> = static_cast<charT>('1')) -> basic_dynamic_bitset<std::size_t>;

template<class charT, class traits>
explicit basic_dynamic_bitset(std::basic_string_view<charT, traits>, std::size_t = 0, std::size_t = std::basic_string_view<charT, traits>::npos, std::type_identity_t<charT> = static_cast<charT>('0'), std::type_identity_t<charT> = static_cast<charT>('1')) -> basic_dynamic_bitset<std::size_t>;

template<class charT>
explicit basic_dynamic_bitset(charT const*, std::size_t = std::basic_string_view<charT>::npos, std::type_identity_t<charT> = static_cast<charT>('0'), std::type_identity_t<charT> = static_cast<charT>('1')) -> basic_dynamic_bitset<std::size_t>;

// The values are converted to the block, so they name none: the allocator does, or std::size_t by default.
template<std::input_iterator BlockInputIterator, class Allocator = std::allocator<std::size_t>>
        requires xstd::simple_allocator<Allocator>
basic_dynamic_bitset(BlockInputIterator, BlockInputIterator, Allocator = Allocator()) -> basic_dynamic_bitset<typename Allocator::value_type, Allocator>;

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
