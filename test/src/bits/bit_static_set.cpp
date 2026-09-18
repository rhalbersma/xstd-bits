//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>         // graded_extents
#include <test/set/ascending.hpp>       // yields_ascending_keys
#include <test/set/concepts.hpp>        // bit_set
#include <test/value_reference.hpp>     // value_reference
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <xstd/bits/bitset.hpp>         // bitset
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <bitset>                       // bitset
#include <concepts>                     // regular, totally_ordered
#include <cstddef>                      // size_t
#include <cstdint>                      // uint64_t
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
                static_assert([] -> bool {
                        // A PATTERN rather than a mutation, which is what makes this one expression at every graded
                        // extent. std::bitset's constructor from unsigned long long masks to the width, so ~0ULL is
                        // every position it has -- and at the zero width that is none, where set(0) would throw
                        // out_of_range and take the whole assertion down over a position that does not exist. It also
                        // leaves nothing here non-const, which a mutation the zero width discards does not.
                        auto const bs = std::bitset<N>(~0ULL);
                        auto const c  = T(bs);
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

// The two conversions are named by a CONCEPT, not by std::bitset, so anything whose N bits this library can prove
// it reads correctly comes in on the same rule. An unsigned integer is the family that proves nothing, because the
// language already states it: bit n of the value is 2^n. The guard is the public question -- does the conversion
// exist? -- which is false exactly where the integer is too narrow for the width.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnUnsignedIntegerIsAFieldOfBitsToo, T, Types)
{
        constexpr auto N = T().max_size();
        if constexpr (std::is_constructible_v<T, std::uint64_t>) {
                // Every position the width has, and none of them. Said WITHOUT a shift: a ternary guards the value
                // it picks but not the expression it does not, so 1ULL << N is still compiled at a width of
                // sixty-four, where the shift is undefined and MSVC says so (C4293) though GCC and clang fold it
                // silently. std::bitset answers the same mask by flipping an empty one, and needs no shift at all.
                constexpr auto all = std::bitset<N>().flip().to_ullong();
                auto const full = T(all);
                BOOST_CHECK_EQUAL(full.size(), N);
                BOOST_CHECK(static_cast<std::uint64_t>(full) == all);

                auto const none = T(std::uint64_t{});
                BOOST_CHECK_EQUAL(none.size(), 0UZ);
                BOOST_CHECK(static_cast<std::uint64_t>(none) == 0ULL);

                static_assert(not std::is_convertible_v<std::uint64_t, T>);
                static_assert(not std::is_convertible_v<T, std::uint64_t>);
        }
}

// And our own bitset reading crosses to the set reading on that same rule, which is the generalisation paying for
// itself: neither side is std::bitset, and neither is named in the constraint.
BOOST_AUTO_TEST_CASE_TEMPLATE(OurOwnBitsetReadingCrossesOnTheSameRule, T, Types)
{
        constexpr auto N = T().max_size();
        using Bitset = xstd::bitset<N>;
        if constexpr (std::is_constructible_v<T, Bitset>) {
                auto b = Bitset();
                for (auto i = 0UZ; i < N; i += 5UZ) {
                        b.set(i);
                }
                auto const c = T(b);
                BOOST_CHECK_EQUAL(c.size(), b.count());
                for (auto i = 0UZ; i < N; ++i) {
                        BOOST_CHECK_EQUAL(c.contains(i), b.test(i));
                }
                BOOST_CHECK(static_cast<Bitset>(c) == b);
        }
}

// RAW BLOCKS cross to the set reading on the same rule as anything else, and the block width is free: the byte is
// the common ground, so eight uint32 blocks and four uint64 blocks spell the same two hundred and fifty-six
// positions. Nothing is probed for either -- a sequence of unsigned integers states its layout.
BOOST_AUTO_TEST_CASE(RawBlocksCrossOnTheSameRule)
{
        constexpr auto N = 256UZ;
        using Wide   = std::array<std::uint64_t, 4>;
        using Narrow = std::array<std::uint32_t, 8>;

        auto const blocks = Wide{ 0x0123'4567'89AB'CDEFULL, 1ULL, 0ULL, 0x8000'0000'0000'0000ULL };
        auto const s = xstd::bit_static_set<N>(blocks);

        BOOST_CHECK(s.contains(0UZ));
        BOOST_CHECK(s.contains(64UZ));
        BOOST_CHECK(s.contains(N - 1UZ));
        BOOST_CHECK(static_cast<Wide>(s) == blocks);

        // The same positions over a different block width.
        auto const narrow = static_cast<Narrow>(s);
        BOOST_CHECK_EQUAL(narrow[0], 0x89AB'CDEFU);
        BOOST_CHECK_EQUAL(narrow[1], 0x0123'4567U);
        BOOST_CHECK_EQUAL(narrow[7], 0x8000'0000U);
        BOOST_CHECK(xstd::bit_static_set<N>(narrow) == s);

        static_assert([] -> bool {
                auto const b = Wide{ 0xDEAD'BEEFULL, 0ULL, 0ULL, 0ULL };
                return static_cast<Wide>(xstd::bit_static_set<N>(b)) == b;
        }());

        // Too narrow for the width is no conversion at all; wider is admitted, as it is for an integer.
        static_assert(not std::is_constructible_v<xstd::bit_static_set<N>, std::array<std::uint64_t, 3>>);
        static_assert(    std::is_constructible_v<xstd::bit_static_set<N>, std::array<std::uint64_t, 5>>);
}

BOOST_AUTO_TEST_SUITE_END()
