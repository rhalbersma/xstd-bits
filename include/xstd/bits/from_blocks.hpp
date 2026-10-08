//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_FROM_BLOCKS_HPP
#define XSTD_BITS_FROM_BLOCKS_HPP

// The tag that says an argument's blocks are read as bits, as std::from_range says a range's elements are read.
namespace xstd {

struct from_blocks_t
{
        explicit from_blocks_t() = default;
};

inline constexpr auto from_blocks = from_blocks_t();

} // namespace xstd

#endif // XSTD_BITS_FROM_BLOCKS_HPP
