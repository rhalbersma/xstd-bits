//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_HPP

#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/detail/ownership.hpp>            // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>     // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <xstd/ints/memory.hpp>                      // align_up
#include <boost/container_hash/is_range.hpp>         // is_range
#include <boost/container_hash/is_tuple_like.hpp>    // is_tuple_like
#include <cstddef>                                   // size_t
#include <functional>                                // hash
#include <limits>                                    // digits
#include <tuple>                                     // tuple_element, tuple_size
#include <type_traits>                               // false_type

namespace xstd {

// The packed std::array<bool, N>, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_array : public detail::bits::sequence_adaptor<detail::bits::contiguous_bit_array<Block, N>, detail::bits::storage::owned, detail::bits::window::all, basic_bit_array<Block, N>>
{
        using base_type = detail::bits::sequence_adaptor<detail::bits::contiguous_bit_array<Block, N>, detail::bits::storage::owned, detail::bits::window::all, basic_bit_array<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_array& x, basic_bit_array& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_array = xstd::basic_bit_array<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_bit_array<Block, N>> : hash<typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

template<std::size_t I, xstd::unsigned_integer Block, std::size_t N>
struct tuple_element<I, xstd::basic_bit_array<Block, N>> : tuple_element<I, typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

template<std::size_t I, xstd::unsigned_integer Block, std::size_t N>
struct tuple_element<I, const xstd::basic_bit_array<Block, N>> : tuple_element<I, const typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t N>
struct tuple_size<xstd::basic_bit_array<Block, N>> : tuple_size<typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, std::size_t N>
struct is_range<xstd::basic_bit_array<Block, N>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N>
struct is_tuple_like<xstd::basic_bit_array<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_ARRAY_HPP
