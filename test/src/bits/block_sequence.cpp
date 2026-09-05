//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>    // BOOST_CHECK_EQUAL, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/block_types.hpp>        // graded_extents, word_types
#include <xstd/bits/bit_traits.hpp>    // bit_storage, bit_traits, block_readable, static_bit_extent
#include <xstd/bits/block_sequence.hpp> // block_array, block_sequence, block_storage, block_vector
#include <algorithm>                   // count, lexicographical_compare_three_way, min
#include <array>                       // array
#include <compare>                     // strong_ordering
#include <cstddef>                     // size_t
#include <cstdint>                     // uint8_t, uint64_t
#include <ranges>                      // iota
#include <utility>                     // pair
#include <vector>                      // vector

BOOST_AUTO_TEST_SUITE(BitBlocks)

namespace {

// std::vector<bool> is the reference reading: one bool per bit, no packing, no invariant to get wrong.
using model = std::vector<bool>;

template<class BB>
auto reference(BB const& b) -> model
{
        auto m = model(b.size());
        for (auto i = 0UZ; i < b.size(); ++i) {
                m[i] = b.test(i);
        }
        return m;
}

// One family of checks per member, disagreements counted rather than asserted. [design.md#counted-not-asserted]
template<class BB>
class checker
{
        BB const& m_x;
        BB const& m_y;
        model m_mx = reference(m_x);
        model m_my = reference(m_y);
        std::size_t m_n = m_x.size();
        std::size_t m_cardinality = static_cast<std::size_t>(std::ranges::count(m_mx, true));
        int& m_disagreements;

        // Two scratch objects by reference: copies per check trip three GCC diagnostics. [design.md#scratch-objects]
        BB& m_a;
        BB& m_b;

        auto fresh_x() -> BB&
        {
                m_a = m_x;
                return m_a;
        }

        auto fresh_y() -> BB&
        {
                m_b = m_y;
                return m_b;
        }

        // The two sites where a comparison becomes a count, so the cast is not thirty. [design.md#counted-not-asserted]
        auto disagree(bool ours, bool theirs) -> void
        {
                m_disagreements += static_cast<int>(ours != theirs);
        }

        auto unequal(std::size_t ours, std::size_t theirs) -> void
        {
                m_disagreements += static_cast<int>(ours != theirs);
        }

        // Position by position against a model of what the operation should have left.
        auto same(model const& m, BB const& got) -> void
        {
                for (auto i = 0UZ; i < m_n; ++i) {
                        disagree(got.test(i), m[i]);
                }
        }

public:
        checker(BB const& x, BB const& y, BB& a, BB& b, int& disagreements)
        :
                m_x(x),
                m_y(y),
                m_disagreements(disagreements),
                m_a(a),
                m_b(b)
        {}

        auto width() -> void
        {
                unequal(m_x.count(), m_cardinality);
                disagree(m_x.any(),  m_cardinality != 0);
                disagree(m_x.none(), m_cardinality == 0);
                disagree(m_x.all(),  m_cardinality == m_n);
                disagree(m_x == m_y, m_mx == m_my);
        }

        // find_front/find_back assert any(); find_first/find_last are total and answer size().
        auto scans() -> void
        {
                if (m_cardinality != 0) {
                        auto front = 0UZ;
                        while (not m_mx[front]) { ++front; }
                        auto back = m_n - 1;
                        while (not m_mx[back]) { --back; }
                        unequal(m_x.find_front(), front);
                        unequal(m_x.find_back(),  back);
                        unequal(m_x.exclusive_find_prev(m_n), back);
                }

                auto first = 0UZ;
                while (first < m_n and not m_mx[first]) { ++first; }
                unequal(m_x.find_first(), first);
                unequal(m_x.find_last(),  m_n);

                for (auto i = 0UZ; i < m_n; ++i) {
                        auto next = i + 1;
                        while (next < m_n and not m_mx[next]) { ++next; }
                        unequal(m_x.exclusive_find_next(i), next);
                }

                // The primitive, checked over its whole domain, n == size() included. [design.md#inclusive-is-the-primitive]
                for (auto i = 0UZ; i <= m_n; ++i) {
                        auto bound = i;
                        while (bound < m_n and not m_mx[bound]) { ++bound; }
                        unequal(m_x.inclusive_find_next(i), bound);
                }
                for (auto i = 1UZ; i <= m_n and m_cardinality != 0; ++i) {
                        auto j = i;
                        while (j-- > 0) {
                                if (m_mx[j]) {
                                        unequal(m_x.exclusive_find_prev(i), j);
                                        break;
                                }
                        }
                }
        }

