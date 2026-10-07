//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/ascending.hpp>                   // yields_ascending_keys
#include <xstd/bits/bit_blocks.hpp>                 // bit_align, bit_least
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set, bit_bounded_set
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/bounded_blocks.hpp>      // bounded_blocks, XSTD_BITS_HAS_CONSTEXPR_BOUNDED
#include <xstd/bits/detail/ownership.hpp>           // owned_bits_t, storage
#include <xstd/bits/detail/set_adaptor.hpp>         // set_adaptor
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                // equal
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint16_t, uint8_t
#include <limits>                                   // numeric_limits
#include <new>                                      // bad_alloc
#include <ranges>                                   // iota, size, to
#include <set>                                      // set
#include <type_traits>                              // integral_constant, is_member_function_pointer_v
#include <utility>                                  // declval

#ifdef XSTD_BITS_HAS_CONSTEXPR_BOUNDED
#include <test/constexpr_check.hpp> // XSTD_CONSTEXPR_CHECK_EQUAL
#endif

// The bounded checker, whose static_assert half arrives only with std::inplace_vector.
#ifdef XSTD_BITS_HAS_CONSTEXPR_BOUNDED
#define XSTD_CONSTEXPR_BOUNDED_CHECK_EQUAL(a, b) XSTD_CONSTEXPR_CHECK_EQUAL((a), (b))
#else
#define XSTD_CONSTEXPR_BOUNDED_CHECK_EQUAL(a, b) BOOST_CHECK_EQUAL((a), (b))
#endif

BOOST_AUTO_TEST_SUITE(BitBoundedSet)

// A capacity of three whole blocks, so a key can sit past the width and still inside the capacity.
using T = xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 24>;

// Dependent, so an absent member is a false rather than a hard error.
template<class X>
constexpr bool has_capacity = requires (X const& x) { x.capacity(); };

// The set reading over a run-time width under a compile-time capacity, built on the set adaptor.
BOOST_AUTO_TEST_CASE(TheBoundedSetIsTheSetAdaptorOverInlineBlocks)
{
        static_assert(std::derived_from<T, xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<xstd::bits::detail::bounded_blocks<std::uint8_t, 3>, 24>, xstd::bits::detail::storage::owned, T>>);
        static_assert(std::same_as<xstd::bit_bounded_set<24>, xstd::basic_bit_bounded_set<std::size_t, std::size_t, 24>>);
}

// A requires-expression failing for a concrete type is ill-formed rather than false ([expr.prim.req]/5).
template<class X>
constexpr bool has_allocator_type = requires { typename X::allocator_type; };

// The capacity is inline, so there is no allocator for the synopsis's allocator lines to name.
BOOST_AUTO_TEST_CASE(ItHasNoAllocatorType)
{
        static_assert(not has_allocator_type<T>);
}

// The capacity is the type's, so max_size is a constant; size() and empty() count the keys.
BOOST_AUTO_TEST_CASE(MaxSizeIsAConstantOfTheTypeAndTheCountsAreFunctions)
{
        static_assert(std::same_as<decltype(T::max_size), std::integral_constant<std::size_t, 24> const>);
        static_assert(std::same_as<decltype(xstd::bit_bounded_set<0>::max_size), std::integral_constant<std::size_t, 0> const>);
        // NOLINTBEGIN(readability-static-accessed-through-instance): the call through an object is what is checked.
        static_assert(std::same_as<decltype(std::declval<T const&>().max_size()), T::size_type>);
        static_assert(noexcept(std::declval<T const&>().max_size()));
        static_assert(std::is_member_function_pointer_v<decltype(&T::size)>);
        static_assert(std::is_member_function_pointer_v<decltype(&T::empty)>);

        static_assert(T::max_size == 24UZ);
        auto const a = T({1UZ, 8UZ});
        BOOST_CHECK_EQUAL(a.max_size(), 24UZ);
        BOOST_CHECK_EQUAL(std::ranges::size(a), 2UZ);
        BOOST_CHECK(not a.empty());
        // NOLINTEND(readability-static-accessed-through-instance)
}

