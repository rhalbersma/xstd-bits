//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // mem_compare_three_way
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, pairs_with_doubletons
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Ordering)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

// boost::dynamic_bitset: bool operator<(const dynamic_bitset& a, const dynamic_bitset& b);
BOOST_AUTO_TEST_CASE(Less)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // boost::dynamic_bitset's operator< orders the bit strings; std::bitset has none and passes vacuously.
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_compare_three_way()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
