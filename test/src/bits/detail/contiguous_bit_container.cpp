//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                               // digits_v, graded_extents, word_types
#include <test/inplace_vector.hpp>                            // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/uint128.hpp>                                   // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/detail/contiguous_bit_array.hpp>          // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_container.hpp>      // contiguous_bit_container
#include <xstd/bits/detail/contiguous_block_range.hpp>        // contiguous_block_range
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // IWYU pragma: keep; contiguous_bit_inplace_vector, named only under TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/detail/contiguous_bit_vector.hpp>         // contiguous_bit_vector
#include <xstd/bits/detail/range_const_reference.hpp>         // fallback::range_const_reference_t, range_const_reference_t
#include <xstd/ints/memory.hpp>                               // IWYU pragma: keep; align_up, named only under TEST_HAS_INPLACE_VECTOR
#include <boost/test/unit_test.hpp>                           // BOOST_CHECK_EQUAL, BOOST_CHECK_LE, BOOST_CHECK_LT, BOOST_CHECK_THROW, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <algorithm>                                          // count, lexicographical_compare_three_way, min
#include <array>                                              // array
#include <bitset>                                             // bitset
#include <compare>                                            // strong_ordering
#include <concepts>                                           // same_as
#include <cstddef>                                            // ptrdiff_t, size_t
#include <cstdint>                                            // uint8_t, uint64_t
#include <initializer_list>                                   // initializer_list
#include <limits>                                             // numeric_limits
#include <memory>                                             // addressof, allocator
#include <version>                                            // IWYU pragma: keep; __cpp_lib_ranges_as_const
#include <new>                                                // IWYU pragma: keep; bad_alloc, named only under TEST_HAS_INPLACE_VECTOR
#include <ranges>                                             // begin, iota, range_const_reference_t, size
#include <span>                                               // dynamic_extent
#include <stdexcept>                                          // length_error
#include <tuple>                                              // get, tuple
#include <vector>                                             // vector

BOOST_AUTO_TEST_SUITE(BitBlocks)

namespace {

// std::vector<bool> is the reference reading: one bool per bit, no packing, no invariant to get wrong.
using model = std::vector<bool>;

template<class BB>
auto reference(BB const& b)
        -> model
{
        auto m = model(b.size());
        for (auto i = 0UZ; i < b.size(); ++i) {
                m[i] = b.test(i);
        }
        return m;
}

// One family of checks per member, disagreements counted rather than asserted.
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

        // Two scratch objects by reference: copies per check trip three GCC diagnostics.
        BB& m_a;
        BB& m_b;

        auto fresh_x()
                -> BB&
        {
                m_a = m_x;
                return m_a;
        }

        auto fresh_y()
                -> BB&
        {
                m_b = m_y;
                return m_b;
        }

        // The two sites where a comparison becomes a count, so the cast is not thirty.
        auto disagree(bool ours, bool theirs)
                -> void
        {
                m_disagreements += static_cast<int>(ours != theirs);
        }

        auto unequal(std::size_t ours, std::size_t theirs)
                -> void
        {
                m_disagreements += static_cast<int>(ours != theirs);
        }

        // Position by position against a model of what the operation should have left.
        auto same(model const& m, BB const& got)
                -> void
        {
                for (auto i = 0UZ; i < m_n; ++i) {
                        disagree(got.test(i), m[i]);
                }
        }

public:
        checker(BB const& x, BB const& y, BB& a, BB& b, int& disagreements)
                : m_x(x)
                , m_y(y)
                , m_disagreements(disagreements)
                , m_a(a)
                , m_b(b)
        {}

        auto width()
                -> void
        {
                unequal(m_x.count(), m_cardinality);
                disagree(m_x.any(), m_cardinality != 0);
                disagree(m_x.none(), m_cardinality == 0);
                disagree(m_x.all(), m_cardinality == m_n);
                disagree(m_x == m_y, m_mx == m_my);
        }

