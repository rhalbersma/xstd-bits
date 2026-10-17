//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/partner.hpp>                                        // partner_t
#include <test/algorithm/sets.hpp>                                           // for_each_set, sets
#include <test/for_each_type.hpp>                                            // for_each_type
#include <xstd/bits/bit_algorithm/bit_lexicographical_compare_three_way.hpp> // bit_lexicographical_compare_three_way
#include <xstd/bits/bit_array.hpp>                                           // bit_array
#include <xstd/bits/bit_functional/bit_greater.hpp>                          // bit_greater
#include <xstd/bits/bit_set/bit_fixed_set.hpp>                               // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>                                     // bit_set
#include <xstd/bits/bit_vector.hpp>                                          // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>                                          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                                                          // invocable, same_as
#include <cstddef>                                                           // size_t
#include <cstdint>                                                           // uint8_t
#include <vector>                                                            // vector

BOOST_AUTO_TEST_SUITE(BitGreater)

namespace {

// Every input of X against every input of Y where bit_greater disagrees with the three-way comparison's sign.
template<class X, class Y>
[[nodiscard]] auto violations(std::vector<X> const& xs, std::vector<Y> const& ys)
        -> int
{
        auto wrong = 0;
        for (auto const& x : xs) {
                for (auto const& y : ys) {
                        wrong += static_cast<int>(xstd::bit_greater()(x, y) != (xstd::bit_lexicographical_compare_three_way(x, y) > 0));
                }
        }
        return wrong;
}

template<class X>
[[nodiscard]] auto inputs()
        -> std::vector<X>
{
        auto nrv = std::vector<X>();
        test::algorithm::for_each_set<X>([&](X const& x) -> void { nrv.push_back(x); });
        return nrv;
}

} // namespace

// Over every pair of inputs of every set column of ours and its kind over the other Block: the comparison > 0.
BOOST_AUTO_TEST_CASE(ItAnswersTheThreeWayComparisonAboveZero)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void { BOOST_CHECK_EQUAL(violations(inputs<X>(), inputs<test::algorithm::partner_t<X>>()), 0); });
}

// Transparent, and over exactly the pairs bit_equal_to takes: one reading and one width kind.
BOOST_AUTO_TEST_CASE(ComparesWhatBitEqualToComparesAlone)
{
        static_assert(std::same_as<xstd::bit_greater::is_transparent, void>);
        static_assert(not xstd::bit_greater()(xstd::bit_fixed_set<70>{1UZ}, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 70>{1UZ, 65UZ}));
        static_assert(std::invocable<xstd::bit_greater, xstd::bit_vector, xstd::basic_bit_vector<std::uint8_t>>);
        static_assert(not std::invocable<xstd::bit_greater, xstd::bit_array<8>, xstd::bit_vector> and not std::invocable<xstd::bit_greater, xstd::bit_fixed_set<8>, xstd::bit_set>);
        static_assert(not std::invocable<xstd::bit_greater, xstd::bit_fixed_set<8>, xstd::bit_array<8>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
