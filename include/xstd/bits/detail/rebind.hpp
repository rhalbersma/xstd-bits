//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_REBIND_HPP
#define XSTD_BITS_DETAIL_REBIND_HPP

#include <cstddef> // size_t

namespace xstd::bits::detail {

// An owner's block, and the same owner over another; one whose width is a template argument names it and resizes too.
template<class Owner>
struct rebind
{};

template<class Owner>
concept rebindable = requires {
        typename rebind<Owner>::block_type;
        typename rebind<Owner>::template with_block<typename rebind<Owner>::block_type>;
};

template<class Owner>
concept resizable = rebindable<Owner> and requires {
        rebind<Owner>::width;
        typename rebind<Owner>::template with_width<rebind<Owner>::width>;
};

template<rebindable Owner>
using rebind_block_t = rebind<Owner>::block_type;

template<resizable Owner>
inline constexpr std::size_t rebind_width_v = rebind<Owner>::width;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_REBIND_HPP
