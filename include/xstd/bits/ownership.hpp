//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_OWNERSHIP_HPP
#define XSTD_BITS_OWNERSHIP_HPP

#include <concepts>    // same_as
#include <type_traits> // conditional_t, is_const_v, remove_const_t

namespace xstd {

// The one template parameter owning-versus-viewing collapses to: an enum rather than a bool, so a diagnostic reads it. [design.md#ownership-is-not-an-axis]
enum class ownership : bool { refers, owns };

[[nodiscard]] constexpr auto owns(ownership o) noexcept
        -> bool
{
        return o == ownership::owns;
}

// What an owner wraps, specialized beside each owner as bit_traits is beside each storage: declared, never defined, so a view over a type that owns nothing is a constraint not satisfied. [design.md#views-over-owners]
template<class Owner>
struct owned_storage;

// The storage a view over an owner refers to, const where the owner is.
template<class Owner>
using owned_bits_t = std::conditional_t<std::is_const_v<Owner>, typename owned_storage<std::remove_const_t<Owner>>::bits_type const, typename owned_storage<std::remove_const_t<Owner>>::bits_type>;

template<class Owner>
using owned_traits_t = owned_storage<std::remove_const_t<Owner>>::traits_type;

// Whether a view over Bits through Traits can refer into Owner: the same storage and door, and const flowing only from the owner into the view.
template<class Owner, class Bits, class Traits>
concept owner_of =
        requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; } and
        std::same_as<typename owned_storage<std::remove_const_t<Owner>>::bits_type, std::remove_const_t<Bits>> and
        std::same_as<owned_traits_t<Owner>, Traits> and
        (std::is_const_v<Bits> or not std::is_const_v<Owner>)
;

}       // namespace xstd

#endif  // XSTD_BITS_OWNERSHIP_HPP
