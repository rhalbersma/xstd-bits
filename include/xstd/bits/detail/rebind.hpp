//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_REBIND_HPP
#define XSTD_BITS_DETAIL_REBIND_HPP

#include <cstddef> // size_t

namespace xstd::bits::detail {

// A fixed-width owner's block and width, and the same owner over another of each; each owner specializes it.
template<class Bits>
struct rebind
{};

template<class Bits>
concept rebindable = requires {
        typename rebind<Bits>::block_type;
        rebind<Bits>::width;
};

template<rebindable Bits>
using rebind_block_t = rebind<Bits>::block_type;

template<rebindable Bits>
constexpr std::size_t rebind_width_v = rebind<Bits>::width;

// Every argument but the block and the width passes through, so the owner's own constraints judge the new pair.
template<rebindable Bits, class Block, std::size_t N>
using rebind_t = rebind<Bits>::template type<Block, N>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_REBIND_HPP
