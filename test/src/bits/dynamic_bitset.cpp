//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bitset_adaptor.hpp>               // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/dynamic_bitset.hpp>               // dynamic_bitset
#include <boost/dynamic_bitset.hpp>                   // dynamic_bitset, to_string
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                  // equal
#include <array>                                      // array
#include <compare>                                    // is_eq, is_gt, is_lt
#include <concepts>                                   // regular, same_as, totally_ordered
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint8_t, uint64_t
#include <functional>                                 // hash
#include <iterator>                                   // back_inserter
#include <memory>                                     // allocator
#include <ranges>                                     // equal, iota
#include <sstream>                                    // istringstream, ostringstream
#include <stdexcept>                                  // invalid_argument, out_of_range, overflow_error
#include <string>                                     // string
#include <tuple>                                      // tuple
#include <utility>                                    // as_const, pair
#include <vector>                                     // vector

BOOST_AUTO_TEST_SUITE(DynamicBitset)

// boost::dynamic_bitset's counterpart over a heap of blocks: the same wrapper, at a run-time width. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE(TheDynamicBitsetIsTheWrapperOverAHeapOfBlocks)
{
        static_assert(std::same_as<xstd::basic_dynamic_bitset<std::uint8_t>, xstd::bitset_adaptor<xstd::detail::bits::contiguous_bit_vector<std::uint8_t>>>);
        static_assert(std::same_as<xstd::basic_dynamic_bitset<std::uint8_t, std::allocator<std::uint8_t>>, xstd::basic_dynamic_bitset<std::uint8_t>>);
        static_assert(std::regular<xstd::basic_dynamic_bitset<std::uint8_t>>);
}

// Ours over a contiguous_bit_vector at two block widths: the counterpart's contract on both.
using Dynamic = std::tuple
<       xstd::basic_dynamic_bitset<std::uint8_t>
,       xstd::basic_dynamic_bitset<std::uint64_t>
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

        // Hashed as boost's counterpart is not, equal values equal. [design.md#the-hashing-invariant]
        BOOST_CHECK_EQUAL(std::hash<T>()(d), std::hash<T>()(T(std::string("000000101"))));
        BOOST_CHECK(std::hash<T>()(d) != std::hash<T>()(T(9)));

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

// The reverse pair at a run-time width: the highest set position below pos, npos where none, a pos past the width meaning from the end.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheReverseSearchesMirrorTheForwardOnes, T, Dynamic)
{
        auto d = T(70);
        d.set(1);
        d.set(69);
        BOOST_CHECK_EQUAL(d.find_last(), 69UZ);
        BOOST_CHECK_EQUAL(d.find_prev(69), 1UZ);
        BOOST_CHECK_EQUAL(d.find_prev(1), T::npos);
        BOOST_CHECK_EQUAL(d.find_prev(T::npos), 69UZ);
        BOOST_CHECK_EQUAL(T(9).find_last(), T::npos);
        BOOST_CHECK_EQUAL(T().find_last(), T::npos);
        BOOST_CHECK_EQUAL(T().find_prev(0), T::npos);
}

