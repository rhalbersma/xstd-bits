//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/block_types.hpp>       // graded_extents
#include <test/sequence/concepts.hpp> // bit_sequence
#include <test/value_reference.hpp>   // value_reference
#include <xstd/bits/bit_array.hpp>    // bit_array
#include <concepts>                   // regular, totally_ordered
#include <functional>                 // hash
#include <iterator>                   // random_access_iterator
#include <ranges>                     // random_access_range

BOOST_AUTO_TEST_SUITE(BitArray)

// Every Block model within one block and the narrow ones across boundaries; the grading is in test/block_types.hpp.
using Types = test::graded_extents<xstd::basic_bit_array>;

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

// Every owner hashes, this one although std::array<bool, N> does not: equal values equal, at every extent. [design.md#the-hashing-invariant]
BOOST_AUTO_TEST_CASE_TEMPLATE(ItHashesAsAnOwner, T, Types)
{
        auto const h = std::hash<T>();
        BOOST_CHECK_EQUAL(h(T()), h(T()));
        if constexpr (T().size() > 0UZ) {
                auto x = T();
                x[0] = true;
                BOOST_CHECK(h(x) != h(T()));
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsABitSequence, T, Types)
{
        static_assert(test::sequence::bit_sequence<T>);
}

BOOST_AUTO_TEST_SUITE_END()
