//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // mem_intersects
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, pairs
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Intersects)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

// boost::dynamic_bitset: bool intersects(const dynamic_bitset& b) const;
BOOST_AUTO_TEST_CASE(Intersects)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // std::bitset has no intersects, so it is answered bit by bit, and the member checked against that.
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_intersects()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
