//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_functional.hpp>        // bit_equal_to, bit_hash, bit_hash_append, bit_not_equal_to
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // basic_bit_fixed_set, bit_fixed_set
#include <boost/hash2/flavor.hpp>              // default_flavor
#include <boost/hash2/xxhash.hpp>              // xxhash_64
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstddef>                             // size_t
#include <cstdint>                             // uint8_t

BOOST_AUTO_TEST_SUITE(Functional)

// The umbrella reaches the equality, its negation and the hash in one include, and they agree across block types.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheEqualityAndTheHash)
{
        auto const x = xstd::bit_fixed_set<70>{1UZ, 65UZ};
        auto const y = xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 70>{1UZ, 65UZ};
        BOOST_CHECK(xstd::bit_equal_to()(x, y) and not xstd::bit_not_equal_to()(x, y));
        auto const hash = xstd::bit_hash<>();
        BOOST_CHECK_EQUAL(hash(x), hash(y));
        auto hx = boost::hash2::xxhash_64();
        auto hy = boost::hash2::xxhash_64();
        xstd::bit_hash_append(hx, boost::hash2::default_flavor(), x);
        xstd::bit_hash_append(hy, boost::hash2::default_flavor(), y);
        BOOST_CHECK_EQUAL(hx.result(), hy.result());
}

BOOST_AUTO_TEST_SUITE_END()
