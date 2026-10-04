//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bit_exchange.hpp>          // converts_between, exchanges_from_bits
#include <test/block_types.hpp>           // graded_extents
#include <test/set/ascending.hpp>         // yields_ascending_keys
#include <test/set/strong_index.hpp>      // agrees_with_std_set_of_strong_indices, offset_traits, strong_index
#include <test/value_reference.hpp>       // value_reference
#include <xstd/bits/bit/bit_convert.hpp>  // bit_convert
#include <xstd/bits/bit_fixed_set.hpp>    // bit_fixed_set
#include <xstd/bits/from_bit_storage.hpp> // from_bit_storage
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                          // array
#include <bitset>                         // bitset
#include <concepts>                       // regular, same_as, totally_ordered
#include <cstddef>                        // size_t
#include <cstdint>                        // uint32_t, uint64_t, uint8_t
#include <iterator>                       // bidirectional_iterator
#include <ranges>                         // bidirectional_range, iota, size, to
#include <stdexcept>                      // out_of_range
#include <type_traits>                    // integral_constant, is_constructible_v, is_convertible_v, is_member_function_pointer_v
#include <utility>                        // declval

BOOST_AUTO_TEST_SUITE(BitFiniteSet)

// The block and the width as the extents expand them, the key being std::size_t.
template<class Block, std::size_t N>
using fixed_set_of = xstd::basic_bit_fixed_set<std::size_t, Block, N>;

// Every Block model within one block, and the narrow ones across boundaries.
using Types = test::graded_extents<fixed_set_of>;

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

// A requires-expression on a concrete type is ill-formed rather than false ([expr.prim.req]/5).
template<class X>
constexpr bool has_allocator_type = requires { typename X::allocator_type; };

// The width is the type's, so there is no allocator for the synopsis's allocator lines to name.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItHasNoAllocatorType, T, Types)
{
        static_assert(not has_allocator_type<T>);
}

// The positions there are to hold are the type's, so max_size is a constant; size() and empty() count the keys.
BOOST_AUTO_TEST_CASE(MaxSizeIsAConstantOfTheTypeAndTheCountsAreFunctions)
{
        using T = xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 9>;
        static_assert(std::same_as<decltype(T::max_size), std::integral_constant<std::size_t, 9> const>);
        static_assert(std::same_as<decltype(xstd::bit_fixed_set<0>::max_size), std::integral_constant<std::size_t, 0> const>);
        // NOLINTBEGIN(readability-static-accessed-through-instance): the call through an object is what is checked.
        static_assert(std::same_as<decltype(std::declval<T const&>().max_size()), T::size_type>);
        static_assert(noexcept(std::declval<T const&>().max_size()));
        static_assert(std::is_member_function_pointer_v<decltype(&T::size)>);
        static_assert(std::is_member_function_pointer_v<decltype(&T::empty)>);

        static_assert(T::max_size == 9UZ);
        auto const a = T({1UZ, 8UZ});
        BOOST_CHECK_EQUAL(a.max_size(), 9UZ);
        BOOST_CHECK_EQUAL(std::ranges::size(a), 2UZ);
        BOOST_CHECK(not a.empty());
        // NOLINTEND(readability-static-accessed-through-instance)
}

