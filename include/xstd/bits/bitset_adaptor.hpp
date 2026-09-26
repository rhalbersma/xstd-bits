//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_ADAPTOR_HPP
#define XSTD_BITS_BITSET_ADAPTOR_HPP

#include <xstd/bits/bit_storage.hpp>                     // bit_storage_capacity_v, bit_storage_extent_v, owned_bit_storage, resizable_bit_storage
#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/bits/detail/words.hpp>                    // owner_storage_t
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/limits.hpp>                          // numeric_limits
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <limits>                                        // numeric_limits
#include <span>                                          // dynamic_extent

namespace xstd {

// A field of bits in a storage of blocks it owns, read as a whole: std::bitset's reading over any block container.
template<owned_bit_storage Blocks, std::size_t N = bit_storage_capacity_v<Blocks>>
        requires (N != std::dynamic_extent) or resizable_bit_storage<Blocks>
class bitset_adaptor : public bits::detail::bitset_adaptor<bits::detail::owner_storage_t<Blocks, N>, bitset_adaptor<Blocks, N>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::owner_storage_t<Blocks, N>, bitset_adaptor<Blocks, N>>;

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

// A word is held as an array of one, at its digits; K = 1 keeps MSVC 17 from dropping the one-word guide.
template<xstd::unsigned_integer Block, std::size_t K = 1>
bitset_adaptor(from_bit_storage_t, Block) -> bitset_adaptor<std::array<Block, bits::detail::num_blocks_v<Block, bit_storage_extent_v<Block> * K>>, bit_storage_extent_v<Block> * K>;

// No guide from zero blocks: an empty array names no width worth deducing.
template<xstd::unsigned_integer Block, std::size_t K>
        requires (K != 0)
bitset_adaptor(from_bit_storage_t, std::array<Block, K>) -> bitset_adaptor<std::array<Block, bits::detail::num_blocks_v<Block, bit_storage_extent_v<std::array<Block, K>>>>, bit_storage_extent_v<std::array<Block, K>>>;

// std::bitset's integer constructor at the width of the integer's type, where that constructor reads every digit.
template<xstd::unsigned_integer Block, std::size_t K = 1>
        requires (xstd::numeric_limits<Block>::digits <= std::numeric_limits<unsigned long long>::digits)
bitset_adaptor(Block) -> bitset_adaptor<std::array<std::size_t, bits::detail::num_blocks_v<std::size_t, bit_storage_extent_v<Block> * K>>, bit_storage_extent_v<Block> * K>;

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Blocks, std::size_t N>
struct hash<xstd::bitset_adaptor<Blocks, N>> : hash<typename xstd::bitset_adaptor<Blocks, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BITSET_ADAPTOR_HPP
