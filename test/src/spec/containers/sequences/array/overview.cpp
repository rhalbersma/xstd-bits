//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/reference.hpp>           // proxy_reference
#include <test/sequence/factory.hpp>    // make_sequence, model_of, stripes
#include <test/sequence/primitives.hpp> // iterates_as_a_constant
#include <test/spec/sequence.hpp>       // array_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                     // same_as
#include <iterator>                     // reverse_iterator
#include <ranges>                       // bidirectional_range, contiguous_range, random_access_range
#include <vector>                       // vector

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
                static_assert(std::ranges::contiguous_range<T> or (test::proxy_reference<T> and std::ranges::random_access_range<T>)); // [array.overview]/1
                auto const a = make_sequence<T>(T().size(), stripes);
                auto b = a;
                b = T();
                BOOST_CHECK_EQUAL(b.size(), a.size()); // [array.overview]/1
        });
}

// [array.overview]/2: list-initialization with up to N elements
BOOST_AUTO_TEST_CASE(ListInitialization)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // What is listed leads, each in its place, and every position after it is value-initialized.
                constexpr auto N = T().size();
                auto const none = std::vector<bool>(N, false);
                BOOST_CHECK(model_of(T{}) == none); // [array.overview]/2
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

// [array.overview]/3: a container and a reversible container, but not empty when default constructed
BOOST_AUTO_TEST_CASE(ContainerRequirements)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(std::ranges::bidirectional_range<T> and std::same_as<typename T::reverse_iterator, std::reverse_iterator<typename T::iterator>>); // [array.overview]/3
                auto const u = T();
                BOOST_CHECK_EQUAL(u.size(), T().max_size());   // [array.overview]/3
                BOOST_CHECK_EQUAL(u.empty(), u.size() == 0UZ); // [array.overview]/3
        });
}

// [array.overview]/5: iterator and const_iterator are constexpr iterators
BOOST_AUTO_TEST_CASE(ConstexprIterators)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(iterates_as_a_constant<T>()); // [array.overview]/5
                BOOST_CHECK(iterates_as_a_constant<T>());   // [array.overview]/5
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
