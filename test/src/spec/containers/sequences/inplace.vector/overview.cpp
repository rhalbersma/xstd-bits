//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/reference.hpp>           // proxy_reference
#include <test/sequence/primitives.hpp> // iterates_as_a_constant
#include <test/spec/container.hpp>      // constant_evaluable_v
#include <test/spec/sequence.hpp>       // inplace_vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <ranges>                       // contiguous_range, random_access_range

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Overview)

// [inplace.vector.overview]/1-2: template<class T, size_t N> class inplace_vector;
BOOST_AUTO_TEST_CASE(InplaceVector)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // A contiguous container, which a proxy relaxes to random access: a bit has no address to lie at.
                static_assert(std::ranges::contiguous_range<T> or (test::proxy_reference<T> and std::ranges::random_access_range<T>)); // [inplace.vector.overview]/1
                static_assert(not requires { typename T::allocator_type; });                                                           // [inplace.vector.overview]/1

                // A sequence container with its optional operations, save those at the front.
                static_assert(requires (T c, bool b) { // [inplace.vector.overview]/2
                        c.front();
                        c.back();
                        c.emplace_back(b);
                        c.push_back(b);
                        c.pop_back();
                        c[0UZ];
                        c.at(0UZ);
                });
                static_assert(not requires (T c, bool b) { c.push_front(b); });    // [inplace.vector.overview]/2
                static_assert(not requires (T c, bool b) { c.emplace_front(b); }); // [inplace.vector.overview]/2
                static_assert(not requires (T c) { c.pop_front(); });              // [inplace.vector.overview]/2
                BOOST_CHECK_EQUAL(T().capacity(), T::capacity());
        });
}

// [inplace.vector.overview]/3: iterator and const_iterator are constexpr iterators
BOOST_AUTO_TEST_CASE(ConstexprIterators)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // Before C++26 the bounded columns hold their blocks in Boost's static_vector, which is not constexpr.
                if constexpr (test::spec::container::constant_evaluable_v<T>) {
                        static_assert(test::sequence::iterates_as_a_constant<T>()); // [inplace.vector.overview]/3
                }
                BOOST_CHECK(test::sequence::iterates_as_a_constant<T>());
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
