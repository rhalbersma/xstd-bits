//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_HPP
#define XSTD_BITS_EXT_BOOST_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                // owned_storage, storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <type_traits>                                   // false_type
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like

namespace xstd::detail::bits {

// The fourth vehicle, from outside the standard library: inline up to a capacity of N bits, on the heap past it.
template<xstd::unsigned_integer Block, std::size_t N, class Allocator>
using contiguous_bit_small_vector =
        contiguous_bit_container<boost::container::small_vector<Block, num_blocks_v<Block, N>, Allocator>>;

} // namespace xstd::detail::bits

namespace xstd {

// The allocator is Boost's own, since the container this column is built on defaults to that one rather than std's.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_bit_small_set : public set_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, storage::owned, basic_bit_small_set<Block, N, Alloc>>
{
        using base_type = set_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, storage::owned, basic_bit_small_set<Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // An allocator names std among the associated namespaces, where std::swap would out-match the container's own.
        friend constexpr auto swap(basic_bit_small_set& x, basic_bit_small_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_bit_small_vector : public sequence_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, storage::owned, window::all, basic_bit_small_vector<Block, N, Alloc>>
{
        using base_type = sequence_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, storage::owned, window::all, basic_bit_small_vector<Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // An allocator names std among the associated namespaces, where std::swap would out-match the container's own.
        friend constexpr auto swap(basic_bit_small_vector& x, basic_bit_small_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_small_bitset : public bitset_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, basic_small_bitset<Block, N, Alloc>>
{
        using base_type = bitset_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, basic_small_bitset<Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // An allocator names std among the associated namespaces, where std::swap would out-match the container's own.
        friend constexpr auto swap(basic_small_bitset& x, basic_small_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_small_set = basic_bit_small_set<std::size_t, N>;

template<std::size_t N>
using bit_small_vector = basic_bit_small_vector<std::size_t, N>;

template<std::size_t N>
using small_bitset = basic_small_bitset<std::size_t, N>;

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct owned_storage<basic_bit_small_set<Block, N, Alloc>> : owned_storage<typename basic_bit_small_set<Block, N, Alloc>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct owned_storage<basic_bit_small_vector<Block, N, Alloc>> : owned_storage<typename basic_bit_small_vector<Block, N, Alloc>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct owned_storage<basic_small_bitset<Block, N, Alloc>> : owned_storage<typename basic_small_bitset<Block, N, Alloc>::adaptor_type>
{};

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct hash<xstd::basic_bit_small_set<Block, N, Alloc>> : hash<typename xstd::basic_bit_small_set<Block, N, Alloc>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct hash<xstd::basic_bit_small_vector<Block, N, Alloc>> : hash<typename xstd::basic_bit_small_vector<Block, N, Alloc>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct hash<xstd::basic_small_bitset<Block, N, Alloc>> : hash<typename xstd::basic_small_bitset<Block, N, Alloc>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_range<xstd::basic_bit_small_set<Block, N, Alloc>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_tuple_like<xstd::basic_bit_small_set<Block, N, Alloc>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_range<xstd::basic_bit_small_vector<Block, N, Alloc>> : std::false_type
{};

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct is_tuple_like<xstd::basic_bit_small_vector<Block, N, Alloc>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_EXT_BOOST_HPP
