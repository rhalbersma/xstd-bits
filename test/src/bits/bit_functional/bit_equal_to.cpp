//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/partner.hpp>                // partner_t
#include <test/algorithm/sets.hpp>                   // for_each_set, sets
#include <test/for_each_type.hpp>                    // for_each_type
#include <xstd/bits/bit_array.hpp>                   // bit_array
#include <xstd/bits/bit_functional/bit_equal_to.hpp> // bit_equal_to
#include <xstd/bits/bit_functional/bit_hash.hpp>     // bit_hash
#include <xstd/bits/bit_set/bit_fixed_set.hpp>       // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>             // basic_bit_set, bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>        // bit_set_view
#include <xstd/bits/bit_vector.hpp>                  // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                 // equal
#include <concepts>                                  // invocable, same_as
#include <cstddef>                                   // size_t
#include <cstdint>                                   // uint8_t
#include <unordered_set>                             // unordered_set
#include <vector>                                    // vector

BOOST_AUTO_TEST_SUITE(BitEqualTo)

namespace {

// Whatever bit_equal_to calls equal, bit_hash hashes alike, and it calls equal exactly what holds the same elements.
template<class X, class Y>
[[nodiscard]] auto keeps_the_invariant(X const& x, Y const& y)
        -> bool
{
        auto const equal = xstd::bit_equal_to()(x, y);
        return equal == std::ranges::equal(x, y) and (not equal or xstd::bit_hash<>()(x) == xstd::bit_hash<>()(y));
}

// Every input of X against every input of Y.
template<class X, class Y>
[[nodiscard]] auto violations(std::vector<X> const& xs, std::vector<Y> const& ys)
        -> int
{
        auto wrong = 0;
        for (auto const& x : xs) {
                for (auto const& y : ys) {
                        wrong += static_cast<int>(not keeps_the_invariant(x, y));
                }
        }
        return wrong;
}

template<class X>
[[nodiscard]] auto inputs()
        -> std::vector<X>
{
        auto nrv = std::vector<X>();
        test::algorithm::for_each_set<X>([&](X const& x) -> void { nrv.push_back(x); });
        return nrv;
}

} // namespace

// Over every pair of inputs of every set column of ours and its kind over the other Block: equal values hash equal.
BOOST_AUTO_TEST_CASE(WhatItCallsEqualBitHashHashesAlike)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void { BOOST_CHECK_EQUAL(violations(inputs<X>(), inputs<test::algorithm::partner_t<X>>()), 0); });
}

// A lookup crosses block types and storages, a set found through a view and through narrower blocks.
BOOST_AUTO_TEST_CASE(ALookupCrossesBlockTypesAndStorages)
{
        auto const sets = std::unordered_set<xstd::bit_fixed_set<100>, xstd::bit_hash<>, xstd::bit_equal_to>({{1UZ, 64UZ}, {99UZ}});
        auto owner      = xstd::bit_fixed_set<100>{99UZ};
        BOOST_CHECK(sets.contains(xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 100>{1UZ, 64UZ}));
        BOOST_CHECK(sets.contains(xstd::bit_set_view(owner)));
        BOOST_CHECK(not sets.contains(xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 100>{1UZ}));

        auto const vectors = std::unordered_set<xstd::bit_vector, xstd::bit_hash<>, xstd::bit_equal_to>({xstd::bit_vector(9UZ, true), xstd::bit_vector(70UZ)});
        BOOST_CHECK(vectors.contains(xstd::basic_bit_vector<std::uint8_t>(9UZ, true)));
        BOOST_CHECK(not vectors.contains(xstd::basic_bit_vector<std::uint8_t>(10UZ, true)));

        auto const dynamic = std::unordered_set<xstd::bit_set, xstd::bit_hash<>, xstd::bit_equal_to>({{3UZ, 300UZ}});
        BOOST_CHECK(dynamic.contains(xstd::basic_bit_set<std::size_t, std::uint8_t>{3UZ, 300UZ}));
}

// Transparent, and never a static width against a run-time one, which bit_hash keeps apart.
BOOST_AUTO_TEST_CASE(ComparesOneReadingAndOneWidthKindAlone)
{
        static_assert(std::same_as<xstd::bit_equal_to::is_transparent, void>);
        static_assert(xstd::bit_equal_to()(xstd::bit_fixed_set<70>{1UZ, 65UZ}, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 70>{1UZ, 65UZ}));
        static_assert(std::invocable<xstd::bit_equal_to, xstd::bit_vector, xstd::basic_bit_vector<std::uint8_t>>);
        static_assert(not std::invocable<xstd::bit_equal_to, xstd::bit_array<8>, xstd::bit_vector> and not std::invocable<xstd::bit_equal_to, xstd::bit_fixed_set<8>, xstd::bit_set>);
        static_assert(not std::invocable<xstd::bit_equal_to, xstd::bit_fixed_set<8>, xstd::bit_array<8>> and not std::invocable<xstd::bit_equal_to, std::vector<bool>, std::vector<bool>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