// The ordering is boost's, pair for pair: every value at every width up to nine against every other, unequal widths included. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingIsBoosts, T, Dynamic)
{
        static_assert(std::totally_ordered<T>);

        auto values = std::vector<std::pair<T, boost::dynamic_bitset<>>>();
        for (auto const w : std::views::iota(0UZ, 10UZ)) {
                for (auto const v : std::views::iota(0ULL, 1ULL << w)) {
                        values.emplace_back(T(w, v), boost::dynamic_bitset<>(w, static_cast<unsigned long>(v)));
                }
        }
        auto disagreements = 0;
        for (auto const& [ x, bx ] : values) {
                for (auto const& [ y, by ] : values) {
                        auto const cmp = x <=> y;
                        disagreements += static_cast<int>(std::is_lt(cmp) != (bx <  by));
                        disagreements += static_cast<int>(std::is_gt(cmp) != (by <  bx));
                        disagreements += static_cast<int>(std::is_eq(cmp) != (bx == by));
                        disagreements += static_cast<int>(cmp != (x.to_string() <=> y.to_string()));
                }
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

namespace {

// One pair of narrow bitsets against boost's own, at two widths and two patterns; a function rather than a loop body
// so the case that sweeps it stays under readability-function-cognitive-complexity's threshold.
auto disagreements_against_boost(std::size_t w, std::size_t u, unsigned long long p, unsigned long long q)
        -> int
{
        using Narrow = xstd::basic_dynamic_bitset<std::uint8_t>;
        auto x  = Narrow(w, p);
        auto y  = Narrow(u, q);
        auto bx = boost::dynamic_bitset<std::uint8_t>(w, static_cast<unsigned long>(p));
        auto by = boost::dynamic_bitset<std::uint8_t>(u, static_cast<unsigned long>(q));

        // Past the sixty-four bits a constructor takes, so the comparison has blocks above them to walk.
        if (w > 64) { x.set(69); bx.set(69); }
        if (u > 64) { y.set(65); by.set(65); }

        auto const cmp = x <=> y;
        return static_cast<int>(std::is_lt(cmp) != (bx <  by))
             + static_cast<int>(std::is_gt(cmp) != (by <  bx))
             + static_cast<int>(std::is_eq(cmp) != (bx == by));
}

}       // namespace

// And across blocks at unequal widths, where the top windows are read a word at a time at either alignment. [design.md#the-blit]
BOOST_AUTO_TEST_CASE(TheOrderingIsBoostsAcrossBlocksAtUnequalWidths)
{
        constexpr auto widths   = std::array{ 0UZ, 3UZ, 8UZ, 9UZ, 16UZ, 17UZ, 25UZ, 70UZ };
        constexpr auto patterns = std::array{ 0ULL, 1ULL, 0b1010'1010ULL, 0b1'0000'0000ULL, 0xFFFFULL, 0x8001ULL, 0x1F'FFFFULL };

        auto wide = 0;
        for (auto const w : widths) {
                for (auto const u : widths) {
                        for (auto const p : patterns) {
                                for (auto const q : patterns) {
                                        wide += disagreements_against_boost(w, u, p, q);
                                }
                        }
                }
        }
        BOOST_CHECK_EQUAL(wide, 0);
}

// boost's block interface: the block-range constructor, every block out, at most every block in.
BOOST_AUTO_TEST_CASE(TheBlockInterfaceIsBoosts)
{
        using T = xstd::basic_dynamic_bitset<std::uint8_t>;
        static_assert(std::same_as<T::block_type, std::uint8_t>);
        static_assert(T::bits_per_block == 8UZ);

        auto const blocks = std::array<std::uint8_t, 2>{ 0b1000'0001, 0b11 };
        auto const d = T(blocks.begin(), blocks.end());
        auto const b = boost::dynamic_bitset<std::uint8_t>(blocks.begin(), blocks.end());
        BOOST_CHECK_EQUAL(d.size(), 16UZ);
        BOOST_CHECK_EQUAL(d.num_blocks(), 2UZ);
        BOOST_CHECK_EQUAL(d.count(), 4UZ);
        auto s = std::string();
        boost::to_string(b, s);
        BOOST_CHECK_EQUAL(d.to_string(), s);

        auto out = std::vector<std::uint8_t>();
        to_block_range(d, std::back_inserter(out));
        BOOST_CHECK(std::ranges::equal(out, blocks));

        auto e = T(16);
        from_block_range(blocks.begin(), blocks.end(), e);
        BOOST_CHECK(e == d);

        // The two-argument form stays the width-and-value constructor, as boost's dispatch keeps it.
        BOOST_CHECK_EQUAL(T(3, 7).to_ullong(), 7ULL);
}

// The rest of boost's surface, first the allocator and max_size. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheAllocatorAndMaxSizeAreBoosts, T, Dynamic)
{
        using Block = T::block_type;
        using Boost = boost::dynamic_bitset<Block>;
        static_assert(std::same_as<typename T::allocator_type, std::allocator<Block>>);

        auto const alloc = std::allocator<Block>();
        auto const a = T(alloc);
        BOOST_CHECK(a.empty());
        BOOST_CHECK(a.get_allocator() == alloc);
        auto const b = T(9, 0b101ULL, alloc);
        BOOST_CHECK_EQUAL(b.to_ullong(), 5ULL);
        auto const blocks = std::array<Block, 2>{ 1, 2 };
        auto const c = T(blocks.begin(), blocks.end(), alloc);
        BOOST_CHECK_EQUAL(c.num_blocks(), 2UZ);

        // A whole number of blocks, no larger than boost's bound over the same blocks.
        BOOST_CHECK_EQUAL(b.max_size() % T::bits_per_block, 0UZ);
        BOOST_CHECK_GE(b.max_size(), b.size());
        BOOST_CHECK_LE(b.max_size(), Boost(9).max_size());
}

// Then the throwing at and test_set. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE_TEMPLATE(AtAndTestSetAreBoosts, T, Dynamic)
{
        auto d = T(9, 0b101ULL);
        BOOST_CHECK_EQUAL(d.at(0), true);
        BOOST_CHECK_EQUAL(std::as_const(d).at(1), false);
        d.at(1) = true;
        BOOST_CHECK(d.test(1));
        BOOST_CHECK_THROW(static_cast<void>(d.at(9)), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(std::as_const(d).at(9)), std::out_of_range);

        BOOST_CHECK_EQUAL(d.test_set(1, false), true);
        BOOST_CHECK_EQUAL(d.test_set(1), false);
        BOOST_CHECK(d.test(1));
}

// The ranged forms against boost's, across a block boundary and up to the last position.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheRangedFormsAreBoosts, T, Dynamic)
{
        using Boost = boost::dynamic_bitset<typename T::block_type>;

        for (auto const& [ pos, len ] : { std::pair{ 0UZ, 0UZ }, std::pair{ 3UZ, 4UZ }, std::pair{ 6UZ, 14UZ }, std::pair{ 0UZ, 20UZ } }) {
                auto ours = T(20, 0b1010'1010'1010'1010'1010ULL);
                auto theirs = Boost(20, 0b1010'1010'1010'1010'1010UL);
                ours.set(pos, len, true); theirs.set(pos, len, true);
                BOOST_CHECK_EQUAL(ours.to_ullong(), theirs.to_ulong());
                ours.flip(pos, len); theirs.flip(pos, len);
                BOOST_CHECK_EQUAL(ours.to_ullong(), theirs.to_ulong());
                ours.reset(pos, len); theirs.reset(pos, len);
                BOOST_CHECK_EQUAL(ours.to_ullong(), theirs.to_ulong());
        }
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
        using T = xstd::basic_dynamic_bitset<std::uint8_t>;
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
