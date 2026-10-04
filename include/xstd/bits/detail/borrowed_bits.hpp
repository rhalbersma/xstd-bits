//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BORROWED_BITS_HPP
#define XSTD_BITS_DETAIL_BORROWED_BITS_HPP

#include <xstd/bits/bit_blocks.hpp>                 // bit_block, bit_block_range
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/ints/concepts/unsigned_integer.hpp>  // unsigned_integer
#include <cstddef>                                  // size_t
#include <memory>                                   // addressof
#include <ranges>                                   // borrowed_range
#include <span>                                     // dynamic_extent, span
#include <type_traits>                              // conditional_t, is_const_v, is_lvalue_reference_v, remove_const_t, remove_reference_t
#include <utility>                                  // declval

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
concept borrowable_block = std::is_lvalue_reference_v<W> and xstd::bit_block<std::remove_reference_t<W>>;

template<class W>
concept borrowable_blocks = xstd::bit_block_range<W> and (std::is_lvalue_reference_v<W> or std::ranges::borrowed_range<W>);

template<class W>
struct borrowed_bits_for;

// A const block or const blocks make const storage, which is how a view over them is read-only.
template<borrowable_block W>
struct borrowed_bits_for<W>
{
        using block_type = std::remove_reference_t<W>;
        using bits_type  = borrowed_bits<std::remove_const_t<block_type>, 1>;
        using type       = std::conditional_t<std::is_const_v<block_type>, bits_type const, bits_type>;
};

template<borrowable_blocks W>
struct borrowed_bits_for<W>
{
        using span_type = block_span_t<std::remove_reference_t<W>>;
        using bits_type = borrowed_bits<std::remove_const_t<typename span_type::element_type>, span_type::extent>;
        using type      = std::conditional_t<std::is_const_v<typename span_type::element_type>, bits_type const, bits_type>;
};

// The storage a view deduces from the blocks it is handed, const where they are.
template<class W>
using borrowed_bits_t = borrowed_bits_for<W>::type;

// The blocks as storage; const blocks are lent writable, and the const storage deduced for them keeps them unwritten.
template<class W>
        requires borrowable_block<W&&> or borrowable_blocks<W&&>
[[nodiscard]] constexpr auto borrow_bits(W&& blocks) noexcept
        -> std::remove_const_t<borrowed_bits_t<W&&>>
{
        using bits_type  = std::remove_const_t<borrowed_bits_t<W&&>>;
        using span_type  = bits_type::block_container_type;
        using block_type = bits_type::block_type;
        if constexpr (xstd::bit_block<std::remove_reference_t<W>>) {
                return bits_type(span_type(const_cast<block_type*>(std::addressof(blocks)), 1UZ));
        } else {
                auto const s = std::span(blocks);
                return bits_type(span_type(const_cast<block_type*>(s.data()), s.size()));
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BORROWED_BITS_HPP
