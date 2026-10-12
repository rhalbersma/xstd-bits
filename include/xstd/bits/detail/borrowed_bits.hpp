//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BORROWED_BITS_HPP
#define XSTD_BITS_DETAIL_BORROWED_BITS_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container
#include <xstd/bits/detail/bit_block_range.hpp>            // bit_block_range
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <cstddef>                                         // size_t
#include <memory>                                          // addressof
#include <ranges>                                          // borrowed_range
#include <span>                                            // dynamic_extent, span
#include <type_traits>                                     // conditional_t, is_const_v, is_lvalue_reference_v, remove_const_t, remove_reference_t
#include <utility>                                         // declval

// Bits in blocks someone else owns, which bit_set_view and bit_span hold by value and read and write in place.
namespace xstd::bits::detail {

// Every bit of the blocks is a position, bit n of block i being position i * digits + n; the width is the blocks'.
template<xstd::unsigned_integer Block, std::size_t Extent = std::dynamic_extent>
using borrowed_bits = bit_block_container<std::span<Block, Extent>>;

// The span over a range of blocks, at the extent its type carries: an array's is static, a vector's is not.
template<class R>
using block_span_t = decltype(std::span(std::declval<R&>()));

// One block lent by lvalue, or a range of blocks lent by lvalue or, as an rvalue, borrowed.
template<class W>
concept borrowable_block = std::is_lvalue_reference_v<W> and xstd::unsigned_integer<std::remove_reference_t<W>>;

template<class W>
concept borrowable_blocks = bit_block_range<W> and (std::is_lvalue_reference_v<W> or std::ranges::borrowed_range<W>);

template<class T, class Bits>
using const_as_t = std::conditional_t<std::is_const_v<T>, Bits const, Bits>;

// A view's storage: someone else's blocks by span, or the storage of an owner over the same blocks it refers into.
template<class Blocks, std::size_t N>
struct view_storage_for
{
        using type = const_as_t<Blocks, bit_block_container<std::remove_const_t<Blocks>, N>>;
};

template<xstd::unsigned_integer Block, std::size_t N>
struct view_storage_for<Block, N>
{
        using type = const_as_t<Block, borrowed_bits<std::remove_const_t<Block>, 1>>;
};

template<class Block, std::size_t E, std::size_t N>
struct view_storage_for<std::span<Block, E>, N>
{
        using type = const_as_t<Block, borrowed_bits<std::remove_const_t<Block>, E>>;
};

template<class Blocks, std::size_t N>
using view_storage_t = view_storage_for<Blocks, N>::type;

// The blocks a guide deduces from what it is handed: a block as itself, a range as the span that lends it.
template<class W>
struct lent_blocks;

template<borrowable_block W>
struct lent_blocks<W>
{
        using type = std::remove_reference_t<W>;
};

template<borrowable_blocks W>
struct lent_blocks<W>
{
        using type = block_span_t<std::remove_reference_t<W>>;
};

template<class W>
using lent_blocks_t = lent_blocks<W>::type;

// The storage a view deduces from the blocks it is handed, const where they are: the view's storage over them.
template<class W>
using borrowed_bits_t = view_storage_t<lent_blocks_t<W>, xstd::bit_blocks_extent_v<lent_blocks_t<W>>>;

// The blocks as storage; const blocks are lent writable, and the const storage deduced for them keeps them unwritten.
template<class W>
        requires borrowable_block<W&&> or borrowable_blocks<W&&>
[[nodiscard]] constexpr auto borrow_bits(W&& blocks) noexcept
        -> std::remove_const_t<borrowed_bits_t<W&&>>
{
        using bits_type  = std::remove_const_t<borrowed_bits_t<W&&>>;
        using span_type  = bits_type::block_container_type;
        using block_type = bits_type::block_type;
        if constexpr (xstd::unsigned_integer<std::remove_reference_t<W>>) {
                return bits_type(span_type(const_cast<block_type*>(std::addressof(blocks)), 1UZ));
        } else {
                auto const s = std::span(blocks);
                return bits_type(span_type(const_cast<block_type*>(s.data()), s.size()));
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BORROWED_BITS_HPP
