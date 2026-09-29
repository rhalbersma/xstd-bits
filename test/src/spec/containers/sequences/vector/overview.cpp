//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // iterates_as_a_constant
#include <test/spec/container.hpp>      // constant_evaluable_v
#include <test/spec/sequence.hpp>       // vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <ranges>                       // random_access_range

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Overview)

// [vector.overview]/2: template<class T, class Allocator = allocator<T>> class vector;
BOOST_AUTO_TEST_CASE(Vector)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                // Contiguous only for an element type other than bool, so a vector of bool is asked for random access.
                static_assert(std::ranges::random_access_range<T> and requires { typename T::allocator_type; }); // [vector.overview]/2
                BOOST_CHECK(T().empty());
        });
}

// [vector.overview]/3: iterator and const_iterator are constexpr iterators
BOOST_AUTO_TEST_CASE(ConstexprIterators)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                // Boost's small_vector, which the small columns hold their blocks in, is not constant-evaluable.
                if constexpr (test::spec::container::constant_evaluable_v<T>) {
                        static_assert(test::sequence::iterates_as_a_constant<T>()); // [vector.overview]/3
                }
                BOOST_CHECK(test::sequence::iterates_as_a_constant<T>());
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