// Total lookups, swept over every width because no single one exposed all six operations.
template<class X>
auto check_key_outside_the_domain(X a, std::size_t x)
        -> void
{
        auto const original = a;

        BOOST_CHECK(not a.contains(x));
        BOOST_CHECK_EQUAL(a.count(x), 0UZ);
        BOOST_CHECK(a.find(x) == a.end());
        BOOST_CHECK(a.lower_bound(x) == a.end());
        BOOST_CHECK(a.upper_bound(x) == a.end());

        auto const [first, last] = a.equal_range(x);
        BOOST_CHECK(first == a.end());
        BOOST_CHECK(last == a.end());

        // A no-op that must stay one: this is the write.
        BOOST_CHECK_EQUAL(a.erase(x), 0UZ);
        BOOST_CHECK(a == original);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(LookupIsTotalOverKeyType, T, Types)
{
        auto const N    = T().max_size();
        auto const full = std::views::iota(0UZ, N) | std::ranges::to<T>();

        // Just past the end, past the last block, and the value that would wrap any n + 1.
        for (auto const x : {N, N + 1, (2 * N) + 1, static_cast<std::size_t>(-1)}) {
                check_key_outside_the_domain(T(), x);
                check_key_outside_the_domain(full, x);
        }
}

// Ascending keys at every width and whatever the insertion order: what makes this a set rather than a bag.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItYieldsAscendingKeys, T, Types)
{
        auto c = T();
        test::set::yields_ascending_keys(c); // empty is trivially ascending

        // Inserted high to low and across block boundaries, so the ascending answer is the container's doing.
        for (auto const key : {70UZ, 64UZ, 63UZ, 9UZ, 1UZ, 0UZ}) {
                if (key < c.max_size()) {
                        c.insert(key);
                }
        }
        test::set::yields_ascending_keys(c);
}

// A static width is a capacity and a std::bitset's own width both, so position n here is bit n there.
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
                for (auto const i : std::views::iota(0UZ, N)) {
                        BOOST_CHECK_EQUAL(c.contains(i), bs.test(i));
                }

                // Out again, and the identity: the two keep the same tail invariant.
                BOOST_CHECK(static_cast<std::bitset<N>>(c) == bs);

                // The empty and the full set, the two the loop above reaches neither of.
                BOOST_CHECK(static_cast<std::bitset<N>>(T()) == std::bitset<N>());
                BOOST_CHECK(static_cast<std::bitset<N>>(T(std::bitset<N>().flip())) == std::bitset<N>().flip());
        }
}

// The same claim in a constant expression, which is what shifts rather than a memcpy buy.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItRoundTripsAtCompileTime, T, Types)
{
        constexpr auto N = T().max_size();
        if constexpr (std::is_constructible_v<T, std::bitset<N>>) {
                static_assert([] -> bool {
                        // A pattern, not a mutation: ~0ULL masks to the width, which at the zero width is no position.
                        auto const bs = std::bitset<N>(~0ULL);
                        auto const c  = T(bs);
                        return c.size() == bs.count() and static_cast<std::bitset<N>>(c) == bs;
                }());
        }
}

// Explicit both ways, not because either could fail: a reader picks the reading rather than a conversion picking it.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheConversionsAreExplicitBothWays, T, Types)
{
        constexpr auto N = T().max_size();
        // Guarded on the constructor rather than the concept, so this asks the public question and not the detail one.
        if constexpr (std::is_constructible_v<T, std::bitset<N>>) {
                static_assert(not std::is_convertible_v<std::bitset<N>, T>);
                static_assert(std::is_constructible_v<std::bitset<N>, T>);
                static_assert(not std::is_convertible_v<T, std::bitset<N>>);
        }
}

// Any other width is no conversion rather than a narrower one, so a mismatch does not compile.
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

// The two conversions are named by a concept and not by std::bitset, and an unsigned integer proves nothing.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnUnsignedIntegerIsAFieldOfBitsToo, T, Types)
{
        constexpr auto N = T().max_size();
        if constexpr (std::is_constructible_v<T, std::uint64_t>) {
                // Every position and none, said without a shift: 1ULL << N still compiles at sixty-four (MSVC's C4293).
                constexpr auto all = std::bitset<N>().flip().to_ullong();
                auto const full    = T(all);
                BOOST_CHECK_EQUAL(full.size(), N);
                BOOST_CHECK(static_cast<std::uint64_t>(full) == all);

                auto const none = T(std::uint64_t{});
                BOOST_CHECK_EQUAL(none.size(), 0UZ);
                BOOST_CHECK(static_cast<std::uint64_t>(none) == 0ULL);

                static_assert(not std::is_convertible_v<std::uint64_t, T>);
                static_assert(not std::is_convertible_v<T, std::uint64_t>);
        }
}

