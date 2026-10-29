//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>  // bit_array
#include <xstd/bits/bit_ranges.hpp> // from_blocks, from_blocks_t
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // same_as
#include <cstdint>                  // uint8_t

BOOST_AUTO_TEST_SUITE(Ranges)

// The umbrella reaches the tag in one include, and an owner reads a block's bits through it.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheTag)
{
        static_assert(std::same_as<decltype(xstd::from_blocks), xstd::from_blocks_t const>);
        static_assert([] -> bool {
                auto const a = xstd::bit_array<8>(xstd::from_blocks, std::uint8_t{0b101});
                return a[0] and not a[1] and a[2];
        }());
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
