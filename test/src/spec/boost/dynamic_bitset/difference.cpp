//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // mem_bit_minus_assign, op_bit_minus
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, pairs
#include <test/spec/input.hpp>        // context, on_copy
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Difference)

using namespace test::bitset;
using test::spec::context;
using test::spec::on_copy;
namespace inputs = test::spec::bitset::inputs;

// boost::dynamic_bitset: dynamic_bitset& operator-=(const dynamic_bitset& b);
BOOST_AUTO_TEST_CASE(MinusAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // std::bitset has no set difference, so it passes vacuously and every candidate with one is checked.
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        on_copy(mem_bit_minus_assign(), a, b);
                }
        });
}

// boost::dynamic_bitset: dynamic_bitset operator-(const dynamic_bitset& a, const dynamic_bitset& b);
BOOST_AUTO_TEST_CASE(Minus)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_bit_minus()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
