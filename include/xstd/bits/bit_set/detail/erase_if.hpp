//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_DETAIL_ERASE_IF_HPP
#define XSTD_BITS_BIT_SET_DETAIL_ERASE_IF_HPP

#include <xstd/bits/bit_set/detail/set_adaptor.hpp> // set_adaptor
#include <xstd/bits/detail/ownership.hpp>           // storage

namespace xstd::bits::detail {

// [set.erasure], over the set's own erase, found by argument-dependent lookup as std::erase_if is not.
template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare, class Predicate>
constexpr auto erase_if(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>& c, Predicate pred)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::size_type
{
        auto const original_size = c.size();
        for (auto i = c.begin(), last = c.end(); i != last;) {
                if (pred(*i)) {
                        i = c.erase(i);
                } else {
                        ++i;
                }
        }
        return original_size - c.size();
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_SET_DETAIL_ERASE_IF_HPP
