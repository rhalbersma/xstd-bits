//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // all_cardinality_sets, all_singleton_sets
#include <test/set/primitives.hpp>  // fn_erase_if
#include <test/spec/random.hpp>     // all_sets
#include <test/spec/set.hpp>        // boundary_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                  // size_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Erasure)

using namespace test;
using namespace test::set;

namespace {

// Nothing, everything, and every other key, so a removal can be none, all, or interleaved with what stays.
auto const never = [](std::size_t) -> bool { return false; };
auto const always = [](std::size_t) -> bool { return true; };
auto const odd = [](std::size_t x) -> bool { return x % 2 == 1; };

} // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(EraseIfRemovesWhatItRemovesFromAStdSet, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>([](auto const& is) {
                fn_erase_if()(is, never);
                fn_erase_if()(is, always);
                fn_erase_if()(is, odd);
        });
        on1::all_singleton_sets<T>([](auto const& is1) {
                fn_erase_if()(is1, odd);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EraseIfRemovesWhatItRemovesFromAStdSetOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>([](auto const& is) {
                fn_erase_if()(is, never);
                fn_erase_if()(is, always);
                fn_erase_if()(is, odd);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