        // find_front/find_back assert any(); find_first/find_last are total and answer size().
        auto scans()
                -> void
        {
                if (m_cardinality != 0) {
                        auto front = 0UZ;
                        while (not m_mx[front]) {
                                ++front;
                        }
                        auto back = m_n - 1;
                        while (not m_mx[back]) {
                                --back;
                        }
                        unequal(m_x.find_front(), front);
                        unequal(m_x.find_back(), back);
                        unequal(m_x.exclusive_find_prev(m_n), back);
                }

                auto first = 0UZ;
                while (first < m_n and not m_mx[first]) {
                        ++first;
                }
                unequal(m_x.find_first(), first);
                unequal(m_x.find_last(), m_n);

                for (auto i = 0UZ; i < m_n; ++i) {
                        auto next = i + 1;
                        while (next < m_n and not m_mx[next]) {
                                ++next;
                        }
                        unequal(m_x.exclusive_find_next(i), next);
                }

                // The primitive, checked over its whole domain, n == size() included.
                for (auto i = 0UZ; i <= m_n; ++i) {
                        auto bound = i;
                        while (bound < m_n and not m_mx[bound]) {
                                ++bound;
                        }
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
        auto relational()
                -> void
        {
                auto subset = true;
                auto differs = false;
                auto meets = false;
                for (auto i = 0UZ; i < m_n; ++i) {
                        subset = subset and (not m_mx[i] or m_my[i]);
                        differs = differs or (m_mx[i] != m_my[i]);
                        meets = meets or (m_mx[i] and m_my[i]);
                }
                disagree(m_x.is_subset_of(m_y), subset);
                disagree(m_x.is_proper_subset_of(m_y), subset and differs);
                disagree(m_x.intersects(m_y), meets);

                // The hidden friend answers the member, and both operand orders alike: a meets b when b meets a.
                disagree(intersects(m_x, m_y), meets);
                disagree(intersects(m_y, m_x), meets);
        }

        // On packed bits the set and pointwise sequence operations are one instruction, so one model answers both.
        auto bitwise()
                -> void
        {
                {
                        auto& a = fresh_x();
                        a &= m_y;
                        auto m = model(m_n);
                        for (auto i = 0UZ; i < m_n; ++i) {
                                m[i] = m_mx[i] and m_my[i];
                        }
                        same(m, a);
                }
                {
                        auto& a = fresh_x();
                        a |= m_y;
                        auto m = model(m_n);
                        for (auto i = 0UZ; i < m_n; ++i) {
                                m[i] = m_mx[i] or m_my[i];
                        }
                        same(m, a);
                }
                {
                        auto& a = fresh_x();
                        a ^= m_y;
                        auto m = model(m_n);
                        for (auto i = 0UZ; i < m_n; ++i) {
                                m[i] = m_mx[i] != m_my[i];
                        }
                        same(m, a);
                }
                {
                        auto& a = fresh_x();
                        a -= m_y;
                        auto m = model(m_n);
                        for (auto i = 0UZ; i < m_n; ++i) {
                                m[i] = m_mx[i] and not m_my[i];
                        }
                        same(m, a);
                }
        }

        auto shifts()
                -> void
        {
                for (auto s = 0UZ; s < m_n; ++s) {
                        {
                                auto& a = fresh_x();
                                a <<= s;
                                auto m = model(m_n);
                                for (auto i = s; i < m_n; ++i) {
                                        m[i] = m_mx[i - s];
                                }
                                same(m, a);
                        }
                        {
                                auto& a = fresh_x();
                                a >>= s;
                                auto m = model(m_n);
                                for (auto i = 0UZ; i + s < m_n; ++i) {
                                        m[i] = m_mx[i + s];
                                }
                                same(m, a);
                        }
                }
        }

        // One method apiece: combined, GCC 15 at -O3 reports a free-nonheap-object that is not there.
        auto whole_set()
                -> void
        {
                auto& a = fresh_x();
                a.set();
                disagree(a.all(), true);
                unequal(a.count(), m_n);
        }

        auto whole_reset()
                -> void
        {
                auto& a = fresh_x();
                a.reset();
                disagree(a.none(), true);
                unequal(a.count(), 0UZ);
        }

        auto whole_flip()
                -> void
        {
                auto& a = fresh_x();
                a.flip();
                unequal(a.count(), m_n - m_cardinality);
                for (auto const i : std::views::iota(0UZ, m_n)) {
                        disagree(a.test(i), not m_mx[i]);
                }
        }

        auto whole_swap()
                -> void
        {
                auto& a = fresh_x();
                auto& b = fresh_y();
                swap(a, b);
                disagree(a == m_y, true);
                disagree(b == m_x, true);
        }

        // Per bit, including the two that report whether the bit was already there.
        auto positions()
                -> void
        {
                for (auto i = 0UZ; i < m_n; ++i) {
                        {
                                auto& a = fresh_x();
                                a.set(i);
                                disagree(a.test(i), true);
                        }
                        {
                                auto& a = fresh_x();
                                a.reset(i);
                                disagree(a.test(i), false);
                        }
                        {
                                auto& a = fresh_x();
                                a.flip(i);
                                disagree(a.test(i), not m_mx[i]);
                        }
                        {
                                auto& a = fresh_x();
                                disagree(a.insert(i), not m_mx[i]);
                                disagree(a.test(i), true);
                        }
                        {
                                auto& a = fresh_x();
                                disagree(a.erase(i), m_mx[i]);
                                disagree(a.test(i), false);
                        }
                }
        }

        // Writing blocks back is the identity, and all-ones must stop at size(): the unused-tail invariant.
        auto blocks()
                -> void
        {
                disagree(m_x.num_blocks() * BB::bits_per_block >= m_n, true);

                {
                        auto& a = fresh_x();
                        for (auto i = 0UZ; i < m_x.num_blocks(); ++i) {
                                a.block(i) = m_x.block(i);
                        }
                        a.erase_unused();
                        disagree(a == m_x, true);
                }
                {
                        auto& a = fresh_x();
                        for (auto i = 0UZ; i < m_x.num_blocks(); ++i) {
                                a.block(i) = static_cast<BB::block_type>(-1);
                        }
                        // The writer restores the invariant, which the reference hands it rather than doing itself.
                        a.erase_unused();
                        disagree(a.all(), true);
                        unequal(a.count(), m_n);
                }
        }
};

template<class BB>
auto check_ops(BB const& x, BB const& y, int& disagreements)
        -> void
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

// Seven patterns, every pair landing on both sides of each branch.

template<class BB>
auto sweep(BB const& empty)
        -> int
{
        auto const n = empty.size();

        auto values = std::vector<BB>();
        // views::iota, not i < n: at width zero that folds to a comparison against zero.
        auto const push = [&](auto fill) -> void {
                auto b = empty;
                for (auto const i : std::views::iota(0UZ, n)) {
                        if (fill(i)) {
                                b.set(i);
                        }
                }
                values.push_back(b);
        };
        // Captured by reference: a static width folds these to constants.
        push([&](std::size_t) -> bool { return false; });
        push([&](std::size_t) -> bool { return true; });
        push([&](std::size_t i) -> bool { return i % 2 == 0; });
        push([&](std::size_t i) -> bool { return i % 3 == 0; });
        push([&](std::size_t i) -> bool { return i == 0 or i + 1 == n; });
        push([&](std::size_t i) -> bool { return i + 1 == n; });
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
constexpr auto a_static_width_is_constexpr()
        -> bool
{
        auto b = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>();
        b.set(8);
        return b.count() == 1 and b.find_first() == 8;
}

constexpr auto a_run_time_width_is_constexpr()
        -> bool
{
        auto b = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>(9);
        b.set(8);
        return b.count() == 1 and b.find_first() == 8;
}

} // namespace

// Both shipped vehicles satisfy contiguous_block_range: growth is detected where it exists, never required.
BOOST_AUTO_TEST_CASE(ItsStorageIsAContiguousSizedRangeOfUnsignedIntegers)
{
        static_assert(xstd::detail::bits::contiguous_block_range<std::array<std::uint8_t, 4>>);
        static_assert(xstd::detail::bits::contiguous_block_range<std::vector<std::uint64_t>>);

        static_assert(not xstd::detail::bits::contiguous_block_range<std::vector<bool>>); // not a contiguous range
        static_assert(not xstd::detail::bits::contiguous_block_range<std::vector<int>>);  // nor unsigned integers

        // The element clause is unsigned_integer, not bitwise_operators: the <bit> intrinsics want the narrower.
        static_assert(not xstd::detail::bits::contiguous_block_range<std::array<std::bitset<64>, 4>>);
}

// The const subscript against P2278R4's range_const_reference_t: wherever both arms exist they must agree.
BOOST_AUTO_TEST_CASE(TheConstReferenceIsP2278s)
{
#ifdef __cpp_lib_ranges_as_const

        static_assert(std::same_as<xstd::detail::bits::fallback::range_const_reference_t<std::array<std::uint8_t, 4>>, std::ranges::range_const_reference_t<std::array<std::uint8_t, 4>>>);
        static_assert(std::same_as<xstd::detail::bits::fallback::range_const_reference_t<std::vector<std::uint64_t>>, std::ranges::range_const_reference_t<std::vector<std::uint64_t>>>);
        static_assert(std::same_as<xstd::detail::bits::fallback::range_const_reference_t<std::vector<bool>>, std::ranges::range_const_reference_t<std::vector<bool>>>);

#endif

        // What the clause buys: no blocks are writable through a const contiguous_bit_container.
        static_assert(std::same_as<xstd::detail::bits::range_const_reference_t<std::array<std::uint8_t, 4>>, std::uint8_t const&>);
        static_assert(std::same_as<xstd::detail::bits::range_const_reference_t<std::vector<std::uint64_t>>, std::uint64_t const&>);

        // Transcribed, not approximated: a conditional_t over is_const says bool const& where the paper says bool.
        static_assert(std::same_as<xstd::detail::bits::fallback::range_const_reference_t<std::vector<bool>>, bool>);
}

// The three members the readings call: insert(n) is partial where growing_insert(n) is total.
BOOST_AUTO_TEST_CASE(TheTotalInsertGrowsWhereThePartialOneAsserts)
{
        using A = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 10>;
        using V = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;

        // The width as a type, dynamic_extent where there is none.
        static_assert(A::extent == 10UZ);
        static_assert(V::extent == std::dynamic_extent);

        auto a = A();
        BOOST_CHECK(not a.test(3));
        a.assign(3, true);
        BOOST_CHECK(a.test(3));
        a.assign(3, false);
        BOOST_CHECK(not a.test(3));
        a.fill(true);
        BOOST_CHECK(a.all());
        a.fill(false);
        BOOST_CHECK(a.none());

        // In range, a static width has nowhere to grow and the total form answers as the partial one does.
        BOOST_CHECK(a.growing_insert(4));
        BOOST_CHECK(not a.growing_insert(4));
        BOOST_CHECK_EQUAL(a.size(), 10UZ);

        // Past the end, a run-time width grows to admit the position, and the bit is new by construction.
        auto v = V();
        BOOST_CHECK_EQUAL(v.size(), 0UZ);
        BOOST_CHECK(v.growing_insert(7));
        BOOST_CHECK_EQUAL(v.size(), 8UZ);
        BOOST_CHECK(v.test(7));
        BOOST_CHECK_EQUAL(v.count(), 1UZ);

        // Already there, so no growth and no newness.
        BOOST_CHECK(not v.growing_insert(7));
        BOOST_CHECK_EQUAL(v.size(), 8UZ);

        // And growing leaves the bits below it alone.
        BOOST_CHECK(v.growing_insert(20));
        BOOST_CHECK_EQUAL(v.size(), 21UZ);
        BOOST_CHECK(v.test(7) and v.test(20));
        BOOST_CHECK_EQUAL(v.count(), 2UZ);
}

// The semantic half a concept cannot check: a[i] is *(begin(a) + i), the same object and not merely an equal one.
template<xstd::detail::bits::contiguous_block_range Blocks>
constexpr auto subscript_agrees_with_iteration(Blocks blocks) noexcept
        -> bool
{
        // The index is the range's difference_type; subscript takes size_type, which is the one cast.
        for (auto i = std::ranges::range_difference_t<Blocks>{}; i < std::ranges::ssize(blocks); ++i) {
                if (std::addressof(blocks[static_cast<Blocks::size_type>(i)]) != std::addressof(*(std::ranges::begin(blocks) + i))) {
                        return false;
                }
        }
        return true;
}

BOOST_AUTO_TEST_CASE(ItsStorageSubscriptIsIterationAtTheSameAddress)
{
        static_assert(subscript_agrees_with_iteration(std::array<std::uint8_t, 4>{1, 2, 3, 4}));
        static_assert(subscript_agrees_with_iteration(std::vector<std::uint64_t>{1, 2, 3, 4}));
        BOOST_CHECK(subscript_agrees_with_iteration(std::vector<std::uint64_t>{1, 2, 3, 4}));
}

// ranges::swap finds a free swap by ADL and a member never, so this storage needs the free one too.
namespace {

int g_storage_swaps = 0;
int g_storage_moves = 0;

// A storage satisfying contiguous_block_range whose swap and moves are distinguishable.
struct counting_blocks
{
        using size_type = std::size_t;

        std::array<std::uint64_t, 4> m_data{};

        // The move operations are counted rather than used: the free swap is what should be called.
        counting_blocks() = default;
        [[maybe_unused]] counting_blocks(counting_blocks const&) = default;
        [[maybe_unused]] auto operator=(counting_blocks const&) -> counting_blocks& = default;

        [[maybe_unused]] counting_blocks(counting_blocks&& other) noexcept
                : m_data(other.m_data)
        {
                ++g_storage_moves;
        }

        [[maybe_unused]] auto operator=(counting_blocks&& other) noexcept
                -> counting_blocks&
        {
                m_data = other.m_data;
                ++g_storage_moves;
                return *this;
        }

        [[maybe_unused]] ~counting_blocks() = default;

        [[nodiscard, maybe_unused]] auto begin()
                -> std::uint64_t*
        {
                return m_data.data();
        }

        [[nodiscard, maybe_unused]] auto begin() const
                -> std::uint64_t const*
        {
                return m_data.data();
        }

        [[nodiscard, maybe_unused]] auto end()
                -> std::uint64_t*
        {
                return m_data.data() + m_data.size();
        }

        [[nodiscard, maybe_unused]] auto end() const
                -> std::uint64_t const*
        {
                return m_data.data() + m_data.size();
        }

        [[nodiscard, maybe_unused]] auto size() const
                -> std::size_t
        {
                return m_data.size();
        }

        [[nodiscard, maybe_unused]] auto operator[](size_type n)
                -> std::uint64_t&
        {
                return m_data[n];
        }

        [[nodiscard, maybe_unused]] auto operator[](size_type n) const
                -> std::uint64_t const&
        {
                return m_data[n];
        }

        [[maybe_unused]] auto operator==(counting_blocks const&) const -> bool = default;

        friend auto swap(counting_blocks& x, counting_blocks& y) noexcept
                -> void
        {
                ++g_storage_swaps;
                x.m_data.swap(y.m_data);
        }
};

} // namespace

BOOST_AUTO_TEST_CASE(ItsSwapIsReachedThroughAdlAndNotTheMoveFallback)
{
        using bits = xstd::detail::bits::contiguous_bit_container<counting_blocks, 256>;

        auto a = bits();
        auto b = bits();

        g_storage_swaps = 0;
        g_storage_moves = 0;
        a.swap(b);
        BOOST_CHECK_EQUAL(g_storage_swaps, 1); // the member, which does the exchange
        BOOST_CHECK_EQUAL(g_storage_moves, 0);

        g_storage_swaps = 0;
        g_storage_moves = 0;
        swap(a, b);
        BOOST_CHECK_EQUAL(g_storage_swaps, 1); // the hidden friend, which forwards to it
        BOOST_CHECK_EQUAL(g_storage_moves, 0);

        g_storage_swaps = 0;
        g_storage_moves = 0;
        std::ranges::swap(a, b);               // and what every adaptor actually calls, reaching the friend by ADL
        BOOST_CHECK_EQUAL(g_storage_swaps, 1); // 0 swaps and 3 moves before the free swap existed
        BOOST_CHECK_EQUAL(g_storage_moves, 0);
}

// A compile-time width costs nothing: the absent size member takes no storage.
BOOST_AUTO_TEST_CASE(AStaticWidthAddsNothingToItsBlocks)
{
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint64_t, 64>) == sizeof(std::array<std::uint64_t, 1>));
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint8_t, 129>) == sizeof(std::array<std::uint8_t, 17>));
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint8_t, 0>) == sizeof(std::array<std::uint8_t, 1>));

        static_assert(xstd::detail::bits::contiguous_bit_array<std::size_t, 64>::has_static_size);
        static_assert(not xstd::detail::bits::contiguous_bit_vector<std::size_t>::has_static_size);
}