// Raw blocks cross on the same rule and the block width is free, the byte being the common ground.
BOOST_AUTO_TEST_CASE(RawBlocksCrossOnTheSameRule)
{
        constexpr auto N = 256UZ;
        using Set        = xstd::bit_fixed_set<N>;
        using Wide       = std::array<std::uint64_t, 4>;
        using Narrow     = std::array<std::uint32_t, 8>;

        auto const blocks = Wide{0x0123'4567'89AB'CDEFULL, 1ULL, 0ULL, 0x8000'0000'0000'0000ULL};
        auto const s      = Set(xstd::from_bit_storage, blocks);

        BOOST_CHECK(s.contains(0UZ));
        BOOST_CHECK(s.contains(64UZ));
        BOOST_CHECK(s.contains(N - 1UZ));
        BOOST_CHECK(xstd::bit_convert<Wide>(s) == blocks);

        // The same positions over a different block width.
        auto const narrow = xstd::bit_convert<Narrow>(s);
        BOOST_CHECK_EQUAL(narrow[0], 0x89AB'CDEFU);
        BOOST_CHECK_EQUAL(narrow[1], 0x0123'4567U);
        BOOST_CHECK_EQUAL(narrow[7], 0x8000'0000U);
        BOOST_CHECK(Set(xstd::from_bit_storage, narrow) == s);

        static_assert([] -> bool {
                auto const b = Wide{0xDEAD'BEEFULL, 0ULL, 0ULL, 0ULL};
                return xstd::bit_convert<Wide>(Set(xstd::from_bit_storage, b)) == b;
        }());

        // Too narrow is no exchange and wider is admitted; the door is the stronger question over is_constructible_v.
        static_assert(not test::exchanges_from_bits<Set, std::array<std::uint64_t, 3>>);
        static_assert(test::exchanges_from_bits<Set, std::array<std::uint64_t, 5>>);
        static_assert(test::exchanges_from_bits<Set, Wide> and test::converts_between<Set, Wide>);
        static_assert(test::exchanges_from_bits<Set, Narrow> and test::converts_between<Set, Narrow>);

        // And the unnamed door is closed, so a sequence of blocks does not read as the from_range spelling.
        static_assert(not std::is_constructible_v<Set, Wide>);
        static_assert(not std::is_constructible_v<Set, Narrow>);
}

// A traits type of its own closes the universe: keys 10 to 29 at positions 0 to 19, answering as std::set does.
BOOST_AUTO_TEST_CASE(AStrongIndexWithItsOwnTraitsKeysItAsStdSetIsKeyed)
{
        using traits = test::set::offset_traits<10UZ, 20UZ>;
        using X      = xstd::basic_bit_fixed_set<test::set::strong_index, std::uint8_t, 20UZ, traits>;
        static_assert(std::same_as<X::key_type, test::set::strong_index>);
        static_assert(std::same_as<X::key_traits_type, traits>);
        static_assert(std::same_as<std::iter_value_t<X::iterator>, test::set::strong_index>);
        static_assert(X::max_size() == 20UZ);

        test::set::agrees_with_std_set_of_strong_indices<X>({}, 10UZ, 30UZ);
        test::set::agrees_with_std_set_of_strong_indices<X>({10UZ, 17UZ, 18UZ, 29UZ}, 10UZ, 30UZ);
        test::set::agrees_with_std_set_of_strong_indices<X>({10UZ, 11UZ, 12UZ, 13UZ, 14UZ, 15UZ, 16UZ, 17UZ, 18UZ, 19UZ, 20UZ, 21UZ, 22UZ, 23UZ, 24UZ, 25UZ, 26UZ, 27UZ, 28UZ, 29UZ}, 10UZ, 30UZ);

        // The positions are the keys less 10, which is what the blocks hold.
        auto const x = X({test::set::strong_index{.value = 10UZ}, test::set::strong_index{.value = 19UZ}});
        BOOST_CHECK(xstd::bit_convert<std::bitset<20>>(x) == std::bitset<20>(0x201ULL));
        BOOST_CHECK_THROW(static_cast<void>(X({test::set::strong_index{.value = 30UZ}})), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
