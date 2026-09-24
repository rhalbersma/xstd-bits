//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_ADAPTOR_HPP
#define XSTD_BITS_BITSET_ADAPTOR_HPP

#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, default_extent_v, num_blocks_v
#include <xstd/bits/from_bits.hpp>                       // from_bits_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/limits.hpp>                          // numeric_limits
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <limits>                                        // numeric_limits

namespace xstd {

// A field of bits in a storage of blocks it owns, read as a whole: std::bitset's reading over any block container.
template<class Blocks, std::size_t N = bits::detail::default_extent_v<Blocks>>
class bitset_adaptor : public bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<Blocks, N>, bitset_adaptor<Blocks, N>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<Blocks, N>, bitset_adaptor<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(bitset_adaptor& x, bitset_adaptor& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

// Block counts computed as the aliases compute them; the defaulted K keeps MSVC 17 from dropping the one-word guide.
template<xstd::unsigned_integer B, std::size_t K = 1>
bitset_adaptor(from_bits_t, B) -> bitset_adaptor<std::array<B, bits::detail::num_blocks_v<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

template<xstd::unsigned_integer B, std::size_t K>
bitset_adaptor(from_bits_t, std::array<B, K>) -> bitset_adaptor<std::array<B, bits::detail::num_blocks_v<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

// std::bitset's integer constructor at the width of the integer's type, where that constructor reads every digit.
template<xstd::unsigned_integer B, std::size_t K = 1>
        requires (xstd::numeric_limits<B>::digits <= std::numeric_limits<unsigned long long>::digits)
bitset_adaptor(B) -> bitset_adaptor<std::array<std::size_t, bits::detail::num_blocks_v<std::size_t, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Blocks, std::size_t N>
struct hash<xstd::bitset_adaptor<Blocks, N>> : hash<typename xstd::bitset_adaptor<Blocks, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BITSET_ADAPTOR_HPP
