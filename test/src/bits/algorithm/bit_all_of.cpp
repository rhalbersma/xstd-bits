//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>       // for_each_window, patterned
#include <xstd/bits/algorithm/bit_all_of.hpp> // bit_all_of
#include <xstd/bits/bit_array.hpp>            // basic_bit_array, bit_array
#include <xstd/bits/bit_fixed_set.hpp>        // bit_fixed_set
#include <xstd/bits/bit_span.hpp>             // bit_span
#include <xstd/bits/bit_vector.hpp>           // bit_vector
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                          // all_of
#include <cstdint>                            // uint8_t
#include <functional>                         // identity
#include <vector>                             // vector

BOOST_AUTO_TEST_SUITE(BitAllOf)

namespace {

template<class R>
concept quantifiable = requires (R const& r) { xstd::bit_all_of(r); };

// The algorithm and std::ranges::all_of with the identity predicate, on one sequence.
template<class R>
[[nodiscard]] constexpr auto agrees(R const& r)
        -> bool
{
        return xstd::bit_all_of(r) == std::ranges::all_of(r, std::identity());
}

} // namespace

// Empty, clear, full, one position set at either end, and a pattern: across a block boundary and an exact block.
BOOST_AUTO_TEST_CASE(AgreesWithRangesOverOwnersAndViews)
{
        for (auto const n : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 64UZ, 65UZ, 130UZ}) {
                auto first = xstd::bit_vector(n);
                auto last  = xstd::bit_vector(n);
                if (n != 0UZ) {
                        first[0]      = true;
                        last[n - 1UZ] = true;
                }
                BOOST_CHECK(agrees(xstd::bit_vector(n)));
                BOOST_CHECK(agrees(xstd::bit_vector(n, true)));
                BOOST_CHECK(agrees(first) and agrees(last));
                BOOST_CHECK(agrees(test::algorithm::patterned(xstd::bit_vector(n))));
                BOOST_CHECK(agrees(xstd::bit_span(last)));
        }
        static_assert(xstd::bit_all_of(xstd::bit_array<0>()) == true);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

// A window asks its own positions alone, at every offset and length across two narrow blocks, full or patterned.
BOOST_AUTO_TEST_CASE(AWindowAsksItsOwnPositions)
{
        auto full = xstd::basic_bit_array<std::uint8_t, 21>();
        full.fill(true);
        auto wrong = 0;
        for (auto const& a : {test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), full}) {
                test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += static_cast<int>(not agrees(w)); });
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, and over bools of ours alone.
BOOST_AUTO_TEST_CASE(AsksOurSequencesAlone)
{
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>())));
        static_assert(quantifiable<xstd::bit_array<8>> and not quantifiable<std::vector<bool>> and not quantifiable<xstd::bit_fixed_set<8>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
