//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_OWNERSHIP_HPP
#define XSTD_BITS_DETAIL_OWNERSHIP_HPP

#include <concepts>    // derived_from, same_as
#include <cstddef>     // size_t
#include <type_traits> // conditional_t, is_const_v, remove_const_t

namespace xstd::bits::detail {

// Named for what is owned rather than for the owner: borrowed is the word std::ranges::enable_borrowed_range uses.
enum class storage : bool
{
        owned,
        borrowed,
};

[[nodiscard]] constexpr auto owns(storage s) noexcept
        -> bool
{
        return s == storage::owned;
}

// Named for how much of the storage a view reaches: all of it, or the sub-range a subspan was cut down to.
enum class window : bool
{
        all,
        sub,
};

// The two ways the same blocks are read: as a set of keys, or as a sequence of bools.
struct set_reading_tag
{};

struct sequence_reading_tag
{};

// What an owner wraps: declared, never defined, so a view over a type that owns nothing is unsatisfied.
template<class Owner>
struct owned_storage;

// A container answers as the vehicle it derives from, which is the one that knows what it wraps.
template<class Owner>
        requires (not std::same_as<Owner, typename Owner::adaptor_type>) and std::derived_from<Owner, typename Owner::adaptor_type> and requires { typename owned_storage<typename Owner::adaptor_type>::bits_type; }
struct owned_storage<Owner> : owned_storage<typename Owner::adaptor_type>
{};

// One of our owners, of any reading and const or not: its storage is named by the owner protocol.
template<class T>
concept owner = requires { typename owned_storage<std::remove_const_t<T>>::bits_type; };

// One of our views, a window included: built on an adaptor, over storage it does not own.
template<class T>
concept view = (not owner<T>) and requires { typename T::adapted_type; };

// The storage a view over an owner refers to, const where the owner is.
template<class Owner>
using owned_bits_t = std::conditional_t<std::is_const_v<Owner>, typename owned_storage<std::remove_const_t<Owner>>::bits_type const, typename owned_storage<std::remove_const_t<Owner>>::bits_type>;

// The storage under a reading, for the library's free functions: both adaptors befriend this, and nothing else does.
struct storage_access
{
        template<class Reading>
        [[nodiscard]] static constexpr auto bits(Reading const& r) noexcept
                -> auto const&
        {
                return static_cast<Reading::adaptor_type const&>(r).bits();
        }

        template<class Reading>
        [[nodiscard]] static constexpr auto bits(Reading& r) noexcept
                -> auto&
        {
                return static_cast<Reading::adaptor_type&>(r).bits();
        }

        // Where a window's position zero sits in that storage.
        template<class Reading>
        [[nodiscard]] static constexpr auto offset(Reading const& r) noexcept
                -> std::size_t
        {
                return static_cast<Reading::adaptor_type const&>(r).offset();
        }
};

// Whether a container, owner or view, reads its blocks as R: a refinement of R answers for R.
template<class T, class R>
concept reads = std::derived_from<typename std::remove_const_t<T>::reads_as, R>;

// Whether a view of reading R may refer into Owner.
template<class Owner, class R>
concept owner_reading = owner<Owner> and reads<Owner, R>;

// Whether a view of reading R over Bits can refer into Owner: same storage, const flowing owner to view.
template<class Owner, class Bits, class R>
concept owner_of =
        owner_reading<Owner, R> and
        std::same_as<typename owned_storage<std::remove_const_t<Owner>>::bits_type, std::remove_const_t<Bits>> and
        (std::is_const_v<Bits> or not std::is_const_v<Owner>);

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_OWNERSHIP_HPP
