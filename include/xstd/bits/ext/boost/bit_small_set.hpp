//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP
#define XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP

#include <xstd/bits/detail/ownership.hpp>                             // storage
#include <xstd/bits/detail/set_adaptor.hpp>                           // set_adaptor
#include <xstd/bits/ext/boost/detail/contiguous_bit_small_vector.hpp> // contiguous_bit_small_vector
#include <xstd/ints/concepts/unsigned_integer.hpp>                    // unsigned_integer
#include <boost/container/new_allocator.hpp>                          // new_allocator
#include <boost/container_hash/is_range.hpp>                          // is_range
#include <boost/container_hash/is_tuple_like.hpp>                     // is_tuple_like
#include <cstddef>                                                    // size_t
#include <functional>                                                 // hash
#include <type_traits>                                                // false_type

namespace xstd {

// The set reading over the small-vector column; the allocator is Boost's own, as that container defaults to it.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_bit_small_set : public detail::bits::set_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, detail::bits::storage::owned, basic_bit_small_set<Block, N, Alloc>>
{
        using base_type = detail::bits::set_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, detail::bits::storage::owned, basic_bit_small_set<Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_small_set& x, basic_bit_small_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_small_set = basic_bit_small_set<std::size_t, N>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct hash<xstd::basic_bit_small_set<Block, N, Alloc>> : hash<typename xstd::basic_bit_small_set<Block, N, Alloc>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// A reading with iterators has to say so; a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_range<xstd::basic_bit_small_set<Block, N, Alloc>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_tuple_like<xstd::basic_bit_small_set<Block, N, Alloc>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP
