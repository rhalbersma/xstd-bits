//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BASIC_BITS_HPP
#define XSTD_BITS_BASIC_BITS_HPP

#include <xstd/bits/grid.hpp>                      // adaptor_t, bits_t
#include <xstd/bits/ownership.hpp>                 // owned_storage
#include <xstd/bits/tags.hpp>                      // array_container_tag, container_tag, reading_tag, sequence_reading_tag, set_reading_tag
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <boost/container_hash/is_range.hpp>       // is_range
#include <boost/container_hash/is_tuple_like.hpp>  // is_tuple_like
#include <cstddef>                                 // size_t
#include <functional>                              // hash
#include <memory>                                  // allocator
#include <span>                                    // dynamic_extent
#include <tuple>                                   // tuple_element, tuple_size
#include <type_traits>                             // false_type

namespace xstd {

// One owner per cell of the reading-by-container grid; the nine public names are the cells that have one.
template<reading_tag R, container_tag C, xstd::unsigned_integer Block, std::size_t N = std::dynamic_extent, class Alloc = std::allocator<Block>>
class basic_bits : public adaptor_t<R, bits_t<C, Block, N, Alloc>, basic_bits<R, C, Block, N, Alloc>>
{
        using base_type = adaptor_t<R, bits_t<C, Block, N, Alloc>, basic_bits<R, C, Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<reading_tag R, container_tag C, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct owned_storage<basic_bits<R, C, Block, N, Alloc>> : owned_storage<typename basic_bits<R, C, Block, N, Alloc>::adaptor_type>
{};

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::reading_tag R, xstd::container_tag C, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct hash<xstd::basic_bits<R, C, Block, N, Alloc>> : hash<typename xstd::basic_bits<R, C, Block, N, Alloc>::adaptor_type>
{};

template<std::size_t I, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct tuple_element<I, xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, Block, N, Alloc>> : tuple_element<I, typename xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, Block, N, Alloc>::adaptor_type>
{};

template<std::size_t I, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct tuple_element<I, const xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, Block, N, Alloc>> : tuple_element<I, const typename xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, Block, N, Alloc>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct tuple_size<xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, Block, N, Alloc>> : tuple_size<typename xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, Block, N, Alloc>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::container_tag C, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_range<xstd::basic_bits<xstd::sequence_reading_tag, C, Block, N, Alloc>> : std::false_type
{};

template<xstd::container_tag C, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_range<xstd::basic_bits<xstd::set_reading_tag, C, Block, N, Alloc>> : std::false_type
{};

template<xstd::container_tag C, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_tuple_like<xstd::basic_bits<xstd::sequence_reading_tag, C, Block, N, Alloc>> : std::false_type
{};

template<xstd::container_tag C, xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_tuple_like<xstd::basic_bits<xstd::set_reading_tag, C, Block, N, Alloc>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BASIC_BITS_HPP