// Built from a range as std::set is, and ordered as std::set is.
BOOST_AUTO_TEST_CASE(ItIsBuiltAndOrderedLikeAStdSet)
{
        auto const s = std::views::iota(0UZ, 24UZ) | std::views::filter([](auto i) { return i % 5 == 0; }) | std::ranges::to<T>();
        auto const k = std::views::iota(0UZ, 24UZ) | std::views::filter([](auto i) { return i % 5 == 0; }) | std::ranges::to<std::set<std::size_t>>();
        BOOST_CHECK(std::ranges::equal(s, k));
        BOOST_CHECK_EQUAL(s.size(), k.size());
}

// A key past the width grows the width, exactly as the heap-backed set does, until the capacity stops it.
BOOST_AUTO_TEST_CASE(InsertingPastTheWidthGrowsItUpToTheCapacity)
{
        auto s = T();
        BOOST_CHECK(s.empty());

        auto const [where, inserted] = s.insert(20);
        BOOST_CHECK(inserted);
        BOOST_CHECK(*where == 20UZ);
        BOOST_CHECK(s.contains(20));
        BOOST_CHECK_EQUAL(s.size(), 1UZ);

        // Below the capacity but above the width, lookups stay total and answer no.
        BOOST_CHECK(not s.contains(23));
        BOOST_CHECK_EQUAL(s.erase(23), 0UZ);
}

// Past the capacity there is nowhere to grow, and the caller gets bad_alloc, as std::inplace_vector's callers do.
BOOST_AUTO_TEST_CASE(InsertingPastTheCapacityThrowsBadAlloc)
{
        auto s = T();

        // max_size() is the positions there are to hold, which under a static capacity is that capacity.
        BOOST_CHECK_EQUAL(s.max_size(), 24UZ);
        static_assert(not has_capacity<T>);
        BOOST_CHECK_THROW(s.insert(24), std::bad_alloc);

        // The failed insert left the set empty, and a key past the capacity is still answerable.
        BOOST_CHECK(s.empty());
        BOOST_CHECK(not s.contains(24));
        BOOST_CHECK(s.find(24) == s.end()); // NOLINT(readability-container-contains)
}

namespace {

// The set holding 0 and key, at the width it is given, so only the highest key can reach the capacity.
auto check_shift_up_to_the_capacity(T const& s, std::size_t key) -> void
{
        auto const room = 23UZ - key;

        // Landing the highest key on the last position is still inside the capacity.
        auto at_capacity = s;
        at_capacity <<= room;
        BOOST_CHECK(at_capacity == T({room, 23UZ}));
        BOOST_CHECK((s << room) == T({room, 23UZ}));

        // One further carries it past, and it alone is dropped: the lower key lands one higher, unless it was that key.
        auto const survivor = key == 0UZ ? T() : T({room + 1UZ});
        auto past_capacity  = s;
        past_capacity <<= room + 1UZ;
        BOOST_CHECK(past_capacity == survivor);
        BOOST_CHECK((s << (room + 1UZ)) == survivor);
}

} // namespace

// A left shift keeps the keys that land below the capacity and drops the rest, at a full width and a narrow one.
BOOST_AUTO_TEST_CASE(ShiftingPastTheCapacityDropsTheKeysThatLandPastIt)
{
        for (auto const key : {0UZ, 3UZ, 9UZ, 23UZ}) {
                auto wide = T({23UZ});
                wide.erase(23UZ);
                wide.insert({0UZ, key});
                check_shift_up_to_the_capacity(wide, key);
                check_shift_up_to_the_capacity(T({0UZ, key}), key);
        }
}

// Growth stops at the capacity, so the shift asks for no storage it lacks and cannot throw.
BOOST_AUTO_TEST_CASE(ShiftingLeftIsNoexcept)
{
        static_assert(noexcept(std::declval<T&>() <<= 1UZ));
        static_assert(noexcept(std::declval<xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 0>&>() <<= 1UZ));
}

