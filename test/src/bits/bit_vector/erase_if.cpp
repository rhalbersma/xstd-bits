//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_vector.hpp> // bit_vector, erase, erase_if
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                // ranges::equal
#include <vector>                   // erase, erase_if, vector

BOOST_AUTO_TEST_SUITE(BitVectorEraseIf)

// [vector.erasure]: each value removed, counted and closed up, as from a std::vector<bool> of the same bools.
BOOST_AUTO_TEST_CASE(ItErasesAsAStdVectorOfBoolDoes)
{
        auto const bools = std::vector<bool>({true, false, true, true, false, false, true});
        for (auto const value : {false, true}) {
                auto x = xstd::bit_vector(bools.begin(), bools.end());
                auto m = bools;
                BOOST_CHECK_EQUAL(erase(x, value), std::erase(m, value));
                BOOST_CHECK(std::ranges::equal(x, m));
        }
        auto x         = xstd::bit_vector(bools.begin(), bools.end());
        auto m         = bools;
        auto const odd = [n = 0UZ](bool) mutable -> bool { return n++ % 2UZ == 1UZ; };
        BOOST_CHECK_EQUAL(erase_if(x, odd), std::erase_if(m, odd));
        BOOST_CHECK(std::ranges::equal(x, m));
}

BOOST_AUTO_TEST_SUITE_END()
