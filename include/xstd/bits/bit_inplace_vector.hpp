//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/detail/ownership.hpp>                     // storage, window
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // contiguous_bit_inplace_vector
#include <xstd/bits/detail/sequence_adaptor.hpp>              // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <cstddef>                                            // size_t
#include <functional>                                         // hash
#include <type_traits>                                        // false_type
#include <boost/container_hash/is_range.hpp>                  // is_range
#include <boost/container_hash/is_tuple_like.hpp>             // is_tuple_like

namespace xstd {

// The packed std::inplace_vector<bool, N> that P0843 declined to write, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_inplace_vector : public detail::bits::sequence_adaptor<detail::bits::contiguous_bit_inplace_vector<Block, N>, detail::bits::storage::owned, detail::bits::window::all, basic_bit_inplace_vector<Block, N>>
{
        using base_type = detail::bits::sequence_adaptor<detail::bits::contiguous_bit_inplace_vector<Block, N>, detail::bits::storage::owned, detail::bits::window::all, basic_bit_inplace_vector<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_inplace_vector& x, basic_bit_inplace_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_inplace_vector = basic_bit_inplace_vector<std::size_t, N>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_bit_inplace_vector<Block, N>> : hash<typename xstd::basic_bit_inplace_vector<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, std::size_t N>
struct is_range<xstd::basic_bit_inplace_vector<Block, N>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N>
struct is_tuple_like<xstd::basic_bit_inplace_vector<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BIT_INPLACE_VECTOR_HPP
