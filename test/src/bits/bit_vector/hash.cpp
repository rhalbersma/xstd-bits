//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_span.hpp>   // bit_span
#include <xstd/bits/bit_vector.hpp> // basic_bit_vector, hash
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstdint>                  // uint8_t
#include <functional>               // hash
#include <type_traits>              // is_default_constructible_v
#include <vector>                   // vector

BOOST_AUTO_TEST_SUITE(BitVectorHash)

using T = xstd::basic_bit_vector<std::uint8_t>;

// The owner hashes as std::vector<bool> does, equal values equal; the view over it no more than std::span does.
BOOST_AUTO_TEST_CASE(TheOwnerHashesAndTheViewDoesNot)
{
        auto const h = std::hash<T>();
        BOOST_CHECK_EQUAL(h(T({true, false, true})), h(T({true, false, true})));
        BOOST_CHECK(h(T({true, false, true})) != h(T({true, false, true, false})));
        BOOST_CHECK(h(T()) != h(T(1)));
        static_assert(not std::is_default_constructible_v<std::hash<xstd::bit_span<std::vector<std::uint8_t>>>>);
}

BOOST_AUTO_TEST_SUITE_END()
