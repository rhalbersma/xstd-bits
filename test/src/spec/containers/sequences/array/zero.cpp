//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/sequence.hpp>   // array_all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <utility>                  // as_const

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Zero)

// [array.zero]/1-3: array<T, 0>
BOOST_AUTO_TEST_CASE(ZeroSized)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                if constexpr (T().size() == 0UZ) {
                        auto a = T();
                        auto b = T();
                        BOOST_CHECK_EQUAL(a.size(), 0UZ);                                                         // [array.zero]/1
                        BOOST_CHECK(a.begin() == a.end() and std::as_const(a).begin() == std::as_const(a).end()); // [array.zero]/2
                        static_assert(noexcept(a.swap(b)));                                                       // [array.zero]/3
                        a.swap(b);
                        BOOST_CHECK(a == b);
                } else {
                        auto const a = T();
                        BOOST_CHECK(a.begin() != a.end());
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