        // Where the block-at-a-time shortcuts live.
        auto relational() -> void
        {
                auto subset = true;
                auto differs = false;
                auto meets = false;
                for (auto i = 0UZ; i < m_n; ++i) {
                        subset  = subset and (not m_mx[i] or m_my[i]);
                        differs = differs or  (m_mx[i] != m_my[i]);
                        meets   = meets   or  (m_mx[i] and m_my[i]);
                }
                disagree(m_x.is_subset_of(m_y),        subset);
                disagree(m_x.is_proper_subset_of(m_y), subset and differs);
                disagree(m_x.intersects(m_y),          meets);
        }

        // On packed bits the set and pointwise sequence operations are one instruction, so one model answers both.
        auto bitwise() -> void
        {
                { auto& a = fresh_x(); a &= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] and     m_my[i]; } same(m, a); }
                { auto& a = fresh_x(); a |= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] or      m_my[i]; } same(m, a); }
                { auto& a = fresh_x(); a ^= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] !=      m_my[i]; } same(m, a); }
                { auto& a = fresh_x(); a -= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] and not m_my[i]; } same(m, a); }
        }

        auto shifts() -> void
        {
                for (auto s = 0UZ; s < m_n; ++s) {
                        { auto& a = fresh_x(); a <<= s; auto m = model(m_n); for (auto i = s;  i < m_n;     ++i) { m[i] = m_mx[i - s]; } same(m, a); }
                        { auto& a = fresh_x(); a >>= s; auto m = model(m_n); for (auto i = 0UZ; i + s < m_n; ++i) { m[i] = m_mx[i + s]; } same(m, a); }
                }
        }

        // One method apiece: combined, GCC 15 at -O3 reports a free-nonheap-object that is not there. [design.md#scratch-objects]
        auto whole_set() -> void
        {
                auto& a = fresh_x();
                a.set();
                disagree(a.all(), true);
                unequal(a.count(), m_n);
        }

        auto whole_reset() -> void
        {
                auto& a = fresh_x();
                a.reset();
                disagree(a.none(), true);
                unequal(a.count(), 0UZ);
        }

        auto whole_flip() -> void
        {
                auto& a = fresh_x();
                a.flip();
                unequal(a.count(), m_n - m_cardinality);
                for (auto const i : std::views::iota(0UZ, m_n)) {
                        disagree(a.test(i), not m_mx[i]);
                }
        }

        auto whole_swap() -> void
        {
                auto& a = fresh_x();
                auto& b = fresh_y();
                a.swap(b);
                disagree(a == m_y, true);
                disagree(b == m_x, true);
        }

        // Per bit, including the two that report whether the bit was already there.
        auto positions() -> void
        {
                for (auto i = 0UZ; i < m_n; ++i) {
                        { auto& a = fresh_x(); a.set(i);   disagree(a.test(i), true);  }
                        { auto& a = fresh_x(); a.reset(i); disagree(a.test(i), false); }
                        { auto& a = fresh_x(); a.flip(i);  disagree(a.test(i), not m_mx[i]); }
                        { auto& a = fresh_x(); disagree(a.insert(i), not m_mx[i]); disagree(a.test(i), true);  }
                        { auto& a = fresh_x(); disagree(a.erase(i),      m_mx[i]); disagree(a.test(i), false); }
                }
        }

        // Writing blocks back is the identity, and all-ones must stop at size(): the unused-tail invariant.
        auto blocks() -> void
        {
                disagree(m_x.num_blocks() * BB::bits_per_block >= m_n, true);

                {
                        auto& a = fresh_x();
                        for (auto i = 0UZ; i < m_x.num_blocks(); ++i) {
                                a.set_block(i, m_x.block(i));
                        }
                        disagree(a == m_x, true);
                }
                {
                        auto& a = fresh_x();
                        for (auto i = 0UZ; i < m_x.num_blocks(); ++i) {
                                a.set_block(i, static_cast<BB::block_type>(-1));
                        }
                        disagree(a.all(), true);
                        unequal(a.count(), m_n);
                }
        }
};

template<class BB>
auto check_ops(BB const& x, BB const& y, int& disagreements) -> void
{
        auto a = x;
        auto b = y;
        auto c = checker<BB>(x, y, a, b, disagreements);
        c.width();
        c.scans();
        c.relational();
        c.bitwise();
        c.shifts();
        c.whole_set();
        c.whole_reset();
        c.whole_flip();
        c.whole_swap();
        c.positions();
        c.blocks();
}

// Seven patterns, every pair landing on both sides of each branch. [design.md#seven-patterns]
template<std::size_t N, class Block>
using graded_block_array = xstd::block_array<Block, N>;

