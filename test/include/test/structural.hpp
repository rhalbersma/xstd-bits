//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_STRUCTURAL_HPP
#define TEST_STRUCTURAL_HPP

#include <concepts> // default_initializable

namespace test {

// A class template taking any value, which a type's value can be an argument to only if that type is structural.
template<auto V>
struct value_parameter
{};

// No standard trait asks [temp.param]/7, so this forms the argument; only for a type whose T{} is a constant.
template<class T>
concept structural = std::default_initializable<T> and requires { typename value_parameter<T{}>; };

} // namespace test

#endif // TEST_STRUCTURAL_HPP
