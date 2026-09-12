//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bitset.hpp>      // bitset
#include <xstd/bits/detail/hash.hpp> // std_hash
#include <boost/hash2/fnv1a.hpp>     // fnv1a_32, fnv1a_64
#include <boost/hash2/xxhash.hpp>    // xxhash_64
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstdint>                   // uint64_t
#include <functional>                // hash

// std_hash's Hash parameter is the one thing std::hash cannot reach, so it is asserted here rather than through a specialization. [design.md#the-hashing-invariant]
BOOST_AUTO_TEST_SUITE(DetailHash)

using set_type = xstd::bitset<8>;

BOOST_AUTO_TEST_CASE(TheDefaultIsFnv1a64)
{
        auto const value = set_type(0b1010'0101);
        BOOST_CHECK_EQUAL(xstd::detail::bits::std_hash(value), xstd::detail::bits::std_hash(value, boost::hash2::fnv1a_64()));

        // And it is the default std::hash takes, since its operator() has no second argument to pass. [design.md#the-hashing-invariant]
        BOOST_CHECK_EQUAL(std::hash<set_type>()(value), xstd::detail::bits::std_hash(value, boost::hash2::fnv1a_64()));
}

BOOST_AUTO_TEST_CASE(ASeededInstanceSubstitutes)
{
        auto const value = set_type(0b1010'0101);
        constexpr auto seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

        // By value, not by type alone: this is what a defaulted template parameter on its own could not express.
        BOOST_CHECK_EQUAL(
                xstd::detail::bits::std_hash(value, boost::hash2::fnv1a_64(seed)),
                xstd::detail::bits::std_hash(value, boost::hash2::fnv1a_64(seed))
        );
        BOOST_CHECK(xstd::detail::bits::std_hash(value, boost::hash2::fnv1a_64(seed)) != xstd::detail::bits::std_hash(value));
}

BOOST_AUTO_TEST_CASE(AnotherAlgorithmSubstitutes)
{
        auto const value = set_type(0b1010'0101);
        BOOST_CHECK(xstd::detail::bits::std_hash(value, boost::hash2::xxhash_64()) != xstd::detail::bits::std_hash(value));
        BOOST_CHECK(xstd::detail::bits::std_hash(value, boost::hash2::fnv1a_32()) != xstd::detail::bits::std_hash(value));
}

// The invariant is per algorithm, and holds under a substituted one too: equal values hash equal. [design.md#the-hashing-invariant]
BOOST_AUTO_TEST_CASE(EqualValuesHashEqualUnderASubstitutedHash)
{
        auto const lhs = set_type(0b1010'0101);
        auto const rhs = set_type(0b1010'0101);
        auto const other = set_type(0b0101'1010);

        BOOST_CHECK_EQUAL(
                xstd::detail::bits::std_hash(lhs, boost::hash2::xxhash_64()),
                xstd::detail::bits::std_hash(rhs, boost::hash2::xxhash_64())
        );
        BOOST_CHECK(
                xstd::detail::bits::std_hash(lhs, boost::hash2::xxhash_64()) !=
                xstd::detail::bits::std_hash(other, boost::hash2::xxhash_64())
        );
}

BOOST_AUTO_TEST_SUITE_END()
