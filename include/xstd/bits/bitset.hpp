//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>       // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/from_bits.hpp>                   // from_bits_t
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <xstd/ints/limits.hpp>                      // numeric_limits
#include <xstd/ints/memory.hpp>                      // align_up
#include <array>                                     // array
#include <cstddef>                                   // size_t
#include <functional>                                // hash
#include <limits>                                    // digits

namespace xstd {

// [template.bitset] over a packed array of Block: what std::bitset<N> is, with the word type in the open.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bitset : public detail::bits::bitset_adaptor<detail::bits::contiguous_bit_array<Block, N>, basic_bitset<Block, N>>
{
        using base_type = detail::bits::bitset_adaptor<detail::bits::contiguous_bit_array<Block, N>, basic_bitset<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bitset& x, basic_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

// The width a field of bits carries in its type: an integer's digits, or an array's blocks of them.
template<xstd::unsigned_integer B>
basic_bitset(from_bits_t, B) -> basic_bitset<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits)>;

template<xstd::unsigned_integer B, std::size_t K>
basic_bitset(from_bits_t, std::array<B, K>) -> basic_bitset<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

// std::bitset's integer constructor at the width of the integer's type, where that constructor reads every digit.
template<xstd::unsigned_integer B>
        requires (xstd::numeric_limits<B>::digits <= std::numeric_limits<unsigned long long>::digits)
basic_bitset(B) -> basic_bitset<std::size_t, static_cast<std::size_t>(xstd::numeric_limits<B>::digits)>;

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

// The width rounded up to whole blocks: no unused tail, so every block is the value.
namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bitset = xstd::basic_bitset<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<xstd::unsigned_integer Block, std::size_t N>
struct hash<xstd::basic_bitset<Block, N>> : hash<typename xstd::basic_bitset<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BITSET_HPP
