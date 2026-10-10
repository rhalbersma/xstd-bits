//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>      // for_each_window, patterned
#include <xstd/bits/algorithm/bit_count.hpp> // bit_count
#include <xstd/bits/bit_array.hpp>           // basic_bit_array, bit_array
#include <xstd/bits/bit_fixed_set.hpp>       // bit_fixed_set
#include <xstd/bits/bit_span.hpp>            // bit_span
#include <xstd/bits/bit_vector.hpp>          // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                         // count
#include <concepts>                          // same_as
#include <cstddef>                           // ptrdiff_t
#include <cstdint>                           // uint8_t
#include <vector>                            // vector

BOOST_AUTO_TEST_SUITE(BitCount)

namespace {

template<class R>
concept countable = requires (R const& r) { xstd::bit_count(r); };

} // namespace

// std::ranges::count(r, true) over owners and a whole view, whose padding it never counts.
BOOST_AUTO_TEST_CASE(CountsAsRangesCountTrue)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        BOOST_CHECK_EQUAL(xstd::bit_count(a), std::ranges::count(a, true));
        BOOST_CHECK_EQUAL(xstd::bit_count(xstd::bit_span(a)), std::ranges::count(a, true));

        for (auto const n : {0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 130UZ}) {
                auto const v = test::algorithm::patterned(xstd::bit_vector(n));
                BOOST_CHECK_EQUAL(xstd::bit_count(v), std::ranges::count(v, true));
                BOOST_CHECK_EQUAL(xstd::bit_count(xstd::bit_vector(n, true)), static_cast<std::ptrdiff_t>(n));
        }
}

// A window counts its own positions alone, at every offset and length across two narrow blocks.
BOOST_AUTO_TEST_CASE(AWindowCountsItsOwnPositions)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += static_cast<int>(xstd::bit_count(w) != std::ranges::count(w, true)); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// The difference type, as std::ranges::count answers, in a constant expression too, and over bools of ours alone.
BOOST_AUTO_TEST_CASE(AnswersTheDifferenceTypeOverOurSequencesAlone)
{
        static_assert(std::same_as<decltype(xstd::bit_count(xstd::bit_array<8>())), std::ptrdiff_t>);
        static_assert(xstd::bit_count(test::algorithm::patterned(xstd::bit_array<70>())) == std::ranges::count(test::algorithm::patterned(std::vector<bool>(70)), true));
        static_assert(countable<xstd::bit_vector> and countable<xstd::basic_bit_vector<std::uint8_t>>);
        static_assert(not countable<std::vector<bool>> and not countable<xstd::bit_fixed_set<8>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
