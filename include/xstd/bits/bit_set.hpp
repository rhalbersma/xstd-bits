//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/detail/ownership.hpp>             // owned_storage, storage
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/detail/set_adaptor.hpp>           // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <cstddef>                                    // size_t
#include <memory>                                     // allocator
#include <functional>                                 // hash
#include <type_traits>                                // false_type
#include <boost/container_hash/is_range.hpp>          // is_range
#include <boost/container_hash/is_tuple_like.hpp>     // is_tuple_like

namespace xstd {

// The set reading over a heap of blocks: the flagship, and the one name without a qualifier.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_bit_set : public detail::bits::set_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, detail::bits::storage::owned, basic_bit_set<Block, Allocator>>
{
        using base_type = detail::bits::set_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, detail::bits::storage::owned, basic_bit_set<Block, Allocator>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_set& x, basic_bit_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using bit_set = basic_bit_set<std::size_t>;

} // namespace xstd

namespace xstd::detail::bits {

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<xstd::unsigned_integer Block, class Allocator>
struct owned_storage<basic_bit_set<Block, Allocator>> : owned_storage<typename basic_bit_set<Block, Allocator>::adaptor_type>
{};

} // namespace xstd::detail::bits

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, class Allocator>
struct hash<xstd::basic_bit_set<Block, Allocator>> : hash<typename xstd::basic_bit_set<Block, Allocator>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace boost::container_hash {

// Only a reading with iterators needs saying: a bitset has none, so the primary template already answers false.
template<xstd::unsigned_integer Block, class Allocator>
struct is_range<xstd::basic_bit_set<Block, Allocator>> : std::false_type
{};

template<xstd::unsigned_integer Block, class Allocator>
struct is_tuple_like<xstd::basic_bit_set<Block, Allocator>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_SET_HPP