template<class BB>
auto sweep(BB const& empty) -> int
{
        auto const n = empty.size();

        auto values = std::vector<BB>();
        // views::iota, not i < n: at width zero that folds to a comparison against zero. [design.md#width-zero-comparisons]
        auto const push = [&](auto fill) -> void {
                auto b = empty;
                for (auto const i : std::views::iota(0UZ, n)) {
                        if (fill(i)) { b.set(i); }
                }
                values.push_back(b);
        };
        // Captured by reference: a static width folds these to constants. [design.md#width-zero-comparisons]
        push([&](std::size_t  ) -> bool { return false;                });
        push([&](std::size_t  ) -> bool { return true;                 });
        push([&](std::size_t i) -> bool { return i % 2 == 0;           });
        push([&](std::size_t i) -> bool { return i % 3 == 0;           });
        push([&](std::size_t i) -> bool { return i == 0 or i + 1 == n; });
        push([&](std::size_t i) -> bool { return i + 1 == n;           });
        push([&](std::size_t i) -> bool { return (i / BB::bits_per_block) + 1UZ < empty.num_blocks(); });

        auto disagreements = 0;
        for (auto const& x : values) {
                for (auto const& y : values) {
                        check_ops(x, y, disagreements);
                }
        }
        return disagreements;
}

// Named rather than immediately-invoked lambdas, so nothing leans on P1102 for no reason.
constexpr auto a_static_width_is_constexpr() -> bool
{
        auto b = xstd::block_array<std::uint8_t, 9>();
        b.set(8);
        return b.count() == 1 and b.find_first() == 8;
}

constexpr auto a_run_time_width_is_constexpr() -> bool
{
        auto b = xstd::block_vector<std::uint8_t>(9);
        b.set(8);
        return b.count() == 1 and b.find_first() == 8;
}

} // namespace

// Both shipped vehicles satisfy block_storage: growth is detected where it exists, never required.
BOOST_AUTO_TEST_CASE(ItsStorageIsAContiguousSizedRangeOfUnsignedIntegers)
{
        static_assert(xstd::block_storage<std::array<std::uint8_t, 4>>);
        static_assert(xstd::block_storage<std::vector<std::uint64_t>>);

        static_assert(not xstd::block_storage<std::vector<bool>>);      // not a contiguous range
        static_assert(not xstd::block_storage<std::vector<int>>);       // nor unsigned integers
}

// A compile-time width costs nothing: the absent size member takes no storage.
BOOST_AUTO_TEST_CASE(AStaticWidthAddsNothingToItsBlocks)
{
        static_assert(sizeof(xstd::block_array<std::uint64_t,  64>) == sizeof(std::array<std::uint64_t,  1>));
        static_assert(sizeof(xstd::block_array<std::uint8_t,  129>) == sizeof(std::array<std::uint8_t,  17>));
        static_assert(sizeof(xstd::block_array<std::uint8_t,    0>) == sizeof(std::array<std::uint8_t,   1>));

        static_assert(    xstd::block_array<std::size_t, 64>::has_static_size);
        static_assert(not xstd::block_vector<>::has_static_size);
}

// Both widths in a constant expression; the run-time one needs C++20 constexpr allocation.
BOOST_AUTO_TEST_CASE(BothWidthsAreUsableAtCompileTime)
{
        static_assert(a_static_width_is_constexpr());
        static_assert(a_run_time_width_is_constexpr());
}

// The static width at every extent instantiated; the three owners hold this same storage.
BOOST_AUTO_TEST_CASE_TEMPLATE(AStaticWidthAgreesWithTheModel, T, test::graded_extents<graded_block_array>)
{
        BOOST_CHECK_EQUAL(sweep(T()), 0);
}

