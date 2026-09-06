//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_RANGES_SET_VIEW_HPP
#define XSTD_BITS_RANGES_SET_VIEW_HPP

#include <xstd/bits/basic_bit_set.hpp> // basic_bit_set
#include <xstd/bits/bit_traits.hpp>    // bit_storage, bit_traits
#include <xstd/bits/ownership.hpp>     // owned_bits_t, owned_storage, owned_traits_t, owner_of, ownership
#include <ranges>                      // enable_borrowed_range, enable_view
#include <type_traits>                 // remove_const_t

// The set reading over bits it does not own: the referring adaptor under the name the sieve calls it by. [design.md#the-views-are-the-adaptors]
namespace xstd::ranges {

// Derived rather than aliased, MSVC deducing no arguments through an alias template; the constructors are spelled out rather than
// inherited, because inheriting them inherits the primary's guides too (P2582), which would tie with the ones restated below.
template<class Bits, bit_storage<std::remove_const_t<Bits>> Traits = bit_traits<std::remove_const_t<Bits>>>
class set_view : public basic_bit_set<Bits, ownership::refers, Traits>
{
        using base = basic_bit_set<Bits, ownership::refers, Traits>;

public:
        [[nodiscard]] constexpr explicit set_view(Bits& c) noexcept : base(c) {}

        template<owner_of<Bits, Traits> Owner>
        [[nodiscard]] constexpr explicit set_view(Owner& c) noexcept : base(c) {}
};

// The primary's two guides, restated: a view deduces the constness of what it views, and over an owner views the storage it wraps.
template<class Bits>
set_view(Bits&) -> set_view<Bits>;

template<class Owner>
        requires requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; }
set_view(Owner&) -> set_view<owned_bits_t<Owner>, owned_traits_t<Owner>>;

}       // namespace xstd::ranges

namespace xstd {

using ranges::set_view;

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins the referring adaptor already makes, restated for the derived name.
namespace std::ranges {

template<class Bits, class Traits>
inline constexpr bool enable_view<xstd::ranges::set_view<Bits, Traits>> = true;

template<class Bits, class Traits>
inline constexpr bool enable_borrowed_range<xstd::ranges::set_view<Bits, Traits>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

#endif  // XSTD_BITS_RANGES_SET_VIEW_HPP
