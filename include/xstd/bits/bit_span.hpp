//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SPAN_HPP
#define XSTD_BITS_BIT_SPAN_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // bit_blocks
#include <xstd/bits/bit_subspan.hpp>                       // IWYU pragma: keep; bit_subspan, what first, last and subspan hand back
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container, bit_block_container_type
#include <xstd/bits/detail/blocks.hpp>                     // blocks_of_t, blocks_width_v
#include <xstd/bits/detail/borrowed_bits.hpp>              // borrowable_block, borrowable_blocks, lent_blocks_t, view_storage_t
#include <xstd/bits/detail/ownership.hpp>                  // owned_bits_t, owner_reading, sequence_reading_tag, storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>           // sequence_adaptor
#include <xstd/bits/detail/views.hpp>                      // window_of
#include <boost/container_hash/is_range.hpp>               // is_range
#include <boost/container_hash/is_tuple_like.hpp>          // is_tuple_like
#include <cstddef>                                         // size_t
#include <ranges>                                          // enable_borrowed_range, enable_view
#include <type_traits>                                     // conditional_t, false_type, is_const_v

// The sequence reading over bits it does not own: like std::span it neither compares nor orders.
namespace xstd {

// Differs from bit_subspan in one non-type argument: this is the whole sequence, that one a window.
template<bit_blocks Blocks, std::size_t N = bit_blocks_extent_v<Blocks>>
class bit_span : public bits::detail::sequence_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bits::detail::window::all, bit_span<Blocks, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bits::detail::window::all, bit_span<Blocks, N>>;

public:
        // std::span's element_type, const where the bits are, beside the value_type of bool the adaptor declares.
        using element_type = std::conditional_t<std::is_const_v<typename base_type::adapted_type>, bool const, bool>;

        static constexpr std::size_t extent = N;

        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<bits::detail::bit_block_container_type Bits>
bit_span(Bits&) -> bit_span<bits::detail::blocks_of_t<Bits>, bits::detail::blocks_width_v<Bits>>;

template<bits::detail::owner_reading<bits::detail::sequence_reading_tag> Owner>
bit_span(Owner&) -> bit_span<bits::detail::blocks_of_t<bits::detail::owned_bits_t<Owner>>, bits::detail::blocks_width_v<bits::detail::owned_bits_t<Owner>>>;

// Blocks handed straight over: a block as itself, a contiguous range as the span that lends it, const where they are.
template<class W>
        requires bits::detail::borrowable_block<W&&> or bits::detail::borrowable_blocks<W&&>
bit_span(W&&) -> bit_span<bits::detail::lent_blocks_t<W&&>>;

} // namespace xstd

namespace boost::container_hash {

template<class Blocks, std::size_t N>
struct is_range<xstd::bit_span<Blocks, N>> : std::false_type
{};

template<class Blocks, std::size_t N>
struct is_tuple_like<xstd::bit_span<Blocks, N>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.

namespace std::ranges {

template<class Blocks, std::size_t N>
inline constexpr bool enable_view<xstd::bit_span<Blocks, N>> = true;

template<class Blocks, std::size_t N>
inline constexpr bool enable_borrowed_range<xstd::bit_span<Blocks, N>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SPAN_HPP
