//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_FOR_EACH_TYPE_HPP
#define TEST_FOR_EACH_TYPE_HPP

#include <boost/core/demangle.hpp>  // demangle
#include <boost/test/unit_test.hpp> // BOOST_TEST_CONTEXT
#include <string>                   // string
#include <tuple>                    // tuple
#include <type_traits>              // type_identity
#include <typeinfo>                 // typeid

namespace test {

template<class T>
[[nodiscard]] auto type_name()
        -> std::string
{
        return boost::core::demangle(typeid(T).name());
}

namespace detail {

template<class... Types>
auto for_each_type(std::type_identity<std::tuple<Types...>>, auto fun)
        -> void
{
        (
                [&] -> void {
                        BOOST_TEST_CONTEXT("type: " << type_name<Types>())
                        {
                                fun.template operator()<Types>();
                        }
                }(),
                ...);
}

} // namespace detail

// fun called with each type of a tuple in turn, under a context that names the type a failure was on.
template<class List>
auto for_each_type(auto fun)
        -> void
{
        detail::for_each_type(std::type_identity<List>(), fun);
}

} // namespace test

#endif // TEST_FOR_EACH_TYPE_HPP
