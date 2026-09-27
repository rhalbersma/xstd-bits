//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/composable.hpp>  // decrement_modulo, increment_modulo
#include <test/set/exhaustive.hpp>  // all_singleton_sets, all_valid, static_capacity
#include <test/spec/random.hpp>     // all_set_key_pairs
#include <test/spec/set.hpp>        // boundary_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                  // size_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Shift)

using namespace test;
using namespace test::set;

// A shift adds or subtracts n from every key and drops what leaves the width; a set without one is skipped.
BOOST_AUTO_TEST_CASE_TEMPLATE(EverySingletonShiftsByEveryDistance, T, test::spec::set::boundary_widths)
{
        // A left shift grows a run-time width, which past a bounded set's capacity throws rather than drops the keys.
        if constexpr (not static_capacity<T>) {
                on1::all_valid<T>([](auto pos) {
                        on1::all_singleton_sets<T>([&](auto const& bs1) {
                                composable::increment_modulo()(bs1, static_cast<std::size_t>(pos));
                        });
                });
        }
        on1::all_valid<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        composable::decrement_modulo()(bs1, static_cast<std::size_t>(pos));
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(RandomSetsShiftByRandomDistances, T, test::spec::set::random_widths)
{
        spec::random::all_set_key_pairs<T>([](auto const& a, auto n) {
                if constexpr (not static_capacity<T>) {
                        composable::increment_modulo()(a, n);
                }
                composable::decrement_modulo()(a, n);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
