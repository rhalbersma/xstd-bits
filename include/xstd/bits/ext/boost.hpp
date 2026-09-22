//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_HPP
#define XSTD_BITS_EXT_BOOST_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>           // IWYU pragma: keep; the adaptor bitset_reading_tag names
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/grid.hpp>                     // adaptor, bits_of, bits_t
#include <xstd/bits/detail/ownership.hpp>                // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>         // IWYU pragma: keep; the adaptor sequence_reading_tag names
#include <xstd/bits/detail/set_adaptor.hpp>              // IWYU pragma: keep; the adaptor set_reading_tag names
#include <xstd/bits/detail/tags.hpp>                     // bitset_reading_tag, enable_container_tag, sequence_reading_tag, set_reading_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <type_traits>                                   // false_type
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like

namespace xstd {

// The fourth vehicle: a run-time width that stays inline up to a static capacity and reaches the heap past it.
struct small_vector_container_tag
{};

template<>
inline constexpr bool enable_container_tag<small_vector_container_tag> = true;

namespace detail::bits {

template<xstd::unsigned_integer Block, std::size_t N, class Allocator>
using contiguous_bit_small_vector =
        contiguous_bit_container<boost::container::small_vector<Block, num_blocks_v<Block, N>, Allocator>>;

} // namespace detail::bits

// N is a capacity in bits here, as it is for the inplace column, and the width underneath it stays dynamic.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct bits_of<small_vector_container_tag, Block, N, Alloc>
{
        using type = detail::bits::contiguous_bit_small_vector<Block, N, Alloc>;
};

// The allocator is Boost's own, since the container this column is built on defaults to that one rather than std's.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_bit_small_set : public adaptor<set_reading_tag, bits_t<small_vector_container_tag, Block, N, Alloc>, storage::owned, window::all, basic_bit_small_set<Block, N, Alloc>>
{
        using base_type = adaptor<set_reading_tag, bits_t<small_vector_container_tag, Block, N, Alloc>, storage::owned, window::all, basic_bit_small_set<Block, N, Alloc>>;

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
class basic_bit_small_vector : public adaptor<sequence_reading_tag, bits_t<small_vector_container_tag, Block, N, Alloc>, storage::owned, window::all, basic_bit_small_vector<Block, N, Alloc>>
{
        using base_type = adaptor<sequence_reading_tag, bits_t<small_vector_container_tag, Block, N, Alloc>, storage::owned, window::all, basic_bit_small_vector<Block, N, Alloc>>;

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
class basic_small_bitset : public adaptor<bitset_reading_tag, bits_t<small_vector_container_tag, Block, N, Alloc>, storage::owned, window::all, basic_small_bitset<Block, N, Alloc>>
{
        using base_type = adaptor<bitset_reading_tag, bits_t<small_vector_container_tag, Block, N, Alloc>, storage::owned, window::all, basic_small_bitset<Block, N, Alloc>>;

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
