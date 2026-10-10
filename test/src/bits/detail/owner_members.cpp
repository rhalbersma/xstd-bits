//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>     // bit_array
#include <xstd/bits/bit_fixed_set.hpp> // bit_fixed_set
#include <xstd/bits/bit_set.hpp>       // bit_set
#include <xstd/bits/bit_set_view.hpp>  // bit_set_view
#include <xstd/bits/bit_span.hpp>      // bit_span
#include <xstd/bits/bit_vector.hpp>    // bit_vector
#include <boost/test/unit_test.hpp>    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                    // swappable, swappable_with
#include <utility>                     // declval, move

namespace {

// What an owner forwards to its storage, asked of a type by name.
template<class C>
concept has_get_allocator = requires (C const& c) { c.get_allocator(); };

template<class C>
concept has_replace = requires (C& c, C::block_container_type&& b) { c.replace(std::move(b)); };

template<class C>
concept has_extract = requires (C&& c) { std::move(c).extract(); };

template<class C>
concept has_member_swap = requires (C& x, C& y) { x.swap(y); };

// The views over the two owners that have every member, so that what a view lacks is never its storage's doing.
using vector_span = decltype(xstd::bit_span(std::declval<xstd::bit_vector&>()));
using set_view    = decltype(xstd::bit_set_view(std::declval<xstd::bit_set&>()));

} // namespace

BOOST_AUTO_TEST_SUITE(OwnerMembers)

BOOST_AUTO_TEST_CASE(EveryOwnerSwaps)
{
        static_assert(std::swappable<xstd::bit_vector> and has_member_swap<xstd::bit_vector>);
        static_assert(std::swappable<xstd::bit_set> and has_member_swap<xstd::bit_set>);
        static_assert(std::swappable<xstd::bit_array<8>> and has_member_swap<xstd::bit_array<8>>);
        static_assert(std::swappable<xstd::bit_fixed_set<8>> and has_member_swap<xstd::bit_fixed_set<8>>);

        auto x = xstd::bit_vector{true, false};
        auto y = xstd::bit_vector{false};
        x.swap(y);
        BOOST_CHECK_EQUAL(x.size(), 1UZ);
        swap(x, y);
        BOOST_CHECK_EQUAL(x.size(), 2UZ);
}

// [vector.bool]'s static swap of two proxies still stands beside the owner's swap of two containers.
BOOST_AUTO_TEST_CASE(TheSequenceKeepsItsProxySwap)
{
        auto v = xstd::bit_vector{true, false};
        xstd::bit_vector::swap(v[0], v[1]);
        BOOST_CHECK(not v[0] and v[1]);
}

// An owner swaps with its own type only: the two readings over one storage never meet.
BOOST_AUTO_TEST_CASE(TwoReadingsDoNotSwap)
{
        static_assert(not std::swappable_with<xstd::bit_vector&, xstd::bit_set&>);
        static_assert(not std::swappable_with<xstd::bit_array<8>&, xstd::bit_fixed_set<8>&>);
}

BOOST_AUTO_TEST_CASE(AnOwnerOverAnAllocatingStorageAnswersAll)
{
        static_assert(has_get_allocator<xstd::bit_vector> and has_replace<xstd::bit_vector> and has_extract<xstd::bit_vector>);
        static_assert(has_get_allocator<xstd::bit_set> and has_replace<xstd::bit_set> and has_extract<xstd::bit_set>);
        static_assert(not has_get_allocator<xstd::bit_array<8>> and not has_get_allocator<xstd::bit_fixed_set<8>>);
}

// A view's handle has none of what an owner forwards to its storage, so none of the members answers.
BOOST_AUTO_TEST_CASE(AViewHasNoneOfThem)
{
        static_assert(not has_member_swap<vector_span> and not has_get_allocator<vector_span> and not has_replace<vector_span> and not has_extract<vector_span>);
        static_assert(not has_member_swap<set_view> and not has_get_allocator<set_view> and not has_replace<set_view> and not has_extract<set_view>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
