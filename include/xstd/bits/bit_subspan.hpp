//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SUBSPAN_HPP
#define XSTD_BITS_BIT_SUBSPAN_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, owner_reading, reading, storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <cstddef>                                       // size_t
#include <ranges>                                        // enable_borrowed_range, enable_view
#include <type_traits>                                   // false_type

namespace xstd {

// A window on the sequence reading: what first, last and subspan hand back, its width in the type where it can be.
template<specialization_of_TN<bits::detail::contiguous_bit_container> Bits, std::size_t Extent>
class bit_subspan : public bits::detail::sequence_adaptor<Bits, bits::detail::storage::borrowed, bits::detail::window::sub, bit_subspan<Bits, Extent>, Extent>
{
        using base_type = bits::detail::sequence_adaptor<Bits, bits::detail::storage::borrowed, bits::detail::window::sub, bit_subspan<Bits, Extent>, Extent>;

public:
        static constexpr std::size_t extent = Extent;

        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<specialization_of_TN<bits::detail::contiguous_bit_container> Bits>
bit_subspan(Bits&) -> bit_subspan<Bits>;

template<bits::detail::owner_reading<bits::detail::reading::sequence> Owner>
bit_subspan(Owner&) -> bit_subspan<bits::detail::owned_bits_t<Owner>>;

} // namespace xstd

namespace xstd::bits::detail {

// A view answers every trait as the vehicle it is built on, which is where each one is defined.
template<class Bits, std::size_t Extent, class Block>
inline constexpr bool blit_source<bit_subspan<Bits, Extent>, Block> = blit_source<typename bit_subspan<Bits, Extent>::adaptor_type, Block>; // NOLINT(readability-redundant-typename)

} // namespace xstd::bits::detail

namespace boost::container_hash {

template<class Bits, std::size_t Extent>
struct is_range<xstd::bit_subspan<Bits, Extent>> : std::false_type
{};

template<class Bits, std::size_t Extent>
struct is_tuple_like<xstd::bit_subspan<Bits, Extent>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.
namespace std::ranges {

template<class Bits, std::size_t Extent>
inline constexpr bool enable_view<xstd::bit_subspan<Bits, Extent>> = true;

template<class Bits, std::size_t Extent>
inline constexpr bool enable_borrowed_range<xstd::bit_subspan<Bits, Extent>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SUBSPAN_HPP
