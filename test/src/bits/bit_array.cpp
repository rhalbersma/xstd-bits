//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>       // graded_extents
#include <test/sequence/concepts.hpp> // bit_sequence
#include <test/sequence/dense.hpp>    // yields_every_position
#include <test/value_reference.hpp>   // value_reference
#include <xstd/bits/bit_array.hpp>    // bit_array
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                      // array
#include <concepts>                   // regular, same_as, totally_ordered
#include <cstddef>                    // ptrdiff_t
#include <functional>                 // hash
#include <iterator>                   // contiguous_iterator, random_access_iterator
#include <ranges>                     // begin, contiguous_range, iota, random_access_range
#include <tuple>                      // tuple_cat
#include <utility>                    // declval

BOOST_AUTO_TEST_SUITE(BitArray)

// Every Block model within one block, the narrow ones across boundaries, and the widest across one too.
using Types = decltype(std::tuple_cat(
        std::declval<test::graded_extents<xstd::basic_bit_array>>(),
        std::declval<test::wide_extents<xstd::basic_bit_array>>()
));

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

// Random access is where it stops: the blocks are contiguous, the bits are not addressable.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItIsNotAContiguousRange, T, Types)
{
        static_assert(not std::ranges::contiguous_range<T>);
        static_assert(not std::contiguous_iterator<typename T::iterator>);
}

// operator& on the proxy answers an iterator, not a pointer, so the identity holds in iterator arithmetic.
BOOST_AUTO_TEST_CASE_TEMPLATE(AddressOfASubscriptIsTheIteratorToIt, T, Types)
{
        static_assert(std::same_as<decltype(&std::declval<T&>()[0UZ]), typename T::iterator>);

        auto a = T();
        for (auto const n : std::views::iota(0UZ, a.size())) {
                auto const step = static_cast<std::ptrdiff_t>(n);
                BOOST_CHECK(&a[n] == &a[0UZ] + step);
                BOOST_CHECK(&a[n] == std::ranges::begin(a) + step);
                BOOST_CHECK(static_cast<bool>(*(&a[n])) == static_cast<bool>(a[n]));
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsConstReferenceIsAValue, T, Types)
{
        static_assert(test::value_reference<typename T::const_reference>);
}

// Every owner hashes, this one although std::array<bool, N> does not: equal values equal, at every extent.
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

// [array]'s synopsis line by line, the model first so the checklist is known to be honest.
static_assert(test::sequence::array_bool<std::array<bool, 5>>);

BOOST_AUTO_TEST_CASE_TEMPLATE(ItAnswersEveryLineOfStdArrayBool, T, Types)
{
        static_assert(test::sequence::array_bool<T>);
}

// A proxy, so a binding over the array writes through to it; by value it would bind to the copy.
BOOST_AUTO_TEST_CASE(AStructuredBindingWritesThroughToTheArray)
{
        auto a = xstd::bit_array<3>({true, false, true});
        auto& [x, y, z] = a;
        y = true;
        BOOST_CHECK(a[1] == true);
        BOOST_CHECK(x == true);
        BOOST_CHECK(z == true);
}

// Every position, densely, agreeing with the subscript -- and not a contiguous range, which no proxy sequence can be.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItYieldsEveryPosition, T, Types)
{
        auto c = T();
        test::sequence::yields_every_position(c);

        for (auto const n : std::views::iota(0UZ, c.size())) {
                c[n] = (n % 3UZ == 0UZ);
        }
        test::sequence::yields_every_position(c);
}

BOOST_AUTO_TEST_SUITE_END()
