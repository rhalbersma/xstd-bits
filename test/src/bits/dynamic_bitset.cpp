//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <xstd/bits/basic_bitset.hpp>             // basic_bitset
#include <xstd/bits/block_sequence.hpp>           // block_vector
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_traits over boost::dynamic_bitset
#include <array>                                  // array
#include <concepts>                               // regular, same_as
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t, uint64_t
#include <memory>                                 // allocator
#include <sstream>                                // istringstream, ostringstream
#include <stdexcept>                              // invalid_argument, out_of_range, overflow_error
#include <string>                                 // string
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(DynamicBitset)

// boost::dynamic_bitset's counterpart over a heap of blocks: the same wrapper, at a run-time width. [design.md#the-idempotent-wrapper]
BOOST_AUTO_TEST_CASE(TheDynamicBitsetIsTheWrapperOverAHeapOfBlocks)
{
        static_assert(std::same_as<xstd::dynamic_bitset<std::uint8_t>, xstd::basic_bitset<xstd::block_vector<std::uint8_t>>>);
        static_assert(std::same_as<xstd::dynamic_bitset<std::uint8_t, std::allocator<std::uint8_t>>, xstd::dynamic_bitset<std::uint8_t>>);
        static_assert(std::regular<xstd::dynamic_bitset<std::uint8_t>>);
}

// Ours over a block_vector, ours over boost itself: the counterpart's contract on both.
using Dynamic = std::tuple
<       xstd::dynamic_bitset<std::uint8_t>
,       xstd::dynamic_bitset<std::uint64_t>
,       xstd::basic_bitset<boost::dynamic_bitset<>>
>;

// The width-and-value constructor, the searches with boost's sentinel, and the set vocabulary boost has.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItAnswersAsBoostDoes, T, Dynamic)
{
        auto d = T(9, 0b101ULL);
        BOOST_CHECK_EQUAL(d.size(), 9UZ);
        BOOST_CHECK_EQUAL(d.count(), 2UZ);
        BOOST_CHECK_EQUAL(d.to_ullong(), 5ULL);
        BOOST_CHECK_EQUAL(d.to_ulong(), 5UL);
        BOOST_CHECK(not d.empty());

        BOOST_CHECK_EQUAL(d.find_first(), 0UZ);
        BOOST_CHECK_EQUAL(d.find_next(0), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(2), T::npos);
        BOOST_CHECK_EQUAL(T(9).find_first(), T::npos);

        auto e = T(9);
        e.set(2);
        BOOST_CHECK(e.is_subset_of(d));
        BOOST_CHECK(e.is_proper_subset_of(d));
        BOOST_CHECK(d.intersects(e));
        BOOST_CHECK(not e.is_proper_subset_of(e));

        auto const f = d - e;
        d -= e;
        BOOST_CHECK(f == d);
        BOOST_CHECK_EQUAL(d.count(), 1UZ);
}

// Growth, boost's members: resize with either fill, push and pop, append a block and a range, reserve, shrink, clear.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItGrowsAsBoostDoes, T, Dynamic)
{
        auto d = T(9, 0b101ULL);
        d.resize(70, true);
        BOOST_CHECK_EQUAL(d.size(), 70UZ);
        BOOST_CHECK_EQUAL(d.count(), 63UZ);
        BOOST_CHECK_THROW(static_cast<void>(d.to_ullong()), std::overflow_error);

        d.resize(9);
        BOOST_CHECK_EQUAL(d.count(), 2UZ);
        d.push_back(true);
        BOOST_CHECK_EQUAL(d.size(), 10UZ);
        BOOST_CHECK(d.test(9));
        d.pop_back();
        BOOST_CHECK_EQUAL(d.size(), 9UZ);

        d.reserve(100);
        BOOST_CHECK_GE(d.capacity(), 100UZ);
        d.shrink_to_fit();
        BOOST_CHECK_GE(d.capacity(), d.size());

        d.clear();
        BOOST_CHECK(d.empty());
        BOOST_CHECK_EQUAL(d.size(), 0UZ);
}

// A run-time width is as wide as the text: the constructors and the extractor read every character, as boost's do.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItIsAsWideAsItsText, T, Dynamic)
{
        auto const s = T(std::string("0101"));
        BOOST_CHECK_EQUAL(s.size(), 4UZ);
        BOOST_CHECK_EQUAL(s.to_ullong(), 5ULL);
        BOOST_CHECK_EQUAL(s.to_string(), "0101");

        auto in = std::istringstream("1101x");
        auto r = T();
        in >> r;
        BOOST_CHECK_EQUAL(r.size(), 4UZ);
        BOOST_CHECK_EQUAL(r.to_ullong(), 13ULL);
        BOOST_CHECK(not in.fail());

        auto out = std::ostringstream();
        out << r;
        BOOST_CHECK_EQUAL(out.str(), "1101");

        auto bad = std::istringstream("x");
        auto q = T();
        bad >> q;
        BOOST_CHECK(bad.fail());

        // The text constructor's two throws, as [bitset.cons]/3-4 has them at a static width.
        BOOST_CHECK_THROW(static_cast<void>(T(std::string("0101"), 5)), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(T(std::string("0x01"))),   std::invalid_argument);
}

// Appending blocks is the storage's own where it has it: ours has, boost has, and the widths agree.
BOOST_AUTO_TEST_CASE(AppendingBlocksWidensByAWord)
{
        using T = xstd::dynamic_bitset<std::uint8_t>;
        auto d = T(3, 0b111ULL);
        d.append(std::uint8_t{0b1});
        BOOST_CHECK_EQUAL(d.size(), 11UZ);
        BOOST_CHECK(d.test(3));
        BOOST_CHECK_EQUAL(d.count(), 4UZ);

        auto const more = std::array<std::uint8_t, 2>{ 0b11, 0b100 };
        d.append(more.begin(), more.end());
        BOOST_CHECK_EQUAL(d.size(), 27UZ);
        BOOST_CHECK_EQUAL(d.count(), 7UZ);
        BOOST_CHECK(d.test(11) and d.test(12) and d.test(21));
}

BOOST_AUTO_TEST_SUITE_END()
