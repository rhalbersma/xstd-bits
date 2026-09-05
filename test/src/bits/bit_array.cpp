//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/door.hpp>       // door_disagreements
#include <test/block_types.hpp>       // graded_extents
#include <test/sequence/concepts.hpp> // bit_sequence
#include <test/value_reference.hpp>   // value_reference
#include <xstd/bits/bit_array.hpp>    // bit_array
#include <xstd/bits/bit_traits.hpp>   // bit_storage, bit_traits, block_readable, static_bit_extent
#include <concepts>                   // regular, totally_ordered
#include <iterator>                   // random_access_iterator
#include <ranges>                     // random_access_range

BOOST_AUTO_TEST_SUITE(BitArray)

// Every Block model within one block and the narrow ones across boundaries; the grading is in test/block_types.hpp.
using Types = test::graded_extents<xstd::bit_array>;

// The clauses one at a time, so a failure names which one; the umbrella asserts the composite.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsTotallyOrdered, T, Types)
{
        static_assert(std::totally_ordered<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsARandomAccessRange, T, Types)
{
        static_assert(std::ranges::random_access_range<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsIteratorIsRandomAccess, T, Types)
{
        using I = T::iterator;
        static_assert(std::random_access_iterator<I>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsConstReferenceIsAValue, T, Types)
{
        static_assert(test::value_reference<typename T::const_reference>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsABitSequence, T, Types)
{
        static_assert(test::sequence::bit_sequence<T>);
}


// Every entry unwraps to the storage's own door, and none is silently dropped. [design.md#the-door]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorAdaptsIt, T, Types)
{
        using traits = xstd::bit_traits<T>;

        static_assert(xstd::bit_storage<T>);
        static_assert(xstd::static_bit_extent<T>);
        static_assert(xstd::block_readable<traits, T>);

        BOOST_CHECK_EQUAL(test::door_disagreements<T>(), 0);
}

// The position-dependent entries, at every position the width offers. [design.md#per-instantiation-slots]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorAnswersAtEveryPosition, T, Types)
{
        BOOST_CHECK_EQUAL(test::door_disagreements_at_positions<T>(), 0);
}

// Both orderings reach the storage's native block-wise members. [design.md#two-readings-disagree]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorNamesBothOrderings, T, Types)
{
        BOOST_CHECK_EQUAL(test::door_ordering_disagreements<T>(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
