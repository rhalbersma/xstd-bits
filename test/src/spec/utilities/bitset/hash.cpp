//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // op_hash
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, pairs
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(Hash)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

// [bitset.hash]/1: template<size_t N> struct hash<bitset<N>>;
BOOST_AUTO_TEST_CASE(Hash)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // Every candidate with a std::hash is checked, and one without passes vacuously.
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_hash()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
