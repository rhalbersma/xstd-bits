//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/reference.hpp>        // proxy_reference
#include <test/sequence/factory.hpp> // model_of
#include <test/spec/sequence.hpp>    // array_all
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <ranges>                    // contiguous_range, random_access_range
#include <vector>                    // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Overview)

using namespace test::sequence;

// [array.overview]/1: template<class T, size_t N> struct array;
BOOST_AUTO_TEST_CASE(Array)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // A contiguous container, which a proxy relaxes to random access: a bit has no address to lie at.
                static_assert(std::ranges::contiguous_range<T> or (test::proxy_reference<T> and std::ranges::random_access_range<T>));
                BOOST_CHECK(true);
        });
}

// [array.overview]/2: list-initialization with up to N elements
BOOST_AUTO_TEST_CASE(ListInitialization)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // What is listed leads, each in its place, and every position after it is value-initialized.
                constexpr auto N = T().size();
                auto const none = std::vector<bool>(N, false);
                BOOST_CHECK(model_of(T{}) == none);
                if constexpr (N >= 1UZ) {
                        auto m = none;
                        m[0] = true;
                        BOOST_CHECK(model_of(T{true}) == m);
                }
                if constexpr (N >= 3UZ) {
                        static_assert(requires (bool b) { T{b, b}; });
                        auto m = none;
                        m[0] = true;
                        m[2] = true;
                        BOOST_CHECK(model_of(T{true, false, true}) == m);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
