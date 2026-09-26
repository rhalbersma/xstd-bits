//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_ARRAY_STORAGE_HPP
#define TEST_ARRAY_STORAGE_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <array>                                         // array
#include <cstddef>                                       // size_t

namespace test {

// The storage of a width fixed at compile time, taking <class Block, size_t N> as graded_extents instantiates it.
template<class Block, std::size_t N>
using array_storage = xstd::bits::detail::contiguous_bit_container<std::array<Block, xstd::bits::detail::num_blocks_v<Block, N>>, N>;

} // namespace test

#endif // TEST_ARRAY_STORAGE_HPP
