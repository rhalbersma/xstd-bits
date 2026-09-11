//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>         // graded_extents
#include <test/set/ascending.hpp>       // yields_ascending_keys
#include <test/set/concepts.hpp>        // bit_set
#include <test/value_reference.hpp>     // value_reference
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                     // regular, totally_ordered
#include <cstddef>                      // size_t
#include <iterator>                     // bidirectional_iterator
#include <ranges>                       // bidirectional_range, iota, to

BOOST_AUTO_TEST_SUITE(BitFiniteSet)

// Every Block model within one block and the narrow ones across boundaries; the grading is in test/block_types.hpp.
using Types = test::graded_extents<xstd::basic_bit_static_set>;

// The clauses one at a time, so a failure names which one; the umbrella asserts the composite.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsTotallyOrdered, T, Types)
{
        static_assert(std::totally_ordered<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsABidirectionalRange, T, Types)
{
        static_assert(std::ranges::bidirectional_range<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsIteratorIsBidirectional, T, Types)
{
        using I = T::iterator;
        static_assert(std::bidirectional_iterator<I>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsConstReferenceIsAValue, T, Types)
{
        static_assert(test::value_reference<typename T::const_reference>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsABitSet, T, Types)
{
        static_assert(test::set::bit_set<T>);
}

// Total lookups, swept over every width because no single one exposed all six operations. [design.md#total-lookups-on-the-container]
template<class X>
auto check_key_outside_the_domain(X a, std::size_t x)
        -> void
{
        auto const original = a;

        BOOST_CHECK(not a.contains(x));
        BOOST_CHECK_EQUAL(a.count(x), 0UZ);
        BOOST_CHECK(a.find(x)        == a.end());
        BOOST_CHECK(a.lower_bound(x) == a.end());
        BOOST_CHECK(a.upper_bound(x) == a.end());

        auto const [ first, last ] = a.equal_range(x);
        BOOST_CHECK(first == a.end());
        BOOST_CHECK(last  == a.end());

        // A no-op that must stay one: this is the write.
        BOOST_CHECK_EQUAL(a.erase(x), 0UZ);
        BOOST_CHECK(a == original);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(LookupIsTotalOverKeyType, T, Types)
{
        auto const N = T().max_size();
        auto const full = std::views::iota(0UZ, N) | std::ranges::to<T>();

        // Just past the end, past the last block, and the value that would wrap any n + 1.
        for (auto const x : { N, N + 1, (2 * N) + 1, static_cast<std::size_t>(-1) }) {
                check_key_outside_the_domain(T(), x);
                check_key_outside_the_domain(full, x);
        }
}

// Ascending keys, at every width and whatever the insertion order: what makes this a set rather than a bag of
// positions. [design.md#two-readings-disagree]
BOOST_AUTO_TEST_CASE_TEMPLATE(ItYieldsAscendingKeys, T, Types)
{
        auto c = T();
        test::set::yields_ascending_keys(c);            // empty is trivially ascending

        // Inserted high to low, and across block boundaries where the width allows, so the ascending answer is
        // the container's doing and not the insertion order's.
        for (auto const key : { 70UZ, 64UZ, 63UZ, 9UZ, 1UZ, 0UZ }) {
                if (key < c.max_size()) {
                        c.insert(key);
                }
        }
        test::set::yields_ascending_keys(c);
}

BOOST_AUTO_TEST_SUITE_END()
