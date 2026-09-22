//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_SMALL_BITSET_HPP
#define XSTD_BITS_EXT_BOOST_SMALL_BITSET_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>                        // bitset_adaptor
#include <xstd/bits/ext/boost/detail/contiguous_bit_small_vector.hpp> // contiguous_bit_small_vector
#include <xstd/ints/concepts/unsigned_integer.hpp>                    // unsigned_integer
#include <boost/container/new_allocator.hpp>                          // new_allocator
#include <cstddef>                                                    // size_t
#include <functional>                                                 // hash

namespace xstd {

// The bitset reading over the small-vector column; the allocator is Boost's own, as that container defaults to it.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_small_bitset : public detail::bits::bitset_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, basic_small_bitset<Block, N, Alloc>>
{
        using base_type = detail::bits::bitset_adaptor<detail::bits::contiguous_bit_small_vector<Block, N, Alloc>, basic_small_bitset<Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_small_bitset& x, basic_small_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using small_bitset = basic_small_bitset<std::size_t, N>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct hash<xstd::basic_small_bitset<Block, N, Alloc>> : hash<typename xstd::basic_small_bitset<Block, N, Alloc>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_EXT_BOOST_SMALL_BITSET_HPP
