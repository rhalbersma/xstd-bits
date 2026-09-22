//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_OWNERSHIP_HPP
#define XSTD_BITS_DETAIL_OWNERSHIP_HPP

#include <xstd/bits/detail/tags.hpp> // bitset_reading_tag, reading_tag
#include <concepts>                  // same_as
#include <type_traits>               // conditional_t, is_const_v, remove_const_t

namespace xstd {

// Named for what is owned rather than for the owner: borrowed is the word std::ranges::enable_borrowed_range uses.
enum class storage : bool { owned,
                            borrowed,
};

[[nodiscard]] constexpr auto owns(storage s) noexcept
        -> bool
{
        return s == storage::owned;
}

// Named for how much of the storage a view reaches: all of it, or the sub-range a subspan was cut down to.
enum class window : bool { all,
                           sub,
};

// What an owner wraps: declared, never defined, so a view over a type that owns nothing is unsatisfied.
template<class Owner>
struct owned_storage;

// The storage a view over an owner refers to, const where the owner is.
template<class Owner>
using owned_bits_t = std::conditional_t<std::is_const_v<Owner>, typename owned_storage<std::remove_const_t<Owner>>::bits_type const, typename owned_storage<std::remove_const_t<Owner>>::bits_type>;

// Whether Owner is an owner a view of reading R may refer into; a bitset is committed to neither reading.
template<class Owner, class R>
concept owner_reading =
        reading_tag<R> and
        requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; } and
        (std::same_as<typename owned_storage<std::remove_const_t<Owner>>::reads, R> or std::same_as<typename owned_storage<std::remove_const_t<Owner>>::reads, bitset_reading_tag>);

// Whether a view of reading R over Bits can refer into Owner: same storage, const flowing owner to view.
template<class Owner, class Bits, class R>
concept owner_of =
        owner_reading<Owner, R> and
        std::same_as<typename owned_storage<std::remove_const_t<Owner>>::bits_type, std::remove_const_t<Bits>> and
        (std::is_const_v<Bits> or not std::is_const_v<Owner>);

} // namespace xstd

#endif // XSTD_BITS_DETAIL_OWNERSHIP_HPP
