//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SEQUENCE_HPP
#define XSTD_BITS_BIT_SEQUENCE_HPP

#include <xstd/bits/bit_array.hpp>                 // basic_bit_array
#include <xstd/bits/bit_inplace_vector.hpp>        // IWYU pragma: keep; basic_bit_inplace_vector, named only under __cpp_lib_inplace_vector
#include <xstd/bits/bit_vector.hpp>                // basic_bit_vector
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <array>                                   // array
#include <cstddef>                                 // size_t
#include <vector>                                  // vector
#include <version>                                 // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

#endif

namespace xstd::detail::bits {

// Declared and never defined: a storage with no sequence of its own is an incomplete type, refused by the alias.
template<class C>
struct sequence_for;

template<xstd::unsigned_integer Block>
struct sequence_for<Block>
{
        using type = basic_bit_array<Block, static_cast<std::size_t>(xstd::numeric_limits<Block>::digits)>;
};

template<xstd::unsigned_integer Block, std::size_t K>
struct sequence_for<std::array<Block, K>>
{
        using type = basic_bit_array<Block, static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) * K>;
};

template<xstd::unsigned_integer Block, class Allocator>
struct sequence_for<std::vector<Block, Allocator>>
{
        using type = basic_bit_vector<Block, Allocator>;
};

#ifdef __cpp_lib_inplace_vector

template<xstd::unsigned_integer Block, std::size_t K>
struct sequence_for<std::inplace_vector<Block, K>>
{
        using type = basic_bit_inplace_vector<Block, static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) * K>;
};

#endif

} // namespace xstd::detail::bits

// The sequence over a storage of blocks, named by the storage: an existing owner under a second spelling.
namespace xstd {

// A type computation and nothing more: the owner it names deduces through its own guides, and this cannot.
template<class C>
        requires requires { typename detail::bits::sequence_for<C>::type; }
using bit_sequence = detail::bits::sequence_for<C>::type;

} // namespace xstd

#endif // XSTD_BITS_BIT_SEQUENCE_HPP
