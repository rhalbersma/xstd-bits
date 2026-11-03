//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SEQUENCE_ERASURE_HPP
#define XSTD_BITS_DETAIL_SEQUENCE_ERASURE_HPP

#include <xstd/bits/detail/ownership.hpp>        // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp> // sequence_adaptor
#include <algorithm>                             // remove_if
#include <cstddef>                               // size_t

namespace xstd::bits::detail {

// [vector.erasure], over the owner's own erase: the proxies move and swap, so remove_if runs unchanged.
template<class Bits, storage Store, window W, class Derived, std::size_t E, class Pred>
constexpr auto erase_if(sequence_adaptor<Bits, Store, W, Derived, E>& c, Pred pred)
        -> sequence_adaptor<Bits, Store, W, Derived, E>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        // [vector.erasure]'s classic remove_if asks pred of *it alone; ranges::remove_if also asks it of bool&.
        auto const first = std::remove_if(c.begin(), c.end(), pred);
        auto const n     = static_cast<decltype(c.size())>(c.end() - first);
        c.erase(first, c.end());
        return n;
}

template<class Bits, storage Store, window W, class Derived, std::size_t E, class U = bool>
constexpr auto erase(sequence_adaptor<Bits, Store, W, Derived, E>& c, U const& value)
        -> sequence_adaptor<Bits, Store, W, Derived, E>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        return xstd::bits::detail::erase_if(c, [&](bool x) -> bool { return x == value; });
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_SEQUENCE_ERASURE_HPP
