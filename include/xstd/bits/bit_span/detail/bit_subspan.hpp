//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SPAN_DETAIL_BIT_SUBSPAN_HPP
#define XSTD_BITS_BIT_SPAN_DETAIL_BIT_SUBSPAN_HPP

#include <xstd/bits/bit_span/detail/views.hpp>    // IWYU pragma: keep; window_of
#include <xstd/bits/detail/bit_blocks.hpp>        // bit_blocks
#include <xstd/bits/detail/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/borrowed_bits.hpp>     // view_storage_t
#include <xstd/bits/detail/ownership.hpp>         // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>  // sequence_adaptor
#include <boost/container_hash/is_range.hpp>      // is_range
#include <boost/container_hash/is_tuple_like.hpp> // is_tuple_like
#include <cstddef>                                // size_t
#include <ranges>                                 // enable_borrowed_range, enable_view
#include <span>                                   // dynamic_extent
#include <type_traits>                            // conditional_t, false_type, is_const_v

namespace xstd {

// A window on the sequence reading: what first, last and subspan hand back, its width in the type where it can be.
template<bit_blocks Blocks, std::size_t Extent = std::dynamic_extent, std::size_t N = bit_blocks_extent_v<Blocks>>
class bit_subspan : public bits::detail::sequence_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bits::detail::window::sub, bit_subspan<Blocks, Extent, N>, Extent>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bits::detail::window::sub, bit_subspan<Blocks, Extent, N>, Extent>;

public:
        // std::span's element_type, const where the bits are, beside the value_type of bool the adaptor declares.
        using element_type = std::conditional_t<std::is_const_v<typename base_type::adapted_type>, bool const, bool>;

        static constexpr std::size_t extent = Extent;

        using base_type::base_type;
        using base_type::operator=;
};

} // namespace xstd

namespace boost::container_hash {

template<class Blocks, std::size_t Extent, std::size_t N>
struct is_range<xstd::bit_subspan<Blocks, Extent, N>> : std::false_type
{};

template<class Blocks, std::size_t Extent, std::size_t N>
struct is_tuple_like<xstd::bit_subspan<Blocks, Extent, N>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.

namespace std::ranges {

template<class Blocks, std::size_t Extent, std::size_t N>
inline constexpr bool enable_view<xstd::bit_subspan<Blocks, Extent, N>> = true;

template<class Blocks, std::size_t Extent, std::size_t N>
inline constexpr bool enable_borrowed_range<xstd::bit_subspan<Blocks, Extent, N>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SPAN_DETAIL_BIT_SUBSPAN_HPP
