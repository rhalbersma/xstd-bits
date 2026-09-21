//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_SET_HPP
#define XSTD_BITS_BIT_INPLACE_SET_HPP

#include <boost/container_hash/is_range.hpp> // is_range
#include <functional>                        // hash
#include <type_traits>                       // false_type
#include <version>                           // IWYU pragma: keep; __cpp_lib_inplace_vector

// The column comes and goes with its storage, and an alias withholds a name rather than a capability.
#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // contiguous_bit_inplace_vector
#include <xstd/bits/ownership.hpp>                            // ownership
#include <xstd/bits/set_adaptor.hpp>                          // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <cstddef>                                            // size_t

namespace xstd {

// The set reading over a run-time width under a compile-time capacity: inplace names where the storage lives.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_inplace_set : public set_adaptor<detail::bits::contiguous_bit_inplace_vector<Block, N>, ownership::owns, basic_bit_inplace_set<Block, N>>
{
        using base_type = set_adaptor<detail::bits::contiguous_bit_inplace_vector<Block, N>, ownership::owns, basic_bit_inplace_set<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, N>;

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<xstd::unsigned_integer Block, std::size_t N>
struct owned_storage<basic_bit_inplace_set<Block, N>> : owned_storage<typename basic_bit_inplace_set<Block, N>::adaptor_type>
{};

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_bit_inplace_set<Block, N>> : hash<typename xstd::basic_bit_inplace_set<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

template<xstd::unsigned_integer Block, std::size_t N>
struct is_range<xstd::basic_bit_inplace_set<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BIT_INPLACE_SET_HPP