// Both widths in a constant expression; the run-time one needs C++20 constexpr allocation.
BOOST_AUTO_TEST_CASE(BothWidthsAreUsableAtCompileTime)
{
        static_assert(a_static_width_is_constexpr());
        static_assert(a_run_time_width_is_constexpr());
}

// The static width at every extent instantiated; the three owners hold this same storage.
BOOST_AUTO_TEST_CASE_TEMPLATE(AStaticWidthAgreesWithTheModel, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        BOOST_CHECK_EQUAL(sweep(T()), 0);
}

// The run-time width, at the same grading: within one block, and across boundaries either side.
BOOST_AUTO_TEST_CASE_TEMPLATE(ARunTimeWidthAgreesWithTheModel, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;
        constexpr auto D = test::digits_v<Block>;

        auto disagreements = 0;
        for (auto const n : {0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D, (3 * D) + 1}) {
                disagreements += sweep(T(n));
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

// Two run-time widths share a type, so == must answer a pair a static width can never form.
BOOST_AUTO_TEST_CASE(RunTimeWidthsOfDifferentSizeAreNotEqual)
{
        using T = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;

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
        auto const b = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>(0);

        BOOST_CHECK_EQUAL(b.size(), 0UZ);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
        BOOST_CHECK_EQUAL(b.count(), 0UZ);
        BOOST_CHECK(b.none());
        BOOST_CHECK(b.all()); // vacuously, as std::bitset<0>::all() is
        BOOST_CHECK(not b.any());
}

// That sole block is entirely padding, which is what the last-block mask exists to say.
BOOST_AUTO_TEST_CASE(AZeroWidthsOneBlockIsAllPaddingAndStaysZero)
{
        auto b = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>(0);

        b.set();
        BOOST_CHECK(b.none());
        b.flip();
        BOOST_CHECK(b.none());
        BOOST_CHECK_EQUAL(b.block(0), 0U);
}

// Default-constructed is zero-width, not block-less.
BOOST_AUTO_TEST_CASE(ADefaultConstructedRunTimeWidthIsZeroWidthWithOneBlock)
{
        auto const b = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>();

        BOOST_CHECK_EQUAL(b.size(), 0UZ);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
        BOOST_CHECK(b == xstd::detail::bits::contiguous_bit_vector<std::uint8_t>(0));
}

namespace {

// A run-time width built from the model, so equality doubles as the invariant check: a dirty tail differs.
template<class T>
[[nodiscard]] auto from_model(model const& m)
        -> T
{
        auto b = T(m.size());
        for (auto i = 0UZ; i < m.size(); ++i) {
                if (m[i]) {
                        b.set(i);
                }
        }
        return b;
}

// Two in three set, so both fill values and every block boundary change something.
[[nodiscard]] auto patterned(std::size_t n)
        -> model
{
        auto m = model(n);
        for (auto i = 0UZ; i < n; ++i) {
                m[i] = i % 3 != 1;
        }
        return m;
}

// The same grading the static sweep uses: within one block, and across boundaries either side.
template<class Block>
[[nodiscard]] constexpr auto graded_widths()
        -> std::array<std::size_t, 10>
{
        constexpr auto D = test::digits_v<Block>;
        return {0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D, (3 * D) + 1};
}

template<class Block>
[[nodiscard]] constexpr auto blocks_for(std::size_t n)
        -> std::size_t
{
        constexpr auto D = test::digits_v<Block>;
        return std::ranges::max((n + D - 1) / D, 1UZ);
}

// What append(block) should do to the model: the block's bits, least significant first.
template<class Block>
auto append_to(model& m, Block value)
        -> void
{
        for (auto i = 0UZ; i < test::digits_v<Block>; ++i) {
                // Cast back before the mask: a shifted narrow word is an int to bugprone-signed-bitwise.
                m.push_back((static_cast<Block>(value >> i) & Block{1}) != Block{0});
        }
}

// Alternating pairs of bits, so a split at any offset lands ones on both sides.
template<class X>
constexpr bool can_resize = requires (X& x) { x.resize(1UZ); x.resize(1UZ, true); };
template<class X>
constexpr bool can_push_pop = requires (X& x) { x.push_back(true); x.pop_back(); };
template<class X>
constexpr bool can_append = requires (X& x) { x.append(x.block(0UZ)); };
template<class X>
constexpr bool can_clear = requires (X& x) { x.clear(); };
template<class X>
constexpr bool can_reserve = requires (X& x) { x.reserve(1UZ); x.shrink_to_fit(); };
template<class X>
constexpr bool has_capacity = requires (X const& x) { x.capacity(); };

template<class Block>
[[nodiscard]] constexpr auto striped()
        -> Block
{
        auto value = Block{0};
        for (auto i = 0UZ; i < test::digits_v<Block>; i += 4) {
                value = static_cast<Block>(value | static_cast<Block>(Block{3} << i));
        }
        return value;
}

} // namespace

// Every resize path: each graded width to each other, both fill values, against the model and a fresh build.
BOOST_AUTO_TEST_CASE_TEMPLATE(ResizingKeepsTheModelAndTheUnusedTailClear, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;

        auto disagreements = 0;
        for (auto const from : graded_widths<Block>()) {
                for (auto const to : graded_widths<Block>()) {
                        for (auto const value : {false, true}) {
                                auto m = patterned(from);
                                auto b = from_model<T>(m);
                                b.resize(to, value);
                                m.resize(to, value);
                                disagreements += static_cast<int>(reference(b) != m);
                                disagreements += static_cast<int>(b != from_model<T>(m));
                                disagreements += static_cast<int>(b.num_blocks() != blocks_for<Block>(to));
                        }
                }
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

// push_back and pop_back are resize by one, checked at every width on the way up and back down.
BOOST_AUTO_TEST_CASE_TEMPLATE(PushingAndPoppingAreResizeByOne, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;
        constexpr auto D = test::digits_v<Block>;

        auto disagreements = 0;
        auto b = T();
        auto m = model();
        for (auto i = 0UZ; i < (3 * D) + 1; ++i) {
                auto const value = i % 3 != 1;
                b.push_back(value);
                m.push_back(value);
                disagreements += static_cast<int>(b.size() != m.size());
                disagreements += static_cast<int>(b != from_model<T>(m));
        }
        while (not m.empty()) {
                b.pop_back();
                m.pop_back();
                disagreements += static_cast<int>(b.size() != m.size());
                disagreements += static_cast<int>(b != from_model<T>(m));
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
}

// Boost's append: a whole block at once, split across two where unaligned; at width zero the floor takes it.
BOOST_AUTO_TEST_CASE_TEMPLATE(AppendingABlockSplitsItAtAnUnalignedWidth, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;

        auto disagreements = 0;
        for (auto const n : graded_widths<Block>()) {
                auto m = patterned(n);
                auto b = from_model<T>(m);

                b.append(striped<Block>());
                append_to(m, striped<Block>());
                disagreements += static_cast<int>(b != from_model<T>(m));

                // And a range of blocks, reserved for first, so the width grows by one block per element.
                auto const blocks = std::array{striped<Block>(), static_cast<Block>(~striped<Block>()), Block{1}};
                b.append(blocks.begin(), blocks.end());
                for (auto const value : blocks) {
                        append_to(m, value);
                }
                disagreements += static_cast<int>(b != from_model<T>(m));
                disagreements += static_cast<int>(b.size() != n + (4 * test::digits_v<Block>));
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

// The width-zero range append, where the bulk path replaces the floor block rather than pushing past it.
BOOST_AUTO_TEST_CASE_TEMPLATE(AppendingARangeFromEmptyAgreesWithTheModel, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;
        auto const blocks = std::array{striped<Block>(), static_cast<Block>(~striped<Block>()), Block{1}};

        auto disagreements = 0;

        {
                auto m = patterned(0UZ);
                auto b = from_model<T>(m);
                b.append(blocks.begin(), blocks.end());
                for (auto const value : blocks) {
                        append_to(m, value);
                }
                disagreements += static_cast<int>(b != from_model<T>(m));
                disagreements += static_cast<int>(b.size() != 3 * test::digits_v<Block>);
        }

        // Nothing appended is nothing changed, at every width including zero.
        for (auto const n : graded_widths<Block>()) {
                auto const m = patterned(n);
                auto b = from_model<T>(m);
                b.append(blocks.begin(), blocks.begin());
                disagreements += static_cast<int>(b != from_model<T>(m));
                disagreements += static_cast<int>(b.size() != n);
        }

        BOOST_CHECK_EQUAL(disagreements, 0);
}

// Capacity is in bits and follows the blocks; reserving and shrinking change it and nothing else.
BOOST_AUTO_TEST_CASE(ReservingAndShrinkingChangeCapacityNotTheBits)
{
        using T = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;

        auto const m = patterned(17);
        auto b = from_model<T>(m);

        b.reserve(40);
        BOOST_CHECK_GE(b.capacity(), 40UZ);
        BOOST_CHECK_EQUAL(b.size(), 17UZ);
        BOOST_CHECK(b == from_model<T>(m));

        b.shrink_to_fit();
        BOOST_CHECK_GE(b.capacity(), b.size());
        BOOST_CHECK(b == from_model<T>(m));
}

// Width zero, one block, all padding: the object a default constructor makes.
BOOST_AUTO_TEST_CASE(ClearingIsResizeToZero)
{
        using T = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;

        auto b = from_model<T>(patterned(17));
        b.clear();
        BOOST_CHECK_EQUAL(b.size(), 0UZ);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
        BOOST_CHECK_EQUAL(b.block(0), 0U);
        BOOST_CHECK(b == T());
}

// A static width has none of it: the members are constrained away rather than asserting.
BOOST_AUTO_TEST_CASE(AStaticWidthDoesNotGrow)
{
        using S = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 8>;
        using D = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;

        static_assert(not can_resize<S> and not can_push_pop<S> and not can_append<S> and not can_clear<S>);
        static_assert(not can_reserve<S> and not has_capacity<S>);

        static_assert(can_resize<D> and can_push_pop<D> and can_append<D> and can_clear<D>);
        static_assert(can_reserve<D> and has_capacity<D>);
}

#ifdef TEST_HAS_INPLACE_VECTOR

// No hole in front of the blocks at any alignment, which is what -Wpadded asks of the class.
BOOST_AUTO_TEST_CASE(TheWidthFillsWhatWouldOtherwisePadTheBlocks)
{
        // Rounded up to the class's own alignment: a sizeof is always a multiple of an alignof.
        constexpr auto tiles = [](std::size_t whole, std::size_t blocks, std::size_t block_align) {
                auto const slot = std::ranges::max(sizeof(std::size_t), block_align);
                return whole == xstd::align_up(blocks + slot, slot);
        };

        static_assert(tiles(sizeof(xstd::detail::bits::contiguous_bit_inplace_vector<std::uint8_t, 24>), sizeof(std::inplace_vector<std::uint8_t, 3>), alignof(std::inplace_vector<std::uint8_t, 3>)));
        static_assert(tiles(sizeof(xstd::detail::bits::contiguous_bit_vector<std::uint8_t>), sizeof(std::vector<std::uint8_t>), alignof(std::vector<std::uint8_t>)));

        // A static width carries no width member at all, so the class is its blocks exactly.
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint8_t, 24>) == sizeof(std::array<std::uint8_t, 3>));

#ifdef TEST_HAS_UINT128

        // The one cell that reaches an over-aligned storage: the width is a block there, and pays nothing for it.
        static_assert(tiles(sizeof(xstd::detail::bits::contiguous_bit_inplace_vector<xstd::uint128, 384>), sizeof(std::inplace_vector<xstd::uint128, 3>), alignof(std::inplace_vector<xstd::uint128, 3>)));
        static_assert(alignof(std::inplace_vector<xstd::uint128, 3>) > alignof(std::size_t));

        // The heap column never reaches it: a vector is a pointer's alignment whatever it holds.
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_vector<xstd::uint128>) == sizeof(xstd::detail::bits::contiguous_bit_vector<std::uint64_t>));

#endif
}

// The third storage: a run-time width under a compile-time capacity, growth past it a bad_alloc.
BOOST_AUTO_TEST_CASE(AnInplaceVectorIsARunTimeWidthUnderAStaticCapacity)
{
        using T = xstd::detail::bits::contiguous_bit_inplace_vector<std::uint8_t, 24>;
        static_assert(not T::has_static_size);
        static_assert(xstd::detail::bits::contiguous_block_range<std::inplace_vector<std::uint8_t, 3>>);
        static_assert(std::same_as<xstd::detail::bits::range_const_reference_t<std::inplace_vector<std::uint8_t, 3>>, std::uint8_t const&>);

        BOOST_CHECK_EQUAL(sweep(T(17)), 0);

        auto b = T();
        BOOST_CHECK_EQUAL(b.capacity(), 24UZ);
        b.resize(24, true);
        BOOST_CHECK(b.all());
        BOOST_CHECK_EQUAL(b.num_blocks(), 3UZ);
        BOOST_CHECK_THROW(b.push_back(true), std::bad_alloc);
        BOOST_CHECK_THROW(b.resize(25), std::bad_alloc);
        BOOST_CHECK_THROW(b.reserve(25), std::bad_alloc);
        b.shrink_to_fit();
        BOOST_CHECK_EQUAL(b.size(), 24UZ);

        // The one storage whose blocks refuse a width without asking for memory, so the refusal can be watched.
        auto c = T(9UZ);
        c.set(2UZ);
        BOOST_CHECK_THROW(c.resize(25UZ), std::bad_alloc);

        // And a refused growth is not a partial one, which the growth with ones is the case for.
        BOOST_CHECK_THROW(c.resize(25UZ, true), std::bad_alloc);
        BOOST_CHECK_EQUAL(c.size(), 9UZ);
        c.resize(20UZ);
        BOOST_CHECK_EQUAL(c.size(), 20UZ);
        BOOST_CHECK_EQUAL(c.count(), 1UZ);
        BOOST_CHECK(c.test(2UZ));
}

#endif

// Every question the three readings ask, asked of the storage in its own name and within the contracts it keeps.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheStorageAnswersEveryReadingsQuestion, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        constexpr auto N = T::extent;

        auto c = T();

        BOOST_CHECK_EQUAL(c.size(), N);
        BOOST_CHECK_EQUAL(c.find_last(), N);
        BOOST_CHECK_EQUAL(c.find_first(), N);
        BOOST_CHECK_EQUAL(c.count(), 0UZ);

        // One position at a time, set then cleared: assign's two arms are the point.
        for (auto i = 0UZ; i < N; ++i) {
                c.assign(i, true);
                BOOST_CHECK(c.test(i));
                BOOST_CHECK_EQUAL(c.count(), 1UZ);
                BOOST_CHECK_EQUAL(c.find_first(), i);
                BOOST_CHECK_EQUAL(c.exclusive_find_prev(i + 1UZ), i);
                BOOST_CHECK_EQUAL(c.exclusive_find_next(i), N);

                c.assign(i, false);
                BOOST_CHECK(not c.test(i));
                BOOST_CHECK_EQUAL(c.count(), 0UZ);
        }
}

// The two the readings cannot synthesize a position at a time: insert reports newness, and fill is bulk.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheInsertAndTheFill, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        constexpr auto N = T::extent;

        auto c = T();
        c.fill(true);
        BOOST_CHECK_EQUAL(c.count(), N);
        c.fill(false);
        BOOST_CHECK_EQUAL(c.count(), 0UZ);

        for (auto i = 0UZ; i < N; ++i) {
                BOOST_CHECK(c.insert(i));
        }
        BOOST_CHECK_EQUAL(c.count(), N);
}

namespace {

// The set reading: the positions held, in increasing order. The sequence reading is reference() itself.
template<class BB>
auto set_reading(BB const& b)
        -> std::vector<std::size_t>
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
auto probes(BB const& empty)
        -> std::vector<BB>
{
        auto const n = empty.size();
        auto out = std::vector<BB>{empty};

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

// The invariant on all three readings: the block-wise answer is the standard algorithm's, or it is wrong.
template<class BB>
auto disagreements(BB const& empty)
        -> int
{
        auto const values = probes(empty);
        auto n = 0;
        for (auto const& x : values) {
                for (auto const& y : values) {
                        auto const sx = set_reading(x);
                        auto const sy = set_reading(y);
                        if (std::lexicographical_compare_three_way(sx.begin(), sx.end(), sy.begin(), sy.end()) != set_lexicographical_compare_three_way(x, y)) {
                                ++n;
                        }
                        // No comparator: vector<bool>'s proxy converts to bool, so it is three_way_comparable.
                        auto const qx = reference(x);
                        auto const qy = reference(y);
                        if (std::lexicographical_compare_three_way(qx.begin(), qx.end(), qy.begin(), qy.end()) != sequence_lexicographical_compare_three_way(x, y)) {
                                ++n;
                        }
                        // The bitset reading is the sequence reading from the top, the bit string's order.
                        if (std::lexicographical_compare_three_way(qx.rbegin(), qx.rend(), qy.rbegin(), qy.rend()) != string_lexicographical_compare_three_way(x, y)) {
                                ++n;
                        }
                }
        }
        return n;
}

} // namespace

// All three orderings, at every static extent, against the algorithms that define them.
BOOST_AUTO_TEST_CASE_TEMPLATE(AllThreeOrderingsAgreeWithTheirReading, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        BOOST_CHECK_EQUAL(disagreements(T()), 0);
}

// The same at a run-time width, which shares no instantiation with the static one.
BOOST_AUTO_TEST_CASE_TEMPLATE(AllThreeOrderingsAgreeAtARunTimeWidth, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;
        constexpr auto D = test::digits_v<Block>;

        auto disagreed = 0;
        for (auto const n : {0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D}) {
                disagreed += disagreements(T(n));
        }
        BOOST_CHECK_EQUAL(disagreed, 0);
}

// Two pairs that separate the three readings pairwise: {0} against {1}, and {0,1} against {1}.
BOOST_AUTO_TEST_CASE(TheThreeOrderingsDisagree)
{
        using T = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>;

        using orderings = std::tuple<std::strong_ordering, std::strong_ordering, std::strong_ordering>;
        constexpr auto compare = [](std::initializer_list<std::size_t> p, std::initializer_list<std::size_t> q) -> orderings {
                auto x = T();
                for (auto const i : p) {
                        x.set(i);
                }
                auto y = T();
                for (auto const i : q) {
                        y.set(i);
                }
                return {set_lexicographical_compare_three_way(x, y), sequence_lexicographical_compare_three_way(x, y), string_lexicographical_compare_three_way(x, y)};
        };

        // {0} against {1}: [0] < [1]; [1,0] > [0,1]; "01" < "10".
        constexpr auto singletons = compare({0}, {1});
        static_assert(std::get<0>(singletons) == std::strong_ordering::less);
        static_assert(std::get<1>(singletons) == std::strong_ordering::greater);
        static_assert(std::get<2>(singletons) == std::strong_ordering::less);

        // {0,1} against {1}: [0,1] < [1]; [1,1] > [0,1]; "11" > "10".
        constexpr auto prefix = compare({0, 1}, {1});
        static_assert(std::get<0>(prefix) == std::strong_ordering::less);
        static_assert(std::get<1>(prefix) == std::strong_ordering::greater);
        static_assert(std::get<2>(prefix) == std::strong_ordering::greater);
}

// The prefix clause, which is the whole of what the set reading adds: {1} beats {} only by being longer.
BOOST_AUTO_TEST_CASE(TheSetOrderingPutsAPrefixFirst)
{
        using T = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>;

        auto x = T();
        x.set(1);
        auto const y = T();

        // {} is a prefix of {1}, so it sorts below -- the opposite of what holding the lower position would say.
        BOOST_CHECK(set_lexicographical_compare_three_way(x, y) == std::strong_ordering::greater);

        // And with something above that position, the clause no longer applies.
        auto z = T();
        z.set(8);
        BOOST_CHECK(set_lexicographical_compare_three_way(x, z) == std::strong_ordering::less);
}

// Dependent, so a storage without an allocator answers false rather than hard-errors.
template<class X>
using allocator_of = X::allocator_type;

template<class X>
constexpr bool has_allocator = requires (X const& x) { sizeof(allocator_of<X>); x.get_allocator(); };

// The allocator where the blocks have one, and max_size in bits at both widths.
BOOST_AUTO_TEST_CASE(TheAllocatorAndTheMaximumWidth)
{
        using V = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;
        static_assert(has_allocator<V>);
        static_assert(std::same_as<V::allocator_type, std::allocator<std::uint8_t>>);
        auto const alloc = std::allocator<std::uint8_t>();
        auto const empty = V(alloc);
        BOOST_CHECK_EQUAL(empty.size(), 0UZ);
        BOOST_CHECK(empty.get_allocator() == alloc);
        auto const nine = V(9UZ, alloc);
        BOOST_CHECK_EQUAL(nine.size(), 9UZ);
        BOOST_CHECK_EQUAL(nine.num_blocks(), 2UZ);
        BOOST_CHECK_EQUAL(nine.max_size() % V::bits_per_block, 0UZ);
        BOOST_CHECK_GE(nine.max_size(), std::vector<std::uint8_t>().max_size() / 2);

        using A = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>;
        static_assert(not has_allocator<A>);
        static_assert(A().max_size() == 9UZ);
}

// The three ceilings a reading can ask for: the storage computes all three and keeps none of them.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheThreeCeilingsAreComputedHereAndKeptAbove, Block, test::word_types)
{
        using V = xstd::detail::bits::contiguous_bit_vector<Block>;
        constexpr auto top = std::numeric_limits<std::size_t>::max();
        constexpr auto pmax = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());

        // Whole blocks, both widths, and the one a distance can name is the narrower by construction.
        static_assert(V::max_addressable_width % V::bits_per_block == 0UZ);
        static_assert(V::max_addressable_width == V::max_addressable_num_blocks * V::bits_per_block);
        static_assert(V::max_addressable_width <= pmax);
        static_assert(V::max_addressable_width > pmax - V::bits_per_block);
        static_assert(V::max_addressable_width < V::max_width);

        auto const v = V();

        // What the blocks can hold and a size_t can count, which is the set reading's answer.
        BOOST_CHECK_EQUAL(v.max_size() % V::bits_per_block, 0UZ);
        BOOST_CHECK_LE(v.max_size(), V::max_width);

        // boost::dynamic_bitset's answer, which saturates where that one clamps: the top of size_t.
        BOOST_CHECK_EQUAL(v.saturating_max_size(), top);
        BOOST_CHECK_EQUAL(v.saturating_max_size() - v.max_size(), V::bits_per_block - 1UZ);

        // std::vector<bool>'s answer, which clamps further, to what a difference_type can count.
        BOOST_CHECK_EQUAL(v.addressable_max_size(), V::max_addressable_width);
        BOOST_CHECK_LT(v.addressable_max_size(), v.max_size());

        // And the refusal the sequence reading spells every growth through: past that width, length_error.
        BOOST_CHECK_EQUAL(V::check_addressable_width(0UZ), 0UZ);
        BOOST_CHECK_EQUAL(V::check_addressable_width(V::max_addressable_width), V::max_addressable_width);
        BOOST_CHECK_THROW((void)V::check_addressable_width(V::max_addressable_width + 1UZ), std::length_error);
        BOOST_CHECK_THROW((void)V::check_addressable_width(top), std::length_error);
}

// The saturating sum every growth computes, and the block count it reaches, said at compile time.
BOOST_AUTO_TEST_CASE(TheBlockCountIsTotalAndTheSumThatReachesItSaturates)
{
        using V = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>;
        constexpr auto top = std::numeric_limits<std::size_t>::max();

        // Whole blocks, and no wider than what the blocks themselves can hold.
        static_assert(V::max_width % V::bits_per_block == 0UZ);
        static_assert(V::max_width == V::max_num_blocks * V::bits_per_block);
        BOOST_CHECK_LE(V().max_size(), V::max_width);

        // base + count where that is a width, and the top of size_t where it is not.
        static_assert(V::width_sum(3UZ, 4UZ) == 7UZ);
        static_assert(V::width_sum(top - 1UZ, 1UZ) == top);
        static_assert(V::width_sum(top, 0UZ) == top);
        static_assert(V::width_sum(top, 1UZ) == top);
        static_assert(V::width_sum(1UZ, top) == top);

        // Both arms at run time as well.
        BOOST_CHECK_EQUAL(V::width_sum(3UZ, 4UZ), 7UZ);
        BOOST_CHECK_EQUAL(V::width_sum(top, 1UZ), top);

        // A division that rounds up by the remainder: floored at one, exact on a boundary, one more just past it.
        static_assert(V::blocks_for(0UZ) == 1UZ);
        static_assert(V::blocks_for(1UZ) == 1UZ);
        static_assert(V::blocks_for(V::bits_per_block) == 1UZ);
        static_assert(V::blocks_for(V::bits_per_block + 1UZ) == 2UZ);
        static_assert(V::blocks_for(V::max_width) == V::max_num_blocks);

        // And total above that: the sixty-three widths where adding first would wrap to zero blocks.
        static_assert(V::blocks_for(V::max_width + 1UZ) == V::max_num_blocks + 1UZ);
        static_assert(V::blocks_for(top - 1UZ) == V::max_num_blocks + 1UZ);
        static_assert(V::blocks_for(top) == V::max_num_blocks + 1UZ);

        // Which is what every saturated sum arrives at, the two composing.
        static_assert(V::blocks_for(V::width_sum(top, 1UZ)) == V::max_num_blocks + 1UZ);
        static_assert(V::blocks_for(V::width_sum(V::max_width, V::bits_per_block)) == V::max_num_blocks + 1UZ);

        // Widths the blocks do hold are not refused at all, and each lands where it was asked for.
        auto v = V(8UZ);
        v.set(3UZ);
        v.resize(24UZ);
        BOOST_CHECK_EQUAL(v.size(), 24UZ);
        BOOST_CHECK_EQUAL(v.num_blocks(), V::blocks_for(24UZ));
        BOOST_CHECK(v.test(3UZ));
        BOOST_CHECK(v.growing_insert(31UZ));
        BOOST_CHECK_EQUAL(v.size(), 32UZ);
        BOOST_CHECK_EQUAL(v.count(), 2UZ);
}

// A word read and written at any position, and the ranged forms over it: both at a static width and at a run-time one.
using WordTypes = std::tuple<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 20>, xstd::detail::bits::contiguous_bit_vector<std::uint8_t>>;

namespace {

// Twenty bits with a fixed pattern, grown first where the width is a run-time one: the sample both word cases read.
template<class T>
auto word_sample()
        -> T
{
        auto b = T();
        if constexpr (requires { b.resize(20UZ); }) {
                b.resize(20UZ);
        }
        for (auto const i : {0UZ, 3UZ, 7UZ, 8UZ, 12UZ, 15UZ, 19UZ}) {
                b.set(i);
        }
        return b;
}

// A whole number of blocks, so there is no unused tail.
template<class T>
auto aligned_sample()
        -> T
{
        auto b = T();
        if constexpr (requires { b.resize(24UZ); }) {
                b.resize(24UZ);
        }
        for (auto const i : {0UZ, 3UZ, 7UZ, 8UZ, 12UZ, 15UZ, 19UZ, 23UZ}) {
                b.set(i);
        }
        return b;
}

// One start and length through set, flip and reset; a function so the sweeping case stays under the threshold.
template<class T>
auto check_ranged_forms(std::size_t n, std::size_t len)
        -> void
{
        auto e = word_sample<T>();
        auto r = reference(e);

        e.set(n, len, true);
        for (auto i = n; i < n + len; ++i) {
                r[i] = true;
        }
        BOOST_CHECK(reference(e) == r);

        e.flip(n, len);
        for (auto i = n; i < n + len; ++i) {
                r[i] = not r[i];
        }
        BOOST_CHECK(reference(e) == r);

        e.set(n, len, false);
        for (auto i = n; i < n + len; ++i) {
                r[i] = false;
        }
        BOOST_CHECK(reference(e) == r);
}

} // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(WordsAreReadAndWrittenAtAnyPosition, T, WordTypes)
{
        auto const c = word_sample<T>();
        // Blocks: 0b1000'1001, 0b1001'0001, 0b0000'1000.
        BOOST_CHECK_EQUAL(c.block_at(0UZ), 0b1000'1001);
        BOOST_CHECK_EQUAL(c.block_at(8UZ), 0b1001'0001);
        BOOST_CHECK_EQUAL(c.block_at(3UZ), 0b0011'0001);
        BOOST_CHECK_EQUAL(c.block_at(12UZ), 0b1000'1001);
        BOOST_CHECK_EQUAL(c.block_at(16UZ), 0b0000'1000);
        BOOST_CHECK_EQUAL(c.block_at(17UZ), 0b0000'0100);

        // block_at lands the masked bits and nothing else, the mask never selecting past size().
        auto d = word_sample<T>();
        d.block_at(3UZ, 0b1111'1111, 0b0001'1110);
        auto m = reference(c);
        for (auto const i : {4UZ, 5UZ, 6UZ, 7UZ}) {
                m[i] = true;
        }
        BOOST_CHECK(reference(d) == m);
        d.block_at(5UZ, 0b0000'0000, 0b0111'1000);
        for (auto const i : {8UZ, 9UZ, 10UZ, 11UZ}) {
                m[i] = false;
        }
        BOOST_CHECK(reference(d) == m);
        d.block_at(16UZ, 0b1111'1111, 0b0000'1111);
        for (auto const i : {16UZ, 17UZ, 18UZ, 19UZ}) {
                m[i] = true;
        }
        BOOST_CHECK(reference(d) == m);
        BOOST_CHECK_EQUAL(d.block(2), 0b0000'1111);
}

// The ranged forms: every start and length, whole words and partial ones, against the model.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheRangedFormsGoAWordAtATime, T, WordTypes)
{
        constexpr auto D = 8UZ;
        for (auto const n : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 15UZ}) {
                for (auto const len : {0UZ, 1UZ, D - 1, D, D + 1, 20UZ - n}) {
                        if (n + len <= 20UZ) {
                                check_ranged_forms<T>(n, len);
                        }
                }
        }
}

// Three blocks with no tail, so a shift's destination block is exactly the splice and nothing masks it afterwards.
using AlignedWordTypes = std::tuple<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 24>, xstd::detail::bits::contiguous_bit_vector<std::uint8_t>>;

// The identity behind one primitive for all three sites: a left shift reads one block lower than a right.
BOOST_AUTO_TEST_CASE_TEMPLATE(BothShiftsAreWordAtOnTheOperand, T, AlignedWordTypes)
{
        constexpr auto D = 8UZ;
        auto const c = aligned_sample<T>();
        auto const last = c.num_blocks() - 1UZ;

        for (auto n = 0UZ; n < c.size(); ++n) {
                auto const n_blocks = n / D;

                auto r = c;
                r >>= n;
                for (auto i = 0UZ; i + n_blocks <= last; ++i) {
                        BOOST_CHECK_EQUAL(r.block(i), c.block_at((i * D) + n));
                }

                auto l = c;
                l <<= n;
                for (auto i = n_blocks + 1UZ; i <= last; ++i) {
                        BOOST_CHECK_EQUAL(l.block(i), c.block_at((i * D) - n));
                }
        }
}

BOOST_AUTO_TEST_SUITE_END()
