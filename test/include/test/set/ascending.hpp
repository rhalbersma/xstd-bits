//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_ASCENDING_HPP
#define TEST_SET_ASCENDING_HPP

#include <boost/test/unit_test.hpp> // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_LT
#include <algorithm>                // is_sorted
#include <cstddef>                  // size_t
#include <optional>                 // optional

// Its own header rather than a corner of set/ordering.hpp, which reaches for bit_set_view and make_bitset that this needs none of: a container's test should not pay for the view's machinery to ask one question.
namespace test::set {

// The set reading yields its keys in ASCENDING order, at every width and for every storage. [design.md#two-readings-disagree]
template<class C>
auto yields_ascending_keys(C const& c)
        -> void
{
        BOOST_CHECK(std::ranges::is_sorted(c));

        auto previous = std::optional<std::size_t>();
        auto counted = 0UZ;
        for (auto const key : c) {
                auto const current = static_cast<std::size_t>(key);
                if (previous.has_value()) {
                        BOOST_CHECK_LT(previous.value(), current);
                }
                previous = current;
                ++counted;
        }
        BOOST_CHECK_EQUAL(counted, c.size());
}

}       // namespace test::set

#endif  // TEST_SET_ASCENDING_HPP
