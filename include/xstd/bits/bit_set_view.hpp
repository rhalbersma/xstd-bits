//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_VIEW_HPP
#define XSTD_BITS_BIT_SET_VIEW_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, owned_storage, owner_reading, reading, storage
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/container_hash/is_range.hpp>             // is_range
#include <functional>                                    // hash
#include <ranges>                                        // enable_borrowed_range, enable_view
#include <type_traits>                                   // false_type, remove_const_t

// The set reading over bits it does not own: the referring adaptor under the name the sieve calls it by.
namespace xstd {

// A class rather than an alias to the referring adaptor, so deduction and diagnostics name the view itself.
template<specialization_of_TN<bits::detail::contiguous_bit_container> Bits>
class bit_set_view : public bits::detail::set_adaptor<Bits, bits::detail::storage::borrowed, bit_set_view<Bits>>
{
        using base_type = bits::detail::set_adaptor<Bits, bits::detail::storage::borrowed, bit_set_view<Bits>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

// The vehicle's two guides, restated on the view so a consumer deduces the name rather than what it is built on.
template<class Bits>
        requires (not requires { typename bits::detail::owned_storage<std::remove_const_t<Bits>>::bits_type; })
bit_set_view(Bits&) -> bit_set_view<Bits>;

template<bits::detail::owner_reading<bits::detail::reading::set> Owner>
bit_set_view(Owner&) -> bit_set_view<bits::detail::owned_bits_t<Owner>>;

} // namespace xstd

namespace boost::container_hash {

template<class Bits>
struct is_range<xstd::bit_set_view<Bits>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Bits>
struct hash<xstd::bit_set_view<Bits>> : hash<typename xstd::bit_set_view<Bits>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.
namespace std::ranges {

template<class Bits>
inline constexpr bool enable_view<xstd::bit_set_view<Bits>> = true;

template<class Bits>
inline constexpr bool enable_borrowed_range<xstd::bit_set_view<Bits>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SET_VIEW_HPP