// The run-time width, at the same grading: within one block, and across boundaries either side.
BOOST_AUTO_TEST_CASE_TEMPLATE(ARunTimeWidthAgreesWithTheModel, Block, test::word_types)
{
        using T = xstd::block_vector<Block>;
        constexpr auto D = test::digits_v<Block>;

        auto disagreements = 0;
        for (auto const n : { 0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D, (3 * D) + 1 }) {
                disagreements += sweep(T(n));
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

// Two run-time widths share a type, so == must answer a pair a static width can never form.
BOOST_AUTO_TEST_CASE(RunTimeWidthsOfDifferentSizeAreNotEqual)
{
        using T = xstd::block_vector<std::uint8_t>;

        BOOST_CHECK(T(8) != T(9));
        BOOST_CHECK(T(8) == T(8));

        auto x = T(9);
        auto y = T(9);
        x.set(0);
        BOOST_CHECK(x != y);
        y.set(0);
        BOOST_CHECK(x == y);
}

// A zero width still owns one block, and every operation must see through it to the width.
BOOST_AUTO_TEST_CASE(AZeroWidthOwnsOneBlockAndReadsEmpty)
{
        auto const b = xstd::block_vector<std::uint8_t>(0);

        BOOST_CHECK_EQUAL(b.size(), 0UZ);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
        BOOST_CHECK_EQUAL(b.count(), 0UZ);
        BOOST_CHECK(b.none());
        BOOST_CHECK(b.all());           // vacuously, as std::bitset<0>::all() is
        BOOST_CHECK(not b.any());
}

// That sole block is entirely padding, which is what the last-block mask exists to say. [design.md#padding]
BOOST_AUTO_TEST_CASE(AZeroWidthsOneBlockIsAllPaddingAndStaysZero)
{
        auto b = xstd::block_vector<std::uint8_t>(0);

        b.set();
        BOOST_CHECK(b.none());
        b.flip();
        BOOST_CHECK(b.none());
        BOOST_CHECK_EQUAL(b.block(0), 0U);
}

// Default-constructed is zero-width, not block-less. [design.md#default-construction]
BOOST_AUTO_TEST_CASE(ADefaultConstructedRunTimeWidthIsZeroWidthWithOneBlock)
{
        auto const b = xstd::block_vector<std::uint8_t>();

        BOOST_CHECK_EQUAL(b.size(), 0UZ);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
        BOOST_CHECK(b == xstd::block_vector<std::uint8_t>(0));
}

// Each entry reaches the member it names, and every call stays inside the kept contracts. [design.md#the-ceiling-principle]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorForwardsToTheStorage, T, test::graded_extents<graded_block_array>)
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        static_assert(xstd::bit_storage<T>);
        static_assert(xstd::static_bit_extent<T>);
        static_assert(xstd::block_readable<traits, T>);

        auto c = T();

        BOOST_CHECK_EQUAL(traits::size(c), N);
        BOOST_CHECK_EQUAL(traits::find_last(c), N);
        BOOST_CHECK_EQUAL(traits::find_first(c), N);
        BOOST_CHECK_EQUAL(traits::count(c), 0UZ);
        BOOST_CHECK_EQUAL(traits::num_blocks(c), c.num_blocks());

        // BOOST_CHECK, not BOOST_CHECK_EQUAL: uint128 has no operator<<. [design.md#uint128-printing]
        for (auto k = 0UZ; k < c.num_blocks(); ++k) {
                BOOST_CHECK(traits::block(c, k) == c.block(k));
        }

        // One position at a time, set then cleared: assign's two arms are the point.
        for (auto i = 0UZ; i < N; ++i) {
                traits::assign(c, i, true);
                BOOST_CHECK(traits::at(c, i));
                BOOST_CHECK_EQUAL(traits::count(c), 1UZ);
                BOOST_CHECK_EQUAL(traits::find_first(c), i);
                BOOST_CHECK_EQUAL(traits::find_prev(c, i + 1UZ), i);
                BOOST_CHECK_EQUAL(traits::find_next(c, i), N);

                traits::assign(c, i, false);
                BOOST_CHECK(not traits::at(c, i));
                BOOST_CHECK_EQUAL(traits::count(c), 0UZ);
        }
}

// The two entries the readings cannot synthesize, in their own case: insert can grow where the storage allows, and fill is bulk. [design.md#what-the-door-reconciles]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorInsertsAndFills, T, test::graded_extents<graded_block_array>)
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        auto c = T();
        traits::fill(c, true);
        BOOST_CHECK_EQUAL(traits::count(c), N);
        traits::fill(c, false);
        BOOST_CHECK_EQUAL(traits::count(c), 0UZ);

        for (auto i = 0UZ; i < N; ++i) {
                traits::insert(c, i);
        }
        BOOST_CHECK_EQUAL(traits::count(c), N);
}

