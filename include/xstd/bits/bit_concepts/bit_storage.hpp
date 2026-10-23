//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_STORAGE_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_STORAGE_HPP

#include <xstd/bits/detail/num_blocks.hpp>           // tiling_block
#include <xstd/bits/detail/resizable_bit_blocks.hpp> // resizable_bit_blocks
#include <ranges>                                    // range_value_t

// What a run-time width's owner takes as its whole state, as std::flat_set takes its KeyContainer.
namespace xstd {

// Contiguous blocks that tile any width, a regular value grown and shrunk by its own resize, all or nothing.
template<class Blocks>
concept bit_storage = bits::detail::resizable_bit_blocks<Blocks> and bits::detail::tiling_block<std::ranges::range_value_t<Blocks>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_STORAGE_HPP
