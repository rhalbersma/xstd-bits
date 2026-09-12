//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ALLOCATOR_TYPEDEF_HPP
#define XSTD_BITS_DETAIL_ALLOCATOR_TYPEDEF_HPP

#include <xstd/misc/type_traits/empty_type.hpp> // empty_type
#include <compare>                              // strong_ordering

namespace xstd::detail::bits {

// The allocator's name where the storage below has one and nothing where it does not: an empty base, a class having no conditional typedef.
// Equality is defaulted so a derived class's defaulted == still compares, this base having nothing to compare. [design.md#a-strict-extension]
// Neither constexpr nor noexcept is written: a defaulted comparison deduces both, and xstd::empty_type's own
// defaulted <=> writes neither either. [design.md#a-strict-extension]
template<class Storage>
struct allocator_typedef
{
        [[nodiscard]] friend auto operator==(allocator_typedef const&, allocator_typedef const&) -> bool = default;
};

template<class Storage>
        requires requires { typename Storage::allocator_type; }
struct allocator_typedef<Storage>
{
        using allocator_type = Storage::allocator_type;

        [[nodiscard]] friend auto operator==(allocator_typedef const&, allocator_typedef const&) -> bool = default;
};

// A view's base in the same position, and empty_type's two comparisons taken back off it: a view is as
// incomparable as std::span, which P1085 stripped of both. empty_type carries a defaulted <=> -- and, by
// [class.compare.default], an implicitly declared defaulted == beside it -- which ADL finds for a DERIVED
// argument, and which would answer "equal" for any two views, having only the empty base to compare.
//
// Deleted here rather than in the adaptor, and so unconstrained, because MSVC rejects a trailing requires clause
// on a deleted friend (C7599) where it accepts one on a defaulted friend. Unconstrained is also the better
// shape: this base is the view's and the owner's is allocator_typedef, so the condition is the base already.
// These win over empty_type's for a derived argument, binding to a MORE DERIVED base being the better
// conversion, so the answer is ill-formed rather than wrong. Both are needed and neither implies the other: <=>
// rewrites the four relationals and never ==, != rewrites from == and never from <=>.
// [design.md#views-follow-their-precedent]
struct incomparable_base : xstd::empty_type<>
{
        [[nodiscard]] friend auto operator==(incomparable_base const&, incomparable_base const&) -> bool = delete;
        [[nodiscard]] friend auto operator<=>(incomparable_base const&, incomparable_base const&) -> std::strong_ordering = delete;
};

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_ALLOCATOR_TYPEDEF_HPP