namespace {

// The set reading: the positions held, in increasing order. The sequence reading is reference() itself.
template<class BB>
auto set_reading(BB const& b) -> std::vector<std::size_t>
{
        auto v = std::vector<std::size_t>();
        for (auto i = 0UZ; i < b.size(); ++i) {
                if (b.test(i)) {
                        v.push_back(i);
                }
        }
        return v;
}

// The values worth pairing at a width: both extremes, the ends, and each block boundary either side of it.
template<class BB>
auto probes(BB const& empty) -> std::vector<BB>
{
        auto const n = empty.size();
        auto out = std::vector<BB>{ empty };

        auto full = empty;
        full.set();
        out.push_back(full);

        auto const single = [&](std::size_t i) -> void {
                auto b = empty;
                b.set(i);
                out.push_back(b);
        };

        if (n > 0) {
                single(0UZ);
                single(n - 1UZ);
                for (auto k = 0UZ; k < empty.num_blocks(); ++k) {
                        auto const lo = k * BB::bits_per_block;
                        if (lo < n) {
                                single(lo);
                                single(std::ranges::min(lo + BB::bits_per_block - 1UZ, n - 1UZ));
                        }
                }
                auto ends = empty;
                ends.set(0UZ);
                ends.set(n - 1UZ);
                out.push_back(ends);
        }
        if (n > 1) {
                single(1UZ);
        }
        return out;
}

// The invariant on both readings: the block-wise answer is the standard algorithm's, or it is wrong. [design.md#the-ordering-invariant]
template<class BB>
auto disagreements(BB const& empty) -> int
{
        auto const values = probes(empty);
        auto n = 0;
        for (auto const& x : values) {
                for (auto const& y : values) {
                        auto const sx = set_reading(x);
                        auto const sy = set_reading(y);
                        if (std::lexicographical_compare_three_way(sx.begin(), sx.end(), sy.begin(), sy.end()) != x.set_three_way(y)) {
                                ++n;
                        }
                        // No comparator: vector<bool>'s proxy converts to bool, which is what makes it three_way_comparable.
                        auto const qx = reference(x);
                        auto const qy = reference(y);
                        if (std::lexicographical_compare_three_way(qx.begin(), qx.end(), qy.begin(), qy.end()) != x.sequence_three_way(y)) {
                                ++n;
                        }
                }
        }
        return n;
}

}       // namespace

// Both orderings, at every static extent, against the algorithms that define them. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE_TEMPLATE(BothOrderingsAgreeWithTheirReading, T, test::graded_extents<graded_block_array>)
{
        BOOST_CHECK_EQUAL(disagreements(T()), 0);
}

// The same at a run-time width, which shares no instantiation with the static one. [design.md#per-instantiation-slots]
BOOST_AUTO_TEST_CASE_TEMPLATE(BothOrderingsAgreeAtARunTimeWidth, Block, test::word_types)
{
        using T = xstd::block_vector<Block>;
        constexpr auto D = test::digits_v<Block>;

        auto disagreed = 0;
        for (auto const n : { 0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D }) {
                disagreed += disagreements(T(n));
        }
        BOOST_CHECK_EQUAL(disagreed, 0);
}

// The pair that separates the readings: {0,1} holds the lower position, and holds more. [design.md#two-readings-disagree]
BOOST_AUTO_TEST_CASE(TheTwoOrderingsDisagree)
{
        using T = xstd::block_array<std::uint8_t, 9>;

        using orderings = std::pair<std::strong_ordering, std::strong_ordering>;
        constexpr auto disagreed = [] -> orderings {
                auto x = T();
                x.set(0);
                x.set(1);
                auto y = T();
                y.set(1);
                return std::pair{ x.set_three_way(y), x.sequence_three_way(y) };
        }();

        static_assert(disagreed.first  == std::strong_ordering::less);
        static_assert(disagreed.second == std::strong_ordering::greater);
}

// The prefix clause, which is the whole of what the set reading adds: {1} beats {} only by being longer.
BOOST_AUTO_TEST_CASE(TheSetOrderingPutsAPrefixFirst)
{
        using T = xstd::block_array<std::uint8_t, 9>;

        auto x = T();
        x.set(1);
        auto const y = T();

        // {} is a prefix of {1}, so it sorts below -- the opposite of what holding the lower position would say.
        BOOST_CHECK(x.set_three_way(y) == std::strong_ordering::greater);

        // And with something above that position, the clause no longer applies.
        auto z = T();
        z.set(8);
        BOOST_CHECK(x.set_three_way(z) == std::strong_ordering::less);
}

// Two named entries, so a caller says which reading it means rather than being handed one. [design.md#two-readings-disagree]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorNamesBothOrderings, T, test::graded_extents<graded_block_array>)
{
        using traits = xstd::bit_traits<T>;

        // Over the same probes, so a width with nothing to differ at is covered by the same code as any other.
        auto const values = probes(T());
        for (auto const& x : values) {
                for (auto const& y : values) {
                        BOOST_CHECK(traits::set_three_way(x, y)      == x.set_three_way(y));
                        BOOST_CHECK(traits::sequence_three_way(x, y) == x.sequence_three_way(y));
                }
        }
}

BOOST_AUTO_TEST_SUITE_END()
