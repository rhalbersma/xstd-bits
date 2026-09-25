//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_VIEWS_HPP
#define XSTD_BITS_DETAIL_VIEWS_HPP

#include <xstd/bits/bit_storage.hpp>             // bit_storage
#include <xstd/bits/detail/sequence_adaptor.hpp> // blit_source, window_of
#include <cstddef>                               // size_t

// What the sequence adaptor asks of the two public views: the window each hands back, and whether each blits.
namespace xstd {

// Declared without default arguments: those are given once, where each view is defined.
template<bit_storage Blocks, std::size_t N>
class bit_span;

template<bit_storage Blocks, std::size_t Extent, std::size_t N>
class bit_subspan;

} // namespace xstd

namespace xstd::bits::detail {

// A window of the whole sequence is named by the same words and width.
template<class Blocks, std::size_t N, class Bits, std::size_t E>
struct window_of<bit_span<Blocks, N>, Bits, E>
{
        using type = bit_subspan<Blocks, E, N>;
};

// A window of a window is named as the window it came from, as std::span's subspan stays a span.
template<class Blocks, std::size_t Extent, std::size_t N, class Bits, std::size_t E>
struct window_of<bit_subspan<Blocks, Extent, N>, Bits, E>
{
        using type = bit_subspan<Blocks, E, N>;
};

// A view answers every trait as the vehicle it is built on, which is where each one is defined.
template<class Blocks, std::size_t N, class Block>
inline constexpr bool blit_source<bit_span<Blocks, N>, Block> = blit_source<typename bit_span<Blocks, N>::adaptor_type, Block>; // NOLINT(readability-redundant-typename)

template<class Blocks, std::size_t Extent, std::size_t N, class Block>
inline constexpr bool blit_source<bit_subspan<Blocks, Extent, N>, Block> = blit_source<typename bit_subspan<Blocks, Extent, N>::adaptor_type, Block>; // NOLINT(readability-redundant-typename)

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_VIEWS_HPP
