//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SPAN_HPP
#define XSTD_BITS_BIT_SPAN_HPP

#include <xstd/bits/bit_storage.hpp>                     // bit_storage
#include <xstd/bits/bit_subspan.hpp>                     // bit_subspan, what first, last and subspan hand back
#include <xstd/bits/detail/borrowed_bits.hpp>            // borrowable_word, borrowable_words
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, owner_reading, reading, storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor, window_of
#include <xstd/bits/detail/words.hpp>                    // lent_words_t, view_storage_t, words_extent_v, words_of_t, words_width_v
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <cstddef>                                       // size_t
#include <ranges>                                        // enable_borrowed_range, enable_view
#include <type_traits>                                   // false_type

// The sequence reading over bits it does not own: like std::span it neither compares nor orders.
namespace xstd {

template<bit_storage Blocks, std::size_t N = bits::detail::words_extent_v<Blocks>>
class bit_span;

} // namespace xstd

namespace xstd::bits::detail {

// A window of the whole sequence is named by the same words and width.
template<class Blocks, std::size_t N, class Bits, std::size_t E>
struct window_of<bit_span<Blocks, N>, Bits, E>
{
        using type = bit_subspan<Blocks, E, N>;
};

} // namespace xstd::bits::detail

namespace xstd {

// Differs from bit_subspan in one non-type argument: this is the whole sequence, that one a window.
template<bit_storage Blocks, std::size_t N>
class bit_span : public bits::detail::sequence_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bits::detail::window::all, bit_span<Blocks, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bits::detail::window::all, bit_span<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<specialization_of_TN<bits::detail::contiguous_bit_container> Bits>
bit_span(Bits&) -> bit_span<bits::detail::words_of_t<Bits>, bits::detail::words_width_v<Bits>>;

template<bits::detail::owner_reading<bits::detail::reading::sequence> Owner>
bit_span(Owner&) -> bit_span<bits::detail::words_of_t<bits::detail::owned_bits_t<Owner>>, bits::detail::words_width_v<bits::detail::owned_bits_t<Owner>>>;

// Words handed straight over: a word as itself, a contiguous range as the span that lends it, const where they are.
template<class W>
        requires bits::detail::borrowable_word<W&&> or bits::detail::borrowable_words<W&&>
bit_span(W&&) -> bit_span<bits::detail::lent_words_t<W&&>>;

} // namespace xstd

namespace xstd::bits::detail {

// A view answers every trait as the vehicle it is built on, which is where each one is defined.
template<class Blocks, std::size_t N, class Block>
inline constexpr bool blit_source<bit_span<Blocks, N>, Block> = blit_source<typename bit_span<Blocks, N>::adaptor_type, Block>; // NOLINT(readability-redundant-typename)

} // namespace xstd::bits::detail

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