// The width a set carries is no part of its value, so a shift keeps the same keys whether the width is wide or narrow.
BOOST_AUTO_TEST_CASE(ShiftingKeepsWhatFitsUnderTheCapacityWhateverTheWidth)
{
        auto wide = T();
        wide.insert(20);
        wide.erase(20);
        wide.insert(3);
        wide <<= 20;
        BOOST_CHECK(wide == T({23UZ}));

        auto narrow = T({3UZ});
        narrow <<= 20;
        BOOST_CHECK(narrow == T({23UZ}));
}

// An empty set has no key to carry, so every distance leaves it empty, the widest too.
BOOST_AUTO_TEST_CASE(ShiftingAnEmptySetLeavesItEmpty)
{
        auto s = T();
        s <<= 24;
        BOOST_CHECK(s.empty());

        s.insert(23);
        s.erase(23);
        s <<= std::numeric_limits<std::size_t>::max();
        BOOST_CHECK(s.empty());
}

// A distance near the top of size_t carries every key past the capacity rather than wrap it onto one that fits.
BOOST_AUTO_TEST_CASE(ShiftingByTheWidestDistancesEmptiesRatherThanWraps)
{
        for (auto const n : {std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max() - 1UZ}) {
                auto s = T({1UZ, 2UZ});
                s <<= n;
                BOOST_CHECK(s.empty());
        }
}

// N is the capacity exactly: a key the last block has room for but N does not is refused all the same.
BOOST_AUTO_TEST_CASE(TheCapacityIsTheRequestedOneExactly)
{
        using U = xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 9>;
        XSTD_CONSTEXPR_BOUNDED_CHECK_EQUAL(U().max_size(), 9UZ);
        static_assert(std::same_as<xstd::bit_align<U>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 16>>);
        static_assert(std::same_as<xstd::bit_align<xstd::bit_bounded_set<9>>, xstd::bit_bounded_set<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::bit_least<xstd::bit_bounded_set<9>>, xstd::basic_bit_bounded_set<std::size_t, std::uint16_t, 9>>);
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 16>>, xstd::bits::detail::bit_block_container<xstd::bits::detail::bounded_blocks<std::uint8_t, 2>>>);

        auto s = U();
        s.insert(8);
        BOOST_CHECK_THROW(s.insert(9), std::bad_alloc);
        BOOST_CHECK_EQUAL(s.size(), 1UZ);
        BOOST_CHECK(s.contains(8) and not s.contains(9));
}

// Width is capacity here as it is on the heap: two sets holding the same keys are equal whatever their widths.
BOOST_AUTO_TEST_CASE(EqualSetsCompareEqualAtUnequalWidths)
{
        auto narrow = T();
        auto wide   = T();

        narrow.insert(3);
        wide.insert(20);
        wide.erase(20);
        wide.insert(3);

        BOOST_CHECK(narrow == wide);
        BOOST_CHECK(not(narrow < wide) and not(wide < narrow));
}

// A key only the wider set holds tells the two apart, on whichever side of the comparison the wider one stands.
BOOST_AUTO_TEST_CASE(AKeyPastTheNarrowerWidthMakesTheSetsUnequal)
{
        auto narrow = T();
        auto wide   = T();

        narrow.insert(3);
        wide.insert(3);
        wide.insert(20);
        BOOST_CHECK(narrow != wide);
        BOOST_CHECK(wide != narrow);

        wide.erase(20);
        BOOST_CHECK(wide == narrow);
}

// Ascending keys, whatever the insertion order: what makes this a set rather than a bag of positions.
BOOST_AUTO_TEST_CASE(ItYieldsAscendingKeys)
{
        auto c = T();
        test::set::yields_ascending_keys(c); // empty is trivially ascending

        // Inserted high to low and across block boundaries, so the ascending answer is the container's doing.
        for (auto const key : {70UZ, 64UZ, 63UZ, 9UZ, 1UZ, 0UZ}) {
                if (key < T::max_size()) {
                        c.insert(key);
                }
        }
        test::set::yields_ascending_keys(c);
}

BOOST_AUTO_TEST_SUITE_END()
