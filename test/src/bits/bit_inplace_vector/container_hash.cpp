//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_inplace_vector.hpp>       // bit_inplace_vector, is_range, is_tuple_like
#include <boost/container_hash/is_range.hpp>      // is_range
#include <boost/container_hash/is_tuple_like.hpp> // is_tuple_like
#include <boost/hash2/hash_append.hpp>            // hash_append
#include <boost/hash2/xxhash.hpp>                 // xxhash_64
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL

BOOST_AUTO_TEST_SUITE(BitInplaceVectorContainerHash)

// Neither range nor tuple to Boost, so Boost.Hash2 appends the value once and equal values append equally.
BOOST_AUTO_TEST_CASE(BoostHashesItAsTheValueItIs)
{
        using T = xstd::bit_inplace_vector<70>;
        static_assert(not boost::container_hash::is_range<T>::value);
        static_assert(not boost::container_hash::is_tuple_like<T>::value);
        auto h1 = boost::hash2::xxhash_64();
        auto h2 = boost::hash2::xxhash_64();
        boost::hash2::hash_append(h1, {}, T({true, false, true}));
        boost::hash2::hash_append(h2, {}, T({true, false, true}));
        BOOST_CHECK_EQUAL(h1.result(), h2.result());
}

BOOST_AUTO_TEST_SUITE_END()
