//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STATIC_SET_HPP
#define XSTD_BITS_BIT_STATIC_SET_HPP

#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/ownership.hpp>                   // ownership
#include <xstd/bits/set_adaptor.hpp>                 // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <xstd/ints/memory.hpp>                      // align_up
#include <cstddef>                                   // size_t
#include <limits>                                    // digits

namespace xstd {

// The static set: the basic name leaves the block open, the restricted one is the machine word.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_static_set : public set_adaptor<detail::bits::contiguous_bit_array<Block, N>, ownership::owns, basic_bit_static_set<Block, N>>
{
        using base_type = set_adaptor<detail::bits::contiguous_bit_array<Block, N>, ownership::owns, basic_bit_static_set<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

template<std::size_t N>
using bit_static_set = basic_bit_static_set<std::size_t, N>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_static_set = xstd::basic_bit_static_set<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<std::size_t, N>;

} // namespace aligned

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<xstd::unsigned_integer Block, std::size_t N>
struct owned_storage<basic_bit_static_set<Block, N>> : owned_storage<typename basic_bit_static_set<Block, N>::adaptor_type>
{};

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_bit_static_set<Block, N>> : hash<typename xstd::basic_bit_static_set<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

template<xstd::unsigned_integer Block, std::size_t N>
struct is_range<xstd::basic_bit_static_set<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_STATIC_SET_HPP
