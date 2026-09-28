//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/concepts.hpp>    // set_size_t, set_size_t_ranges
#include <test/spec/set.hpp>        // all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstddef>                  // size_t
#include <set>                      // set

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Overview)

// [set.overview]/1-3: template<class Key, class Compare = less<Key>, class Allocator = allocator<Key>> class set;
BOOST_AUTO_TEST_CASE(Set)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // The model is held to the synopsis first, so a line it fails is the checklist's error, not ours.
                static_assert(test::set::set_size_t<std::set<std::size_t>>);
                static_assert(test::set::set_size_t<T>);
#ifdef __cpp_lib_containers_ranges

                static_assert(test::set::set_size_t_ranges<std::set<std::size_t>>);
                static_assert(test::set::set_size_t_ranges<T>);

#endif
                BOOST_CHECK(true);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
