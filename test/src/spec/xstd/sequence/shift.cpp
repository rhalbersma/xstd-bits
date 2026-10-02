//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/rejection.hpp>  // has_shift_left, has_shift_left_assign, has_shift_right, has_shift_right_assign
#include <test/spec/sequence.hpp>   // all
#include <test/spec/span.hpp>       // all
#include <xstd/bits/bitset.hpp>     // bitset
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Sequence)
BOOST_AUTO_TEST_SUITE(Shift)

namespace {

// No sequence shifts, by this library's design: std::shift_right already names what a sequence's << would do.
template<class X>
auto check_no_left_shift()
        -> void
{
        static_assert(not test::spec::has_shift_left_assign<X>);
        static_assert(not test::spec::has_shift_left<X>);
}

template<class X>
auto check_no_right_shift()
        -> void
{
        static_assert(not test::spec::has_shift_right_assign<X>);
        static_assert(not test::spec::has_shift_right<X>);
}

} // namespace

// xstd sequence: X& operator<<=(size_t n); and X operator<<(const X& lhs, size_t n); are not declared
BOOST_AUTO_TEST_CASE(ShiftLeft)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void { check_no_left_shift<T>(); });
        test::for_each_type<test::spec::span::all>([]<class T> -> void { check_no_left_shift<T>(); });
        // The bitset reading over the same storage shifts, so a misspelled operator cannot pass the checks above.
        static_assert(test::spec::has_shift_left_assign<xstd::bitset<17>> and test::spec::has_shift_left<xstd::bitset<17>>);
        BOOST_CHECK(true);
}

// xstd sequence: X& operator>>=(size_t n); and X operator>>(const X& lhs, size_t n); are not declared
BOOST_AUTO_TEST_CASE(ShiftRight)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void { check_no_right_shift<T>(); });
        test::for_each_type<test::spec::span::all>([]<class T> -> void { check_no_right_shift<T>(); });
        static_assert(test::spec::has_shift_right_assign<xstd::bitset<17>> and test::spec::has_shift_right<xstd::bitset<17>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
