//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STATIC_SET_HPP
#define XSTD_BITS_BIT_STATIC_SET_HPP

#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/detail/ownership.hpp>            // storage
#include <xstd/bits/detail/set_adaptor.hpp>          // set_adaptor
#include <xstd/bits/from_bits.hpp>                   // from_bits_t
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <xstd/ints/limits.hpp>                      // numeric_limits
#include <xstd/ints/memory.hpp>                      // align_up
#include <boost/container_hash/is_range.hpp>         // is_range
#include <boost/container_hash/is_tuple_like.hpp>    // is_tuple_like
#include <array>                                     // array
#include <cstddef>                                   // size_t
#include <functional>                                // hash
#include <limits>                                    // digits
#include <type_traits>                               // false_type

namespace xstd {

// The static set: the basic name leaves the block open, the restricted one is the machine word.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_static_set : public detail::bits::set_adaptor<detail::bits::contiguous_bit_array<Block, N>, detail::bits::storage::owned, basic_bit_static_set<Block, N>>
{
        using base_type = detail::bits::set_adaptor<detail::bits::contiguous_bit_array<Block, N>, detail::bits::storage::owned, basic_bit_static_set<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_static_set& x, basic_bit_static_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

// The width a field of bits carries in its type: an integer's digits, or an array's blocks of them.
template<xstd::unsigned_integer B>
basic_bit_static_set(from_bits_t, B) -> basic_bit_static_set<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits)>;

template<xstd::unsigned_integer B, std::size_t K>
basic_bit_static_set(from_bits_t, std::array<B, K>) -> basic_bit_static_set<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<std::size_t, N>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_static_set = xstd::basic_bit_static_set<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_bit_static_set<Block, N>> : hash<typename xstd::basic_bit_static_set<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, std::size_t N>
struct is_range<xstd::basic_bit_static_set<Block, N>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N>
struct is_tuple_like<xstd::basic_bit_static_set<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_STATIC_SET_HPP
