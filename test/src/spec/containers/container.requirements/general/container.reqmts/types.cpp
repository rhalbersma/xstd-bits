//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/primitives.hpp> // nested_types
#include <test/set/exhaustive.hpp>      // L1, all_cardinality_sets, all_singleton_sets, limit_v
#include <test/set/primitives.hpp>      // mem_const_reference, nested_types
#include <test/spec/random.hpp>         // all_sets
#include <test/spec/sequence.hpp>       // every_width
#include <test/spec/set.hpp>            // boundary_widths, every_width, random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL_COLLECTIONS
#include <algorithm>                    // copy
#include <array>                        // array
#include <cstddef>                      // size_t
#include <iterator>                     // inserter
#include <ranges>                       // filter, to
#include <set>                          // set

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Types)

using namespace test;
using namespace test::set;

namespace {

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

} // namespace

// [container.reqmts]/2-9: the nested types
BOOST_AUTO_TEST_SUITE(NestedTypes)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldForEverySet, T, test::spec::set::every_width)
{
        [[maybe_unused]] auto const _ = nested_types<T>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldForEverySequence, T, test::spec::sequence::every_width)
{
        [[maybe_unused]] auto const _ = sequence::nested_types<T>();
}

// A dereferenced iterator is a const_reference to a key the set holds.
BOOST_AUTO_TEST_CASE_TEMPLATE(ASetsConstReferenceIsAKeyOverEveryCardinalityAndSingleton, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_const_reference());
        on1::all_singleton_sets<T>(mem_const_reference());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ASetsConstReferenceIsAKeyOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(mem_const_reference());
}

// A reference converts to the key and no further, so a key type that converts from it takes one conversion.
BOOST_AUTO_TEST_CASE_TEMPLATE(ASetsKeysCopyIntoASetOfAnImplicitlyConstructibleType, T, test::spec::set::every_width)
{
        constexpr auto primes = std::array{2UZ, 3UZ, 5UZ, 7UZ, 11UZ, 13UZ, 17UZ, 19UZ, 23UZ, 29UZ, 31UZ};
        auto const src = primes | std::views::filter([](auto p) -> bool { return p < limit_v<T, L1>; }) | std::ranges::to<T>();
        std::set<Implicit> dst;
        std::ranges::copy(src, std::inserter(dst, dst.end()));
        BOOST_CHECK_EQUAL_COLLECTIONS(src.begin(), src.end(), dst.begin(), dst.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ASetsKeysCopyIntoASetOfAnImplicitlyConstructibleTypeOverRandomSets, T, test::spec::set::random_widths)
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
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
