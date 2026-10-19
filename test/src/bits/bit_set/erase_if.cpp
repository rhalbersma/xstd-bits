//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set.hpp>    // bit_fixed_set, bit_inplace_set, bit_set, erase_if
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                // ranges::equal
#include <cstddef>                  // size_t
#include <set>                      // erase_if, set

BOOST_AUTO_TEST_SUITE(BitSetEraseIf)

// [set.erasure]: the keys a predicate names removed and counted, as from a std::set of the same keys.
BOOST_AUTO_TEST_CASE(ItErasesAsAStdSetDoes)
{
        auto const odd = [](std::size_t k) -> bool { return k % 2UZ == 1UZ; };
        auto m         = std::set<std::size_t>({1UZ, 2UZ, 3UZ, 64UZ, 65UZ});
        auto s         = xstd::bit_set({1UZ, 2UZ, 3UZ, 64UZ, 65UZ});
        auto f         = xstd::bit_fixed_set<70>({1UZ, 2UZ, 3UZ, 64UZ, 65UZ});
        auto i         = xstd::bit_inplace_set<70>({1UZ, 2UZ, 3UZ, 64UZ, 65UZ});
        auto const n   = std::erase_if(m, odd);
        BOOST_CHECK_EQUAL(erase_if(s, odd), n);
        BOOST_CHECK_EQUAL(erase_if(f, odd), n);
        BOOST_CHECK_EQUAL(erase_if(i, odd), n);
        BOOST_CHECK(std::ranges::equal(s, m) and std::ranges::equal(f, m) and std::ranges::equal(i, m));
}

BOOST_AUTO_TEST_SUITE_END()
