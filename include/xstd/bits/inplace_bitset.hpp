//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_INPLACE_BITSET_HPP
#define XSTD_BITS_INPLACE_BITSET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bit_storage.hpp>                     // bit_storage_extent_v
#include <xstd/bits/bitset_adaptor.hpp>                  // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/memory.hpp>                          // align_up
#include <cstddef>                                       // size_t
#include <inplace_vector>                                // inplace_vector
#include <limits>                                        // numeric_limits

namespace xstd {

// A resizable bitset that never allocates; no bit_ prefix, bitset already carrying the word.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_inplace_bitset = bitset_adaptor<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>;

template<std::size_t N>
using inplace_bitset = basic_inplace_bitset<std::size_t, N>;

// Spelled as the alias spells its block count, so that alias deduction reaches the aligned capacity.
template<xstd::unsigned_integer Block, std::size_t K>
bitset_adaptor(from_bit_storage_t, std::inplace_vector<Block, K>) -> bitset_adaptor<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, bit_storage_extent_v<Block> * K>>, bit_storage_extent_v<Block> * K>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_inplace_bitset = xstd::basic_inplace_bitset<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using inplace_bitset = basic_inplace_bitset<std::size_t, N>;

} // namespace aligned

} // namespace xstd

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_INPLACE_BITSET_HPP
