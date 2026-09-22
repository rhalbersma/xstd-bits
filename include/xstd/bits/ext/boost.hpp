//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_HPP
#define XSTD_BITS_EXT_BOOST_HPP

#include <xstd/bits/detail/basic_bits.hpp>               // basic_bits
#include <xstd/bits/detail/bitset_adaptor.hpp>           // IWYU pragma: keep; the adaptor bitset_reading_tag names
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/grid.hpp>                     // bits_of
#include <xstd/bits/detail/sequence_adaptor.hpp>         // IWYU pragma: keep; the adaptor sequence_reading_tag names
#include <xstd/bits/detail/set_adaptor.hpp>              // IWYU pragma: keep; the adaptor set_reading_tag names
#include <xstd/bits/detail/tags.hpp>                     // bitset_reading_tag, enable_container_tag, sequence_reading_tag, set_reading_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <cstddef>                                       // size_t

namespace xstd {

// The fourth vehicle: a run-time width that stays inline up to a static capacity and reaches the heap past it.
struct small_vector_container_tag
{};

template<>
inline constexpr bool enable_container_tag<small_vector_container_tag> = true;

namespace detail::bits {

template<xstd::unsigned_integer Block, std::size_t N, class Allocator>
using contiguous_bit_small_vector =
        contiguous_bit_container<boost::container::small_vector<Block, num_blocks_v<Block, N>, Allocator>>;

} // namespace detail::bits

// N is a capacity in bits here, as it is for the inplace column, and the width underneath it stays dynamic.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct bits_of<small_vector_container_tag, Block, N, Alloc>
{
        using type = detail::bits::contiguous_bit_small_vector<Block, N, Alloc>;
};

// The allocator is Boost's own, since the container this column is built on defaults to that one rather than std's.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
using basic_bit_small_set = basic_bits<set_reading_tag, small_vector_container_tag, Block, N, Alloc>;

template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
using basic_bit_small_vector = basic_bits<sequence_reading_tag, small_vector_container_tag, Block, N, Alloc>;

template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
using basic_small_bitset = basic_bits<bitset_reading_tag, small_vector_container_tag, Block, N, Alloc>;

template<std::size_t N>
using bit_small_set = basic_bit_small_set<std::size_t, N>;

template<std::size_t N>
using bit_small_vector = basic_bit_small_vector<std::size_t, N>;

template<std::size_t N>
using small_bitset = basic_small_bitset<std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_HPP
