//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SPAN_HPP
#define XSTD_BITS_BIT_SPAN_HPP

#include <boost/container_hash/is_range.hpp> // is_range
#include <xstd/bits/basic_bit_sequence.hpp> // basic_bit_sequence
#include <xstd/bits/bit_traits.hpp>         // bit_storage, bit_traits
#include <xstd/bits/ownership.hpp>          // owned_bits_t, owned_storage, owned_traits_t, owner_of, ownership
#include <ranges>                           // enable_borrowed_range, enable_view
#include <type_traits>                      // false_type, remove_const_t

// The sequence reading over bits it does not own: the referring adaptor, which like std::span neither compares nor orders. [design.md#the-views-are-the-adaptors]
namespace xstd {

// Derived rather than aliased, MSVC deducing no arguments through an alias template; the constructors are spelled out rather than
// inherited, because inheriting them inherits the primary's guides too (P2582), which would tie with the ones restated below.
template<class Bits, bit_storage<std::remove_const_t<Bits>> Traits = bit_traits<std::remove_const_t<Bits>>>
class bit_span : public basic_bit_sequence<Bits, ownership::refers, false, Traits>
{
        using base = basic_bit_sequence<Bits, ownership::refers, false, Traits>;

public:
        [[nodiscard]] constexpr explicit bit_span(Bits& c) noexcept : base(c) {}

        template<owner_of<Bits, Traits> Owner>
        [[nodiscard]] constexpr explicit bit_span(Owner& c) noexcept : base(c) {}
};

// The primary's two guides, restated: a view deduces the constness of what it views, and over an owner views the storage it wraps.
template<class Bits>
bit_span(Bits&) -> bit_span<Bits>;

template<class Owner>
        requires requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; }
bit_span(Owner&) -> bit_span<owned_bits_t<Owner>, owned_traits_t<Owner>>;

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins the referring adaptor already makes, restated for the derived name.
namespace std::ranges {

template<class Bits, class Traits>
inline constexpr bool enable_view<xstd::bit_span<Bits, Traits>> = true;

template<class Bits, class Traits>
inline constexpr bool enable_borrowed_range<xstd::bit_span<Bits, Traits>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

// A derived class is not its base to a partial specialization, so the view restates that it is no range to ContainerHash; it has no hook, as it has no ==. [design.md#the-hashing-invariant]
namespace boost::container_hash {

template<class Bits, class Traits>
struct is_range<xstd::bit_span<Bits, Traits>> : std::false_type {};

}       // namespace boost::container_hash

#endif  // XSTD_BITS_BIT_SPAN_HPP
