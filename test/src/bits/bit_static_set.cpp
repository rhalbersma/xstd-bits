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
#include <bitset>                       // bitset
#include <concepts>                     // regular, totally_ordered
#include <cstddef>                      // size_t
#include <iterator>                     // bidirectional_iterator
#include <ranges>                       // bidirectional_range, iota, to
#include <type_traits>                  // is_constructible_v, is_convertible_v

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

// Total lookups, swept over every width because no single one exposed all six operations.
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

// Ascending keys, at every width and whatever the insertion order: what makes this a set rather than a bag of positions.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItYieldsAscendingKeys, T, Types)
{
        auto c = T();
        test::set::yields_ascending_keys(c);            // empty is trivially ascending

        // Inserted high to low, and across block boundaries where the width allows, so the ascending answer is the container's doing and not the insertion order's.
        for (auto const key : { 70UZ, 64UZ, 63UZ, 9UZ, 1UZ, 0UZ }) {
                if (key < c.max_size()) {
                        c.insert(key);
                }
        }
        test::set::yields_ascending_keys(c);
}

// A static width is a CAPACITY under this reading and a std::bitset's own width both, so position n here is bit n
// there and the conversion has no policy to choose: nothing truncates, nothing grows, nothing throws. Asserted at
// every graded extent and every Block, which is what makes it a claim about the bits and not about one block width
// -- xstd::uint128 blocks are WIDER than the std::bitset object they come from, uint8_t ones narrower, and the byte
// the two agree on is neither.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItRoundTripsThroughStdBitset, T, Types)
{
        constexpr auto N = T().max_size();
        if constexpr (std::is_constructible_v<T, std::bitset<N>>) {
                auto bs = std::bitset<N>();
                for (auto i = 0UZ; i < N; i += 7UZ) {
                        bs.set(i);
                }

                auto const c = T(bs);
                BOOST_CHECK_EQUAL(c.size(), bs.count());
                for (auto i = 0UZ; i < N; ++i) {
                        BOOST_CHECK_EQUAL(c.contains(i), bs.test(i));
                }

                // Out again, and the identity: the two keep the same tail invariant, so nothing is left over either way.
                BOOST_CHECK(static_cast<std::bitset<N>>(c) == bs);

                // The empty and the full set, the two the loop above reaches neither of.
                BOOST_CHECK(static_cast<std::bitset<N>>(T()) == std::bitset<N>());
                BOOST_CHECK(static_cast<std::bitset<N>>(T(std::bitset<N>().flip())) == std::bitset<N>().flip());
        }
}

// The same claim in a constant expression, which is what the storage's byte primitive being shifts rather than a
// memcpy buys: neither direction reads memory it must be running to see.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItRoundTripsAtCompileTime, T, Types)
{
        constexpr auto N = T().max_size();
        if constexpr (std::is_constructible_v<T, std::bitset<N>>) {
                static_assert([]{
                        auto bs = std::bitset<N>();
                        if constexpr (N > 0UZ) {
                                // Guarded, because a zero width is one of the graded extents and std::bitset<0>::set(0)
                                // throws out_of_range -- which is no constant expression, and would take this whole
                                // assertion down over a position that does not exist rather than over the conversion.
                                bs.set(0UZ);
                                bs.set(N - 1UZ);
                        }
                        auto const c = T(bs);
                        return c.size() == bs.count() and static_cast<std::bitset<N>>(c) == bs;
                }());
        }
}

// EXPLICIT in both directions, and not because either could fail: a set of positions and a field of bits are two
// readings of the same bits, and this library makes a reader pick one rather than letting a conversion pick for them.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheConversionsAreExplicitBothWays, T, Types)
{
        constexpr auto N = T().max_size();
        // Guarded on the constructor rather than on the concept behind it: a standard library laying its bits out
        // some other way withholds BOTH of these, and this test asks the public question, not the detail one.
        if constexpr (std::is_constructible_v<T, std::bitset<N>>) {
                static_assert(not std::is_convertible_v  <std::bitset<N>, T>);
                static_assert(    std::is_constructible_v<std::bitset<N>, T>);
                static_assert(not std::is_convertible_v  <T, std::bitset<N>>);
        }
}

// Any other width is not a narrower conversion, it is no conversion: the two widths mean the same positions or the
// question has no answer, so a mismatch is a call that does not compile rather than one that silently drops keys.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnyOtherWidthIsNoConversionAtAll, T, Types)
{
        constexpr auto N = T().max_size();
        if constexpr (std::is_constructible_v<T, std::bitset<N>>) {
                static_assert(not std::is_constructible_v<T, std::bitset<N + 1UZ>>);
                if constexpr (N > 1UZ) {
                        static_assert(not std::is_constructible_v<T, std::bitset<N - 1UZ>>);
                }
        }
}

BOOST_AUTO_TEST_SUITE_END()
