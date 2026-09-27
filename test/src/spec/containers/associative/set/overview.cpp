//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/concepts.hpp>    // set_size_t, set_size_t_ranges
#include <test/spec/set.hpp>        // every_width
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstddef>                  // size_t
#include <set>                      // set

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Overview)

// The model is held to the synopsis first, so a line it does not answer is the checklist's error and not ours.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnswersEveryLineOfTheSynopsis, T, test::spec::set::every_width)
{
        static_assert(test::set::set_size_t<std::set<std::size_t>>);
        static_assert(test::set::set_size_t<T>);
#ifdef __cpp_lib_containers_ranges

        static_assert(test::set::set_size_t_ranges<std::set<std::size_t>>);
        static_assert(test::set::set_size_t_ranges<T>);

#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
