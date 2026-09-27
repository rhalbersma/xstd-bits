//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // L1, all_cardinality_sets, all_singleton_set_pairs, all_singleton_sets, empty_set_pair, limit_v
#include <test/set/primitives.hpp>  // fn_swap, mem_const_iterator, mem_const_reference, mem_empty, mem_max_size, mem_size, mem_swap, nested_types, op_equal_to, op_not_equal_to
#include <test/spec/random.hpp>     // all_set_pairs, all_sets
#include <test/spec/set.hpp>        // boundary_widths, every_width, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL_COLLECTIONS
#include <algorithm>                // copy
#include <array>                    // array
#include <cstddef>                  // size_t
#include <iterator>                 // inserter
#include <ranges>                   // filter, to
#include <set>                      // set
#include <utility>                  // as_const

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Container)
BOOST_AUTO_TEST_SUITE(Reqmts)

using namespace test;
using namespace test::set;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheContainerRequirementsHoldOnAnEmptyPair, T, test::spec::set::every_width)
{
        [[maybe_unused]] auto const _ = nested_types<T>();

        on0::empty_set_pair<T>(mem_swap());
        on0::empty_set_pair<T>(fn_swap());

        on0::empty_set_pair<T>(op_equal_to());
        on0::empty_set_pair<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheIteratorsAndObserversHoldOverEveryCardinalityAndSingleton, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_const_reference());
        on1::all_singleton_sets<T>(mem_const_reference());

        on1::all_cardinality_sets<T>([](auto& is) {
                mem_const_iterator()(is);
        });
        on1::all_cardinality_sets<T>([](auto const& is) {
                mem_const_iterator()(is);
        });
        on1::all_singleton_sets<T>([](auto& is1) {
                mem_const_iterator()(is1);
        });
        on1::all_singleton_sets<T>([](auto const& is1) {
                mem_const_iterator()(is1);
        });

        on1::all_cardinality_sets<T>(mem_empty());
        on1::all_cardinality_sets<T>(mem_size());
        on1::all_cardinality_sets<T>(mem_max_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EqualityAndSwapHoldOverEverySingletonPair, T, test::spec::set::boundary_widths)
{
        on2::all_singleton_set_pairs<T>(op_equal_to());
        on2::all_singleton_set_pairs<T>(op_not_equal_to());
        on2::all_singleton_set_pairs<T>(mem_swap());
        on2::all_singleton_set_pairs<T>(fn_swap());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheContainerRequirementsHoldOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>([](auto& is) {
                mem_const_reference()(is);
                mem_const_iterator()(is);
                mem_const_iterator()(std::as_const(is));
                mem_empty()(is);
                mem_size()(is);
                mem_max_size()(is);
        });
        spec::random::all_set_pairs<T>([](auto& a, auto& b) {
                op_equal_to()(a, b);
                op_not_equal_to()(a, b);
                mem_swap()(a, b);
                fn_swap()(a, b);
        });
}

class Implicit
{
        std::size_t m_value;

public:
        [[nodiscard]] constexpr explicit(false) Implicit(std::size_t v) noexcept
                : m_value(v)
        {}

        // Implicit is the point: this class exists to convert both ways without a cast.
        [[nodiscard]] constexpr explicit(false) operator std::size_t() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return m_value;
        }
};

// A reference converts to the key and no further, so a key type that converts from it takes one conversion.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheKeysCopyIntoASetOfAnImplicitlyConstructibleType, T, test::spec::set::every_width)
{
        constexpr auto primes = std::array{2UZ, 3UZ, 5UZ, 7UZ, 11UZ, 13UZ, 17UZ, 19UZ, 23UZ, 29UZ, 31UZ};
        auto const src = primes | std::views::filter([](auto p) -> bool { return p < limit_v<T, L1>; }) | std::ranges::to<T>();
        std::set<Implicit> dst;
        std::ranges::copy(src, std::inserter(dst, dst.end()));
        BOOST_CHECK_EQUAL_COLLECTIONS(src.begin(), src.end(), dst.begin(), dst.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheKeysOfRandomSetsCopyIntoASetOfAnImplicitlyConstructibleType, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>([](auto const& src) {
                std::set<Implicit> dst;
                std::ranges::copy(src, std::inserter(dst, dst.end()));
                BOOST_CHECK_EQUAL_COLLECTIONS(src.begin(), src.end(), dst.begin(), dst.end());
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
