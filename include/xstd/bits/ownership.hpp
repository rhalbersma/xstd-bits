//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_OWNERSHIP_HPP
#define XSTD_BITS_OWNERSHIP_HPP

#include <concepts>    // same_as
#include <type_traits> // conditional_t, is_const_v, remove_const_t

namespace xstd {

// The one template parameter owning-versus-viewing collapses to: an enum rather than a bool, so a diagnostic reads it.
enum class ownership : bool { refers,
                              owns,
};

[[nodiscard]] constexpr auto owns(ownership o) noexcept
        -> bool
{
        return o == ownership::owns;
}

// Which reading an owner is committed to; a bitset is committed to neither, which is what its two views are for.
enum class reading : unsigned char { set,
                                     sequence,
                                     bitset,
};

// What an owner wraps, specialized beside each owner: declared, never defined, so a view over a type that owns nothing is a constraint not satisfied.
template<class Owner>
struct owned_storage;

// The storage a view over an owner refers to, const where the owner is.
template<class Owner>
using owned_bits_t = std::conditional_t<std::is_const_v<Owner>, typename owned_storage<std::remove_const_t<Owner>>::bits_type const, typename owned_storage<std::remove_const_t<Owner>>::bits_type>;

// Whether Owner is an owner that a view of reading R may refer into. A set owner is already committed to the set reading, so a sequence view over it would choose for the caller; a bitset is committed to neither, which is why either view may refer into one.
template<class Owner, reading R>
concept owner_reading =
        requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; } and
        (owned_storage<std::remove_const_t<Owner>>::reads == R or owned_storage<std::remove_const_t<Owner>>::reads == reading::bitset);

// Whether a view of reading R over Bits can refer into Owner: a reading that does not mix with the owner's, the same storage, and const flowing only from the owner into the view.
template<class Owner, class Bits, reading R>
concept owner_of =
        owner_reading<Owner, R> and
        std::same_as<typename owned_storage<std::remove_const_t<Owner>>::bits_type, std::remove_const_t<Bits>> and
        (std::is_const_v<Bits> or not std::is_const_v<Owner>);

} // namespace xstd

#endif // XSTD_BITS_OWNERSHIP_HPP
