//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_REBIND_HPP
#define XSTD_BITS_DETAIL_REBIND_HPP

#include <cstddef> // size_t

namespace xstd::bits::detail {

// A fixed-width owner's block and width, and the same owner over another of each; each owner specializes it.
template<class Owner>
struct rebind
{};

template<class Owner>
concept rebindable = requires {
        typename rebind<Owner>::block_type;
        rebind<Owner>::width;
};

template<rebindable Owner>
using rebind_block_t = rebind<Owner>::block_type;

template<rebindable Owner>
constexpr std::size_t rebind_width_v = rebind<Owner>::width;

// Every argument but the block and the width passes through, so the owner's own constraints judge the new pair.
template<rebindable Owner, class Block, std::size_t N>
using rebind_t = rebind<Owner>::template type<Block, N>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_REBIND_HPP
