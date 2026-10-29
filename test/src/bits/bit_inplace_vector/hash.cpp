//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_inplace_vector.hpp> // bit_inplace_vector, hash
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <functional>                       // hash

BOOST_AUTO_TEST_SUITE(BitInplaceVectorHash)

// Every owner hashes, this one although std::inplace_vector does not: equal values equal, and the width counts.
BOOST_AUTO_TEST_CASE(ItHashesAsAnOwner)
{
        using T      = xstd::bit_inplace_vector<70>;
        auto const h = std::hash<T>();
        BOOST_CHECK_EQUAL(h(T({true, false, true})), h(T({true, false, true})));
        BOOST_CHECK(h(T({true, false, true})) != h(T({true, false, true, false})));
        BOOST_CHECK(h(T()) != h(T(1)));
}

BOOST_AUTO_TEST_SUITE_END()
