//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_INPLACE_BITSET_HPP
#define XSTD_BITS_INPLACE_BITSET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

// The column comes and goes with its storage, and an alias withholds a name rather than a capability.
#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bitset_adaptor.hpp>                       // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // contiguous_bit_inplace_vector
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <cstddef>                                            // size_t
#include <functional>                                         // hash

namespace xstd {

// A resizable bitset that never allocates; no bit_ prefix, bitset already carrying the word.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_inplace_bitset : public bitset_adaptor<detail::bits::contiguous_bit_inplace_vector<Block, N>, basic_inplace_bitset<Block, N>>
{
        using base_type = bitset_adaptor<detail::bits::contiguous_bit_inplace_vector<Block, N>, basic_inplace_bitset<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

template<std::size_t N>
using inplace_bitset = basic_inplace_bitset<std::size_t, N>;

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<xstd::unsigned_integer Block, std::size_t N>
struct owned_storage<basic_inplace_bitset<Block, N>> : owned_storage<typename basic_inplace_bitset<Block, N>::adaptor_type>
{};

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_inplace_bitset<Block, N>> : hash<typename xstd::basic_inplace_bitset<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_INPLACE_BITSET_HPP
