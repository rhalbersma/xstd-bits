//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/ordering.hpp>              // ordering_agrees_with_std_set
#include <xstd/bits/bit_array.hpp>            // bit_array
#include <xstd/bits/bit_fixed_set.hpp>        // bit_fixed_set
#include <xstd/bits/bit_set.hpp>              // bit_set
#include <xstd/bits/bit_set_view.hpp>         // bit_set_view
#include <xstd/bits/bit_span.hpp>             // bit_span
#include <xstd/bits/detail/bit_container.hpp> // bit_container
#include <xstd/bits/detail/ownership.hpp>     // storage
#include <xstd/bits/detail/set_adaptor.hpp>   // set_adaptor
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <array>                              // array
#include <concepts>                           // constructible_from, derived_from, same_as
#include <cstddef>                            // size_t
#include <cstdint>                            // uint64_t, uint8_t
#include <functional>                         // hash
#include <range/v3/view/set_algorithm.hpp>    // set_union
#include <ranges>                             // borrowed_range, range, range_value_t, view
#include <set>                                // set
#include <tuple>                              // tuple
#include <utility>                            // declval
#include <vector>                             // vector

BOOST_AUTO_TEST_SUITE(BitSetView)

namespace {

// A static extent over blocks no container owns, and a run-time one over a set owner's growing storage.
using ViewedTypes = std::tuple<std::array<std::uint8_t, 1>, xstd::bit_set>;

// Written through a view of the type, which is the one door both of them share.
template<class T>
auto three_set()
        -> T
{
        auto bits = T();
        xstd::bit_set_view(bits).insert(3UZ);
        return bits;
}

using Storage = xstd::bits::detail::bit_container<std::array<std::size_t, 1>, 8>;
using Blocks = std::array<std::size_t, 1>;

template<class T>
using view_of = decltype(xstd::bit_set_view(std::declval<T&>()));

// Named rather than a lambda, so the conversion happens at a call boundary the way a caller would meet it.
constexpr auto takes_a_set_view(xstd::bit_set_view<Blocks, 8> v) noexcept
        -> bool
{
        return v.contains(3UZ);
}

} // namespace

// The view is the referring adaptor under another name, and over an owner it refers into the storage the owner wraps.
BOOST_AUTO_TEST_CASE(TheViewIsTheReferringAdaptor)
{
        static_assert(std::derived_from<xstd::bit_set_view<Blocks, 8>, xstd::bits::detail::set_adaptor<Storage, xstd::bits::detail::storage::borrowed, xstd::bit_set_view<Blocks, 8>>>);
        static_assert(std::same_as<view_of<Storage>, xstd::bit_set_view<Blocks, 8>>);
        static_assert(std::same_as<view_of<Storage const>, xstd::bit_set_view<Blocks const, 8>>);

        static_assert(std::same_as<view_of<xstd::bit_fixed_set<8>>, xstd::bit_set_view<Blocks, 8>>);
        static_assert(std::same_as<view_of<xstd::bit_fixed_set<8> const>, xstd::bit_set_view<Blocks const, 8>>);
}

// A set owner is committed to the set reading and a sequence owner to the sequence one; only the first is set-viewed.
BOOST_AUTO_TEST_CASE(TheReadingsDoNotMix)
{
        static_assert(std::same_as<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>())), xstd::bit_span<Blocks, 8>>);
        static_assert(std::constructible_from<xstd::bit_set_view<Blocks, 8>, xstd::bit_fixed_set<8>&>);
        static_assert(not std::constructible_from<xstd::bit_set_view<Blocks, 8>, xstd::bit_array<8>&>);
}

// Viewing an owner is implicit, viewing raw storage is not: the first claims nothing the owner does not carry.
BOOST_AUTO_TEST_CASE(ViewingAnOwnerIsImplicit)
{
        static_assert(std::convertible_to<xstd::bit_fixed_set<8>&, xstd::bit_set_view<Blocks, 8>>);
        static_assert(std::convertible_to<xstd::bit_fixed_set<8> const&, xstd::bit_set_view<Blocks const, 8>>);
        static_assert(not std::convertible_to<xstd::bit_fixed_set<8> const&, xstd::bit_set_view<Blocks, 8>>);

        static_assert(not std::convertible_to<xstd::bit_fixed_set<8>, xstd::bit_set_view<Blocks, 8>>);
        static_assert(not std::convertible_to<xstd::bit_fixed_set<8>&&, xstd::bit_set_view<Blocks, 8>>);

        static_assert(std::constructible_from<xstd::bit_set_view<Blocks, 8>, Storage&>);
        static_assert(not std::convertible_to<Storage&, xstd::bit_set_view<Blocks, 8>>);

        auto s = xstd::bit_fixed_set<8>();
        s.insert(3UZ);
        BOOST_CHECK(takes_a_set_view(s));
}

// The types a bit_set_view exists for: those holding a set of positions without offering it.
BOOST_AUTO_TEST_CASE(TheViewedTypesAreTheOnesHoldingASetWithoutOfferingIt)
{
        // A block is no range of positions, and blocks range over blocks; the view supplies the set, borrowed like span.
        static_assert(not std::ranges::range<std::uint64_t>);
        static_assert(std::same_as<std::ranges::range_value_t<Blocks>, std::size_t>);

        static_assert(std::ranges::view<view_of<Storage>>);
        static_assert(std::ranges::borrowed_range<view_of<Storage>>);
        static_assert(not std::ranges::view<xstd::bit_fixed_set<8>>);
}

