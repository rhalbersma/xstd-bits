//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_REBIND_HPP
#define XSTD_BITS_DETAIL_REBIND_HPP

#include <concepts>    // same_as
#include <cstddef>     // size_t
#include <type_traits> // conditional_t, is_const_v, is_volatile_v, remove_cv_t

namespace xstd::bits::detail {

// An owner's block, and the same owner over another; one whose width is a template argument names it and resizes too.
template<class Owner>
struct rebind
{};

// U qualified as T is, so a cv-qualified owner transforms into one qualified alike, as make_unsigned's cv integer does.
template<class T, class U>
using cv_as_t = std::conditional_t<std::is_const_v<T>, std::conditional_t<std::is_volatile_v<T>, U const volatile, U const>, std::conditional_t<std::is_volatile_v<T>, U volatile, U>>;

// cv-transparent: asked of the owner with its cv stripped, same_as guarding the defaulted parameter.
template<class Owner_cv, class Owner = std::remove_cv_t<Owner_cv>>
concept rebindable =
        std::same_as<Owner, std::remove_cv_t<Owner_cv>> and
        requires {
                typename rebind<Owner>::block_type;
                typename rebind<Owner>::template with_block<typename rebind<Owner>::block_type>;
        };

template<class Owner_cv, class Owner = std::remove_cv_t<Owner_cv>>
concept resizable =
        std::same_as<Owner, std::remove_cv_t<Owner_cv>> and rebindable<Owner> and
        requires {
                rebind<Owner>::width;
                typename rebind<Owner>::template with_width<rebind<Owner>::width>;
        };

template<rebindable Owner>
using rebind_block_t = rebind<std::remove_cv_t<Owner>>::block_type;

template<resizable Owner>
inline constexpr std::size_t rebind_width_v = rebind<std::remove_cv_t<Owner>>::width;

// The owner over another block, or another width, keeping its own cv; a block is held by value, so its cv is dropped.
template<rebindable Owner, class Block>
using with_block_t = cv_as_t<Owner, typename rebind<std::remove_cv_t<Owner>>::template with_block<std::remove_cv_t<Block>>>;

template<resizable Owner, std::size_t N>
using with_width_t = cv_as_t<Owner, typename rebind<std::remove_cv_t<Owner>>::template with_width<N>>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_REBIND_HPP
