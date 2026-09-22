//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_HPP

#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/detail/ownership.hpp>             // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>      // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <boost/container_hash/is_range.hpp>          // is_range
#include <boost/container_hash/is_tuple_like.hpp>     // is_tuple_like
#include <cstddef>                                    // size_t
#include <functional>                                 // hash
#include <memory>                                     // allocator
#include <type_traits>                                // false_type

namespace xstd {

// The sequence reading over a heap of blocks: std::vector<bool> under the name Hinnant proposed for it.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_bit_vector : public detail::bits::sequence_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, detail::bits::storage::owned, detail::bits::window::all, basic_bit_vector<Block, Allocator>>
{
        using base_type = detail::bits::sequence_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, detail::bits::storage::owned, detail::bits::window::all, basic_bit_vector<Block, Allocator>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_vector& x, basic_bit_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using bit_vector = basic_bit_vector<std::size_t>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, class Allocator>
struct hash<xstd::basic_bit_vector<Block, Allocator>> : hash<typename xstd::basic_bit_vector<Block, Allocator>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, class Allocator>
struct is_range<xstd::basic_bit_vector<Block, Allocator>> : std::false_type
{};

template<xstd::unsigned_integer Block, class Allocator>
struct is_tuple_like<xstd::basic_bit_vector<Block, Allocator>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_VECTOR_HPP
