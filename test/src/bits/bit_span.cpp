//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_span.hpp>   // bit_span, bit_subspan
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                    // array
#include <concepts>                 // same_as
#include <cstdint>                  // uint64_t
#include <span>                     // dynamic_extent, span

BOOST_AUTO_TEST_SUITE(Span)

// The umbrella reaches the whole span and the window its subspan returns in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheSpanAndItsWindow)
{
        auto blocks     = std::array<std::uint64_t, 1>{0b1010};
        auto const span = xstd::bit_span(blocks);
        using Blocks    = std::span<std::uint64_t, 1>;
        static_assert(std::same_as<decltype(span), xstd::bit_span<Blocks, 64> const>);
        static_assert(std::same_as<decltype(span.subspan(1UZ, 2UZ)), xstd::bit_subspan<Blocks, std::dynamic_extent, 64>>);
        BOOST_CHECK(span.subspan(1UZ, 2UZ)[0]);
}

BOOST_AUTO_TEST_SUITE_END()
