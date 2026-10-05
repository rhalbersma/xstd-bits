//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_CLOSED_PROXY_HPP
#define TEST_CLOSED_PROXY_HPP

#include <concepts> // same_as
#include <iterator> // iter_reference_t
#include <utility>  // declval

namespace test {

// * and & take an iterator and its proxy into each other, and the proxy is the reference the iterator names.
template<class I>
concept closed_proxy =
        std::same_as<decltype(&*std::declval<I const&>()), I> and
        std::same_as<decltype(*&std::declval<std::iter_reference_t<I> const&>()), std::iter_reference_t<I>> and
        std::same_as<std::iter_reference_t<I>, typename I::reference>;

// Both constnesses of a container's iterator, closed alike.
template<class C>
concept closed_proxies = closed_proxy<typename C::iterator> and closed_proxy<typename C::const_iterator>;

} // namespace test

#endif // TEST_CLOSED_PROXY_HPP
