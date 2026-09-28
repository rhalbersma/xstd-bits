//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/reference.hpp>       // proxy_reference
#include <test/spec/sequence.hpp>   // inplace_vector_all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // same_as

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Data)

// [inplace.vector.data]: constexpr T* data() noexcept; constexpr const T* data() const noexcept;
BOOST_AUTO_TEST_CASE(Data)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // A proxy has no element to point at, so only a real reference is asked for data().
                static_assert(test::proxy_reference<T> or requires (T c, T const cc) {
                        { c.data() } -> std::same_as<bool*>;
                        { cc.data() } -> std::same_as<bool const*>;
                });
                BOOST_CHECK(true);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
