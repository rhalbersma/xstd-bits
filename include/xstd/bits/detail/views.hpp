//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_VIEWS_HPP
#define XSTD_BITS_DETAIL_VIEWS_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp> // bit_blocks
#include <xstd/bits/detail/sequence_adaptor.hpp> // window_of
#include <cstddef>                               // size_t

// What the sequence adaptor asks of the two public views: the window each hands back.
namespace xstd {

// Declared without default arguments: those are given once, where each view is defined.
template<bit_blocks Blocks, std::size_t N>
class bit_span;

template<bit_blocks Blocks, std::size_t Extent, std::size_t N>
class bit_subspan;

} // namespace xstd

namespace xstd::bits::detail {

// A window of the whole sequence is named by the same blocks and width.
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

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_VIEWS_HPP
