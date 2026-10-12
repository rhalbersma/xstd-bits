//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_VIEW_HPP
#define XSTD_BITS_BIT_SET_VIEW_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container, bit_block_container_type
#include <xstd/bits/detail/blocks.hpp>                     // blocks_of_t, blocks_width_v
#include <xstd/bits/detail/borrowed_bits.hpp>              // borrowable_block, borrowable_blocks, lent_blocks_t, view_storage_t
#include <xstd/bits/detail/ownership.hpp>                  // owned_bits_t, owner_reading, set_reading_tag, storage
#include <xstd/bits/detail/set_adaptor.hpp>                // set_adaptor
#include <boost/container_hash/is_range.hpp>               // is_range
#include <cstddef>                                         // size_t
#include <functional>                                      // hash
#include <ranges>                                          // enable_borrowed_range, enable_view
#include <type_traits>                                     // false_type

// The set reading over bits it does not own: the referring adaptor under the name the sieve calls it by.
namespace xstd {

// A class rather than an alias to the referring adaptor, so deduction and diagnostics name the view itself.
template<bit_blocks Blocks, std::size_t N = bit_blocks_extent_v<Blocks>>
class bit_set_view : public bits::detail::set_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bit_set_view<Blocks, N>>
{
        using base_type = bits::detail::set_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bit_set_view<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<bits::detail::bit_block_container_type Bits>
bit_set_view(Bits&) -> bit_set_view<bits::detail::blocks_of_t<Bits>, bits::detail::blocks_width_v<Bits>>;

template<bits::detail::owner_reading<bits::detail::set_reading_tag> Owner>
bit_set_view(Owner&) -> bit_set_view<bits::detail::blocks_of_t<bits::detail::owned_bits_t<Owner>>, bits::detail::blocks_width_v<bits::detail::owned_bits_t<Owner>>>;

// Blocks handed straight over: a block as itself, a contiguous range as the span that lends it, const where they are.
template<class W>
        requires bits::detail::borrowable_block<W&&> or bits::detail::borrowable_blocks<W&&>
bit_set_view(W&&) -> bit_set_view<bits::detail::lent_blocks_t<W&&>>;

} // namespace xstd

namespace boost::container_hash {

template<class Blocks, std::size_t N>
struct is_range<xstd::bit_set_view<Blocks, N>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Blocks, std::size_t N>
struct hash<xstd::bit_set_view<Blocks, N>> : hash<typename xstd::bit_set_view<Blocks, N>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.

namespace std::ranges {

template<class Blocks, std::size_t N>
inline constexpr bool enable_view<xstd::bit_set_view<Blocks, N>> = true;

template<class Blocks, std::size_t N>
inline constexpr bool enable_borrowed_range<xstd::bit_set_view<Blocks, N>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SET_VIEW_HPP
