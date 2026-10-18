//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>        // for_each_sequence, for_each_window, patterned, sequences
#include <test/algorithm/sets.hpp>             // for_each_set, iterators_of, sets, stops
#include <test/for_each_type.hpp>              // for_each_type
#include <xstd/bits/bit_array.hpp>             // basic_bit_array, bit_array
#include <xstd/bits/bit_iterator/bit_next.hpp> // bit_next
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // bit_fixed_set
#include <xstd/bits/bit_span/bit_span.hpp>     // bit_span
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstddef>                             // ptrdiff_t, size_t
#include <cstdint>                             // uint8_t
#include <iterator>                            // next
#include <ranges>                              // iterator_t
#include <set>                                 // set
#include <vector>                              // vector

BOOST_AUTO_TEST_SUITE(BitNext)

namespace {

template<class I>
concept steppable = requires (I it) { xstd::ranges::bit_next(it, 1); };

// From each stop to each one forward of it, unbounded, and bounded by every stop at every distance.
template<class X>
[[nodiscard]] auto disagreements(X const& s)
        -> int
{
        auto const its = test::algorithm::iterators_of(s);
        auto const n   = its.size() - 1UZ;
        auto wrong     = 0;
        for (auto const i : test::algorithm::stops(n)) {
                for (auto const j : test::algorithm::stops(n)) {
                        if (i <= j) {
                                wrong += static_cast<int>(xstd::ranges::bit_next(its[i], static_cast<std::ptrdiff_t>(j - i)) != its[j]);
                        }
                        for (auto const steps : {0Z, 1Z, 9Z, 200Z}) {
                                if (i <= j) {
                                        wrong += static_cast<int>(xstd::ranges::bit_next(its[i], steps, its[j]) != std::ranges::next(its[i], steps, its[j]));
                                }
                        }
                }
        }
        return wrong;
}

} // namespace

// Every input of every set column of ours, ascending and descending, against stepping one key at a time.
BOOST_AUTO_TEST_CASE(AgreesWithRangesNextOverEverySetColumn)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_set<X>([&](X const& x) -> void { wrong += disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// Every input of every sequence column of ours, a span of each and every window, as std::ranges::next steps them.
BOOST_AUTO_TEST_CASE(AgreesWithRangesNextOverEverySequenceColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x) + disagreements(xstd::bit_span(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, and over our iterators alone.
BOOST_AUTO_TEST_CASE(StepsOurIteratorsAlone)
{
        static_assert([] -> bool {
                auto const s = xstd::bit_fixed_set<200>{3UZ, 64UZ, 150UZ, 199UZ};
                return *xstd::ranges::bit_next(s.begin(), 2) == 150UZ and xstd::ranges::bit_next(s.begin(), 9, s.end()) == s.end();
        }());
        static_assert(steppable<std::ranges::iterator_t<xstd::bit_fixed_set<8>>> and steppable<std::ranges::iterator_t<xstd::bit_array<8>>>);
        static_assert(not steppable<std::vector<bool>::iterator> and not steppable<std::set<std::size_t>::iterator>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
