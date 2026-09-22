//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DYNAMIC_BITSET_HPP
#define XSTD_BITS_DYNAMIC_BITSET_HPP

#include <xstd/bits/detail/ownership.hpp>             // owned_storage
#include <xstd/bits/detail/bitset_adaptor.hpp>        // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <cstddef>                                    // size_t
#include <memory>                                     // allocator
#include <functional>                                 // hash

namespace xstd {

// The bitset reading over a heap of blocks, boost::dynamic_bitset being its counterpart.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_dynamic_bitset : public bitset_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, basic_dynamic_bitset<Block, Allocator>>
{
        using base_type = bitset_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, basic_dynamic_bitset<Block, Allocator>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // An allocator names std among the associated namespaces, where std::swap would out-match the container's own.
        friend constexpr auto swap(basic_dynamic_bitset& x, basic_dynamic_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using dynamic_bitset = basic_dynamic_bitset<std::size_t>;

// A container answers every trait as the vehicle it is built on, which is where each one is defined.
template<xstd::unsigned_integer Block, class Allocator>
struct owned_storage<basic_dynamic_bitset<Block, Allocator>> : owned_storage<typename basic_dynamic_bitset<Block, Allocator>::adaptor_type>
{};

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, class Allocator>
struct hash<xstd::basic_dynamic_bitset<Block, Allocator>> : hash<typename xstd::basic_dynamic_bitset<Block, Allocator>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_DYNAMIC_BITSET_HPP
