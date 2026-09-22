//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SUBSPAN_HPP
#define XSTD_BITS_BIT_SUBSPAN_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // storage, window
#include <xstd/bits/detail/tags.hpp>                     // sequence_reading_tag
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <ranges>                                        // enable_borrowed_range, enable_view
#include <type_traits>                                   // false_type, remove_const_t

namespace xstd {

// A window on the sequence reading: what first, last and subspan hand back.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits>
class bit_subspan : public sequence_adaptor<Bits, storage::borrowed, window::sub, bit_subspan<Bits>>
{
        using base_type = sequence_adaptor<Bits, storage::borrowed, window::sub, bit_subspan<Bits>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
bit_subspan(Bits&) -> bit_subspan<Bits>;

template<owner_reading<sequence_reading_tag> Owner>
bit_subspan(Owner&) -> bit_subspan<owned_bits_t<Owner>>;

// A view answers every trait as the vehicle it is built on, which is where each one is defined.
template<class Bits, class Block>
inline constexpr bool blit_source<bit_subspan<Bits>, Block> = blit_source<typename bit_subspan<Bits>::adaptor_type, Block>; // NOLINT(readability-redundant-typename)

} // namespace xstd

namespace boost::container_hash {

template<class Bits>
struct is_range<xstd::bit_subspan<Bits>> : std::false_type
{};

template<class Bits>
struct is_tuple_like<xstd::bit_subspan<Bits>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.
namespace std::ranges {

template<class Bits>
inline constexpr bool enable_view<xstd::bit_subspan<Bits>> = true;

template<class Bits>
inline constexpr bool enable_borrowed_range<xstd::bit_subspan<Bits>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SUBSPAN_HPP
