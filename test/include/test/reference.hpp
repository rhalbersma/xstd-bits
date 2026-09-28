//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_REFERENCE_HPP
#define TEST_REFERENCE_HPP

#include <test/value_reference.hpp> // value_reference
#include <concepts>                 // convertible_to, same_as

namespace test {

// [container.reqmts]/4-5 as written: an element is an object, reached through value_type& and const value_type&.
template<class X>
concept real_reference = std::same_as<typename X::reference, typename X::value_type&> and std::same_as<typename X::const_reference, typename X::value_type const&>;

// [vector.bool.pspc]'s relaxation of real_reference: a class stands in for an element that has no address.
template<class X>
concept proxy_reference = not real_reference<X> and std::convertible_to<typename X::reference, typename X::value_type> and (std::same_as<typename X::const_reference, typename X::value_type> or value_reference<typename X::const_reference>);

} // namespace test

#endif // TEST_REFERENCE_HPP
