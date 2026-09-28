//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // mem_is_proper_subset_of, mem_is_proper_subset_of_edges, mem_is_subset_of
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, bitsets, pairs_with_doubletons
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Subset)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

// boost::dynamic_bitset: bool is_subset_of(const dynamic_bitset& a) const;
BOOST_AUTO_TEST_CASE(IsSubsetOf)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // std::bitset has no subset test, so it is answered bit by bit, and the member is checked against that.
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_is_subset_of()(a, b);
                }
        });
}

// boost::dynamic_bitset: bool is_proper_subset_of(const dynamic_bitset& a) const;
BOOST_AUTO_TEST_CASE(IsProperSubsetOf)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_is_proper_subset_of()(a, b);
                }
                // Each bitset is also the base of four edges, which then differ in its first and last blocks.
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        auto x = a;
                        mem_is_proper_subset_of_edges()(x, x);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
