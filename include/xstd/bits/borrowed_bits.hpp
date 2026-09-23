//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BORROWED_BITS_HPP
#define XSTD_BITS_BORROWED_BITS_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/contiguous_block_range.hpp>   // borrowed_block_span
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <cstddef>                                       // size_t
#include <memory>                                        // addressof
#include <ranges>                                        // contiguous_range, sized_range
#include <span>                                          // dynamic_extent, span
#include <utility>                                       // declval

// Bits in words someone else owns, for bit_set_view and bit_span to read and write in place.
namespace xstd {

// Every bit of the words is a position, bit n of word i being position i * digits + n; the width is the words'.
template<xstd::unsigned_integer Block, std::size_t Extent = std::dynamic_extent>
using borrowed_bits = detail::bits::contiguous_bit_container<std::span<Block, Extent>>;

// One word, as its own digits: bit n is 2^n, so a view over it reads the integer's set bits.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto borrow_bits(Block& word) noexcept
        -> borrowed_bits<Block, 1>
{
        return borrowed_bits<Block, 1>(std::span<Block, 1>(std::addressof(word), 1UZ));
}

// A contiguous range of words at the extent its type carries: an array's is static, a vector's is not.
template<std::ranges::contiguous_range R>
        requires std::ranges::sized_range<R> and detail::bits::borrowed_block_span<decltype(std::span(std::declval<R&>()))>
[[nodiscard]] constexpr auto borrow_bits(R& words) noexcept
        -> detail::bits::contiguous_bit_container<decltype(std::span(words))>
{
        return detail::bits::contiguous_bit_container<decltype(std::span(words))>(std::span(words));
}

} // namespace xstd

#endif // XSTD_BITS_BORROWED_BITS_HPP
