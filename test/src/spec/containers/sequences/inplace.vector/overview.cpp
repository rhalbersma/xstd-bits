//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/reference.hpp>       // proxy_reference
#include <test/spec/sequence.hpp>   // inplace_vector_all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <ranges>                   // contiguous_range, random_access_range

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Overview)

// [inplace.vector.overview]/1-5: template<class T, size_t N> class inplace_vector;
BOOST_AUTO_TEST_CASE(InplaceVector)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // A contiguous container, which a proxy relaxes to random access: a bit has no address to lie at.
                static_assert(std::ranges::contiguous_range<T> or (test::proxy_reference<T> and std::ranges::random_access_range<T>)); // [inplace.vector.overview]/1
                BOOST_CHECK(true);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
