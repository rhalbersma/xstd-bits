//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_VIEW_HPP
#define XSTD_BITS_BIT_SET_VIEW_HPP

#include <xstd/bits/detail/borrowed_bits.hpp>            // borrowable_word, borrowable_words
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, owner_reading, reading, storage
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/bits/detail/words.hpp>                    // block_words, lent_words_t, view_storage_t, words_extent_v, words_of_t, words_width_v
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/container_hash/is_range.hpp>             // is_range
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <ranges>                                        // enable_borrowed_range, enable_view
#include <type_traits>                                   // false_type

// The set reading over bits it does not own: the referring adaptor under the name the sieve calls it by.
namespace xstd {

// A class rather than an alias to the referring adaptor, so deduction and diagnostics name the view itself.
template<bits::detail::block_words Blocks, std::size_t N = bits::detail::words_extent_v<Blocks>>
class bit_set_view : public bits::detail::set_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bit_set_view<Blocks, N>>
{
        using base_type = bits::detail::set_adaptor<bits::detail::view_storage_t<Blocks, N>, bits::detail::storage::borrowed, bit_set_view<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<specialization_of_TN<bits::detail::contiguous_bit_container> Bits>
bit_set_view(Bits&) -> bit_set_view<bits::detail::words_of_t<Bits>, bits::detail::words_width_v<Bits>>;

template<bits::detail::owner_reading<bits::detail::reading::set> Owner>
bit_set_view(Owner&) -> bit_set_view<bits::detail::words_of_t<bits::detail::owned_bits_t<Owner>>, bits::detail::words_width_v<bits::detail::owned_bits_t<Owner>>>;

// Words handed straight over: a word as itself, a contiguous range as the span that lends it, const where they are.
template<class W>
        requires bits::detail::borrowable_word<W&&> or bits::detail::borrowable_words<W&&>
bit_set_view(W&&) -> bit_set_view<bits::detail::lent_words_t<W&&>>;

} // namespace xstd

namespace boost::container_hash {

template<class Blocks, std::size_t N>
struct is_range<xstd::bit_set_view<Blocks, N>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Blocks, std::size_t N>
struct hash<xstd::bit_set_view<Blocks, N>> : hash<typename xstd::bit_set_view<Blocks, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.
namespace std::ranges {

template<class Blocks, std::size_t N>
inline constexpr bool enable_view<xstd::bit_set_view<Blocks, N>> = true;

template<class Blocks, std::size_t N>
inline constexpr bool enable_borrowed_range<xstd::bit_set_view<Blocks, N>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SET_VIEW_HPP
