//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/partner.hpp>            // partner_t
#include <test/algorithm/sequences.hpp>          // for_each_sequence, for_each_window, patterned, sequences
#include <test/algorithm/sets.hpp>               // for_each_set, sets
#include <test/for_each_type.hpp>                // for_each_type
#include <xstd/bits/bit_algorithm/bit_equal.hpp> // bit_equal
#include <xstd/bits/bit_array.hpp>               // basic_bit_array, bit_array
#include <xstd/bits/bit_bounded_vector.hpp>      // basic_bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>           // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_key_mapping.hpp>         // bit_key_mapping
#include <xstd/bits/bit_set.hpp>                 // bit_set
#include <xstd/bits/bit_set_view.hpp>            // bit_set_view
#include <xstd/bits/bit_span.hpp>                // bit_span
#include <xstd/bits/bit_vector.hpp>              // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>              // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                             // equal
#include <cstddef>                               // size_t
#include <cstdint>                               // uint64_t, uint8_t
#include <functional>                            // greater
#include <ranges>                                // range_value_t
#include <vector>                                // vector

BOOST_AUTO_TEST_SUITE(BitEqual)

namespace {

template<class R1, class R2>
concept comparable = requires (R1 const& r1, R2 const& r2) { xstd::bit_equal(r1, r2); };

// The algorithm on r1 and r2 and std::ranges::equal on their elements in iteration order, either way round.
template<class R1, class R2>
[[nodiscard]] constexpr auto agrees(R1 const& r1, R2 const& r2)
        -> bool
{
        auto const model = [](auto const& a, auto const& b) -> bool { return std::ranges::equal(a, b); };
        auto const x     = std::vector<std::ranges::range_value_t<R1>>(r1.begin(), r1.end());
        auto const y     = std::vector<std::ranges::range_value_t<R2>>(r2.begin(), r2.end());
        return xstd::bit_equal(r1, r2) == model(x, y) and xstd::bit_equal(r2, r1) == model(y, x);
}

// Every input of X against every input of Y.
template<class X, class Y>
[[nodiscard]] auto pairwise_disagreements(std::vector<X> const& xs, std::vector<Y> const& ys)
        -> int
{
        auto wrong = 0;
        for (auto const& x : xs) {
                for (auto const& y : ys) {
                        wrong += static_cast<int>(not agrees(x, y));
                }
        }
        return wrong;
}

// Every input of X, collected from for_each.
template<class X, class ForEach>
[[nodiscard]] auto inputs(ForEach for_each)
        -> std::vector<X>
{
        auto nrv = std::vector<X>();
        for_each([&](X const& x) -> void { nrv.push_back(x); });
        return nrv;
}

} // namespace

// Every pair of inputs of every sequence column of ours, against itself and against its kind over the other Block.
BOOST_AUTO_TEST_CASE(AgreesWithRangesEqualOverEverySequenceColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                using Y      = test::algorithm::partner_t<X>;
                auto const x = inputs<X>([](auto f) -> void { test::algorithm::for_each_sequence<X>(f); });
                auto const y = inputs<Y>([](auto f) -> void { test::algorithm::for_each_sequence<Y>(f); });
                BOOST_CHECK_EQUAL(pairwise_disagreements(x, x) + pairwise_disagreements(x, y), 0);
        });
}

// Every pair of inputs of every set column of ours, either direction, and against its kind over the other Block.
BOOST_AUTO_TEST_CASE(AgreesWithRangesEqualOverEverySetColumn)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void {
                using Y      = test::algorithm::partner_t<X>;
                auto const x = inputs<X>([](auto f) -> void { test::algorithm::for_each_set<X>(f); });
                auto const y = inputs<Y>([](auto f) -> void { test::algorithm::for_each_set<Y>(f); });
                BOOST_CHECK_EQUAL(pairwise_disagreements(x, x) + pairwise_disagreements(x, y), 0);
        });
}

// Windows at every offset and length against each other and against whole views, compared a word at a time.
BOOST_AUTO_TEST_CASE(WindowsAndViewsCompareTheirOwnElements)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto const v = test::algorithm::patterned(xstd::basic_bit_vector<std::uint64_t>(21UZ));
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void {
                test::algorithm::for_each_window(xstd::bit_span(v), [&](auto const& u) -> void { wrong += static_cast<int>(not agrees(w, u)); });
        });
        BOOST_CHECK_EQUAL(wrong, 0);
        auto owner = xstd::bit_fixed_set<130>{0UZ, 64UZ, 129UZ};
        BOOST_CHECK(agrees(xstd::bit_set_view(owner), xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 130>{0UZ, 64UZ}));
        BOOST_CHECK(agrees(xstd::bit_span(a), xstd::basic_bit_array<std::uint64_t, 21>()) and agrees(xstd::bit_span(a), test::algorithm::patterned(xstd::basic_bit_array<std::uint64_t, 21>())));
}

// A capacity of nought against a run-time width that holds bools: the sizes still decide.
BOOST_AUTO_TEST_CASE(ZeroCapacityAgainstARunTimeWidth)
{
        auto const none = xstd::basic_bit_bounded_vector<std::uint8_t, 0>();
        BOOST_CHECK(agrees(none, xstd::bit_vector()) and agrees(none, xstd::bit_vector(1UZ)));
}

// In a constant expression, and over two of one reading and one width kind alone, as bit_hasher keeps them apart.
BOOST_AUTO_TEST_CASE(ComparesOneReadingAndOneWidthKindAlone)
{
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>()), test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 70>())));
        static_assert(agrees(xstd::bit_fixed_set<70>{1UZ, 65UZ}, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 70>{1UZ, 66UZ}));
        static_assert(comparable<xstd::bit_array<8>, xstd::basic_bit_array<std::uint8_t, 8>> and comparable<xstd::bit_vector, xstd::basic_bit_vector<std::uint8_t>>);
        static_assert(comparable<xstd::bit_fixed_set<8>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 8>>);
        static_assert(not comparable<xstd::bit_array<8>, xstd::bit_vector> and not comparable<xstd::bit_array<8>, xstd::bit_array<9>> and not comparable<xstd::bit_fixed_set<8>, xstd::bit_set>);
        static_assert(not comparable<xstd::bit_array<8>, xstd::bit_fixed_set<8>> and not comparable<std::vector<bool>, std::vector<bool>>);
        static_assert(not comparable<xstd::bit_fixed_set<8>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 8, xstd::bit_key_mapping<std::size_t>, std::greater<>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