// The view hashes what it presents, so the owner's set reading of the same bits hashes the same.
BOOST_AUTO_TEST_CASE(TheViewHashesAsAValue)
{
        auto bits = Storage();
        for (auto const i : {1UZ, 3UZ, 5UZ}) {
                bits.set(i);
        }
        auto const owned = xstd::bit_fixed_set<8>({1, 3, 5});
        BOOST_CHECK_EQUAL(std::hash<view_of<Storage>>()(xstd::bit_set_view(bits)), std::hash<xstd::bit_fixed_set<8>>()(owned));

        auto narrow = std::vector<std::uint8_t>{0b0010'1010};
        auto wide = std::vector<std::uint8_t>{0b0010'1010, 0, 0, 0, 0, 0, 0, 0};
        BOOST_CHECK_EQUAL(std::hash<view_of<std::vector<std::uint8_t>>>()(xstd::bit_set_view(narrow)), std::hash<view_of<std::vector<std::uint8_t>>>()(xstd::bit_set_view(wide)));
}

// Asking is total whatever the extent, exactly as [set] has it.
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryExtentAnswersForPositionsPastItsWidth, T, ViewedTypes)
{
        auto bits = three_set<T>();
        auto const v = xstd::bit_set_view(bits);

        // Both ways round, so that find and lower_bound are each seen taking either arm.
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.count(3), 1);
        // find is the subject here, which is the one call readability-container-contains would remove.
        BOOST_CHECK(v.find(3) != v.end()); // NOLINT(readability-container-contains)
        BOOST_CHECK(v.lower_bound(3) == v.find(3));

        BOOST_CHECK(not v.contains(99));
        BOOST_CHECK_EQUAL(v.count(99), 0);
        BOOST_CHECK(v.find(99) == v.end()); // NOLINT(readability-container-contains)
        BOOST_CHECK(v.lower_bound(99) == v.end());

        // Asked from inside the width, where the scan actually runs: nothing is above 3 here.
        BOOST_CHECK(v.upper_bound(3) == v.end());
        BOOST_CHECK(v.upper_bound(99) == v.end());

        // Erasing what is not there is std::set::erase's no-op returning zero.
        BOOST_CHECK_EQUAL(v.erase(99), 0);
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.size(), 1);
}

// [set] gives insert no way to fail, so a dynamic extent grows its owner's storage rather than asserting.
BOOST_AUTO_TEST_CASE(ADynamicExtentGrowsToHoldAPositionPastItsCurrentSize)
{
        auto bits = xstd::bit_set{3};
        auto const v = xstd::bit_set_view(bits);

        auto const [where, inserted] = v.insert(99);
        BOOST_CHECK(inserted);
        BOOST_CHECK(where != v.end());
        BOOST_CHECK(v.contains(99));
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.size(), 2);
        BOOST_CHECK(bits.contains(99));

        // And within the width it is the plain write.
        v.insert(5);
        BOOST_CHECK(v.contains(5) and bits.contains(5));

        v.erase(99);
        BOOST_CHECK(not v.contains(99));
        BOOST_CHECK_EQUAL(bits.size(), 2);
}

// The set ordering against std::set rather than a restatement of it, over an owner at either width.
BOOST_AUTO_TEST_CASE(EveryViewedOwnerOrdersLikeAStdSet)
{
        test::set::ordering_agrees_with_std_set<xstd::bit_fixed_set<8>>();
        test::set::ordering_agrees_with_std_set<xstd::bit_set>();
}

// One block cannot reach the arm the block-parallel comparison exists for.
BOOST_AUTO_TEST_CASE(TheOrderingSpansBlocksAndNotJustPositions)
{
        test::set::ordering_agrees_with_std_set<xstd::basic_bit_fixed_set<std::uint8_t, 9>>(9);

        // Three blocks, where "anything above" has to look past the next block as well as into it.
        test::set::ordering_agrees_with_std_set_sampled<xstd::basic_bit_fixed_set<std::uint8_t, 18>>(18UZ, 20000UZ);
        test::set::ordering_agrees_with_std_set_sampled<xstd::basic_bit_set<std::uint8_t>>(18UZ, 20000UZ);
}

// A view of keys composes with the lazy set algebra; the block-wise operators are the owner's shortcut.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheLazySetAlgebraRunsOverTheView, T, ViewedTypes)
{
        auto x = three_set<T>();
        auto y = three_set<T>();

        // Named: range-v3's viewable_range predates P2415 and takes a view only by lvalue or its own marker.
        auto const xv = xstd::bit_set_view(x);
        auto const yv = xstd::bit_set_view(y);
        xv.insert({1, 5});
        yv.insert({5, 7});

        auto merged = std::set<std::size_t>();
        for (std::size_t const k : ::ranges::views::set_union(xv, yv)) {
                merged.insert(k);
        }
        BOOST_CHECK((merged == std::set<std::size_t>{1, 3, 5, 7}));
}

BOOST_AUTO_TEST_SUITE_END()
