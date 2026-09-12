//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                               // digits_v, graded_extents, word_types
#include <test/inplace_vector.hpp>                            // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/uint128.hpp>                                   // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_traits.hpp>                           // bit_storage, bit_traits, block_readable, static_bit_extent
#include <xstd/bits/detail/contiguous_bit_array.hpp>          // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_container.hpp>      // contiguous_bit_container
#include <xstd/bits/detail/contiguous_block_container.hpp>    // contiguous_block_container
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // IWYU pragma: keep; contiguous_bit_inplace_vector, named only under TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/detail/contiguous_bit_vector.hpp>         // contiguous_bit_vector
#include <boost/test/unit_test.hpp>                           // BOOST_CHECK_EQUAL, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <algorithm>                                          // count, lexicographical_compare_three_way, min
#include <array>                                              // array
#include <bitset>                                             // bitset
#include <compare>                                            // strong_ordering
#include <concepts>                                           // same_as
#include <cstddef>                                            // size_t
#include <cstdint>                                            // uint8_t, uint64_t
#include <initializer_list>                                   // initializer_list
#include <memory>                                             // addressof, allocator
#include <new>                                                // IWYU pragma: keep; bad_alloc, behind TEST_HAS_INPLACE_VECTOR
#include <ranges>                                             // begin, iota, size
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

        // The two sites where a comparison becomes a count, so the cast is not thirty. [design.md#counted-not-asserted]
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
        :
                m_x(x),
                m_y(y),
                m_disagreements(disagreements),
                m_a(a),
                m_b(b)
        {}

        auto width()
                -> void
        {
                unequal(m_x.count(), m_cardinality);
                disagree(m_x.any(),  m_cardinality != 0);
                disagree(m_x.none(), m_cardinality == 0);
                disagree(m_x.all(),  m_cardinality == m_n);
                disagree(m_x == m_y, m_mx == m_my);
        }

        // find_front/find_back assert any(); find_first/find_last are total and answer size().
        auto scans()
                -> void
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
        auto relational()
                -> void
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
        auto bitwise()
                -> void
        {
                { auto& a = fresh_x(); a &= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] and     m_my[i]; } same(m, a); }
                { auto& a = fresh_x(); a |= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] or      m_my[i]; } same(m, a); }
                { auto& a = fresh_x(); a ^= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] !=      m_my[i]; } same(m, a); }
                { auto& a = fresh_x(); a -= m_y; auto m = model(m_n); for (auto i = 0UZ; i < m_n; ++i) { m[i] = m_mx[i] and not m_my[i]; } same(m, a); }
        }

        auto shifts()
                -> void
        {
                for (auto s = 0UZ; s < m_n; ++s) {
                        { auto& a = fresh_x(); a <<= s; auto m = model(m_n); for (auto i = s;  i < m_n;     ++i) { m[i] = m_mx[i - s]; } same(m, a); }
                        { auto& a = fresh_x(); a >>= s; auto m = model(m_n); for (auto i = 0UZ; i + s < m_n; ++i) { m[i] = m_mx[i + s]; } same(m, a); }
                }
        }

        // One method apiece: combined, GCC 15 at -O3 reports a free-nonheap-object that is not there. [design.md#scratch-objects]
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
                a.swap(b);
                disagree(a == m_y, true);
                disagree(b == m_x, true);
        }

        // Per bit, including the two that report whether the bit was already there.
        auto positions()
                -> void
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
        auto blocks()
                -> void
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

// Seven patterns, every pair landing on both sides of each branch. [design.md#seven-patterns]

template<class BB>
auto sweep(BB const& empty)
        -> int
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

// Both shipped vehicles satisfy contiguous_block_container: growth is detected where it exists, never required.
BOOST_AUTO_TEST_CASE(ItsStorageIsAContiguousSizedRangeOfUnsignedIntegers)
{
        static_assert(xstd::detail::bits::contiguous_block_container<std::array<std::uint8_t, 4>>);
        static_assert(xstd::detail::bits::contiguous_block_container<std::vector<std::uint64_t>>);

        static_assert(not xstd::detail::bits::contiguous_block_container<std::vector<bool>>);      // not a contiguous range
        static_assert(not xstd::detail::bits::contiguous_block_container<std::vector<int>>);       // nor unsigned integers

        // The element clause is unsigned_integer and not the wider bitwise_operators, which std::bitset would satisfy: a block is asked for the <bit> intrinsics too, and they are constrained on unsigned_integer. [design.md#contiguous-block-container]
        static_assert(not xstd::detail::bits::contiguous_block_container<std::array<std::bitset<64>, 4>>);
}

// The semantic half a concept cannot check: a[i] is *(begin(a) + i), the same object and not merely an equal one.
template<class Blocks>
constexpr auto subscript_agrees_with_iteration(Blocks blocks) noexcept
        -> bool
{
        // The index is the range's own difference_type, so begin(blocks) + i needs no conversion; subscript takes the container's size_type, which the concept names and which is the one cast, keeping -Wsign-conversion honest.
        for (auto i = std::ranges::range_difference_t<Blocks>{}; i < std::ranges::ssize(blocks); ++i) {
                if (std::addressof(blocks[static_cast<Blocks::size_type>(i)]) != std::addressof(*(std::ranges::begin(blocks) + i))) {
                        return false;
                }
        }
        return true;
}

BOOST_AUTO_TEST_CASE(ItsStorageSubscriptIsIterationAtTheSameAddress)
{
        static_assert(subscript_agrees_with_iteration(std::array<std::uint8_t, 4>{ 1, 2, 3, 4 }));
        static_assert(subscript_agrees_with_iteration(std::vector<std::uint64_t>{ 1, 2, 3, 4 }));
        BOOST_CHECK(subscript_agrees_with_iteration(std::vector<std::uint64_t>{ 1, 2, 3, 4 }));
}

// ranges::swap finds a free swap by ADL and a member never, so contiguous_bit_container needs the free one its three adaptors already have: without it every container moves a whole contiguous_bit_container three times instead of swapping its blocks once, and a storage with an optimized swap never sees it. [design.md#swap-goes-through-adl]
namespace {

int g_storage_swaps = 0;
int g_storage_moves = 0;

// A storage satisfying contiguous_block_container whose swap and moves are distinguishable.
struct counting_blocks
{
        using size_type = std::size_t;

        std::array<std::uint64_t, 4> m_data {};

        // The move operations are counted rather than used: once the free swap exists nothing calls them, which is the point of the test, so they and the members that only satisfy the concept say so.
        counting_blocks() = default;
        [[maybe_unused]] counting_blocks(counting_blocks const&) = default;
        [[maybe_unused]] auto operator=(counting_blocks const&) -> counting_blocks& = default;
        [[maybe_unused]] counting_blocks(counting_blocks&& other) noexcept : m_data(other.m_data) { ++g_storage_moves; }
        [[maybe_unused]] auto operator=(counting_blocks&& other) noexcept -> counting_blocks& { m_data = other.m_data; ++g_storage_moves; return *this; }
        [[maybe_unused]] ~counting_blocks() = default;

        [[nodiscard, maybe_unused]] auto begin()       -> std::uint64_t*       { return m_data.data(); }
        [[nodiscard, maybe_unused]] auto begin() const -> std::uint64_t const* { return m_data.data(); }
        [[nodiscard, maybe_unused]] auto end()         -> std::uint64_t*       { return m_data.data() + m_data.size(); }
        [[nodiscard, maybe_unused]] auto end()   const -> std::uint64_t const* { return m_data.data() + m_data.size(); }
        [[nodiscard, maybe_unused]] auto size()  const -> std::size_t          { return m_data.size(); }

        [[nodiscard, maybe_unused]] auto operator[](size_type n)       -> std::uint64_t&       { return m_data[n]; }
        [[nodiscard, maybe_unused]] auto operator[](size_type n) const -> std::uint64_t const& { return m_data[n]; }

        [[maybe_unused]] auto operator==(counting_blocks const&) const -> bool = default;

        friend auto swap(counting_blocks& x, counting_blocks& y) noexcept
                -> void
        {
                ++g_storage_swaps;
                x.m_data.swap(y.m_data);
        }
};

}       // namespace

BOOST_AUTO_TEST_CASE(ItsSwapIsReachedThroughAdlAndNotTheMoveFallback)
{
        using bits = xstd::detail::bits::contiguous_bit_container<counting_blocks, 256>;

        auto a = bits();
        auto b = bits();

        g_storage_swaps = 0;
        g_storage_moves = 0;
        a.swap(b);
        BOOST_CHECK_EQUAL(g_storage_swaps, 1);          // the member reaches the storage's swap
        BOOST_CHECK_EQUAL(g_storage_moves, 0);

        g_storage_swaps = 0;
        g_storage_moves = 0;
        std::ranges::swap(a, b);                        // and so does what every adaptor actually calls
        BOOST_CHECK_EQUAL(g_storage_swaps, 1);          // 0 swaps and 3 moves before the free swap existed
        BOOST_CHECK_EQUAL(g_storage_moves, 0);
}

// A compile-time width costs nothing: the absent size member takes no storage.
BOOST_AUTO_TEST_CASE(AStaticWidthAddsNothingToItsBlocks)
{
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint64_t,  64>) == sizeof(std::array<std::uint64_t,  1>));
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint8_t,  129>) == sizeof(std::array<std::uint8_t,  17>));
        static_assert(sizeof(xstd::detail::bits::contiguous_bit_array<std::uint8_t,    0>) == sizeof(std::array<std::uint8_t,   1>));

        static_assert(    xstd::detail::bits::contiguous_bit_array<std::size_t, 64>::has_static_size);
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
        for (auto const n : { 0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D, (3 * D) + 1 }) {
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
        BOOST_CHECK(b.all());           // vacuously, as std::bitset<0>::all() is
        BOOST_CHECK(not b.any());
}

// That sole block is entirely padding, which is what the last-block mask exists to say. [design.md#padding]
BOOST_AUTO_TEST_CASE(AZeroWidthsOneBlockIsAllPaddingAndStaysZero)
{
        auto b = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>(0);

        b.set();
        BOOST_CHECK(b.none());
        b.flip();
        BOOST_CHECK(b.none());
        BOOST_CHECK_EQUAL(b.block(0), 0U);
}

// Default-constructed is zero-width, not block-less. [design.md#default-construction]
BOOST_AUTO_TEST_CASE(ADefaultConstructedRunTimeWidthIsZeroWidthWithOneBlock)
{
        auto const b = xstd::detail::bits::contiguous_bit_vector<std::uint8_t>();

        BOOST_CHECK_EQUAL(b.size(), 0UZ);
        BOOST_CHECK_EQUAL(b.num_blocks(), 1UZ);
        BOOST_CHECK(b == xstd::detail::bits::contiguous_bit_vector<std::uint8_t>(0));
}

namespace {

// A run-time width built from the model, so equality against it doubles as the invariant check: a dirty tail compares unequal.
template<class T>
[[nodiscard]] auto from_model(model const& m)
        -> T
{
        auto b = T(m.size());
        for (auto i = 0UZ; i < m.size(); ++i) {
                if (m[i]) { b.set(i); }
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
        return { 0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D, (3 * D) + 1 };
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
                // Cast back before the mask: a shifted narrow word is an int, which bugprone-signed-bitwise reads as a signed operand.
                m.push_back((static_cast<Block>(value >> i) & Block{1}) != Block{0});
        }
}

// Alternating pairs of bits, so a split at any offset lands ones on both sides.
template<class X> constexpr bool can_resize    = requires (X& x) { x.resize(1UZ); x.resize(1UZ, true); };
template<class X> constexpr bool can_push_pop  = requires (X& x) { x.push_back(true); x.pop_back(); };
template<class X> constexpr bool can_append    = requires (X& x) { x.append(x.block(0UZ)); };
template<class X> constexpr bool can_clear     = requires (X& x) { x.clear(); };
template<class X> constexpr bool can_reserve   = requires (X& x) { x.reserve(1UZ); x.shrink_to_fit(); };
template<class X> constexpr bool has_capacity  = requires (X const& x) { x.capacity(); };

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

}       // namespace

// Every resize path: each graded width to each other, with both fill values, against the model and against a fresh build from it. [design.md#growth]
BOOST_AUTO_TEST_CASE_TEMPLATE(ResizingKeepsTheModelAndTheUnusedTailClear, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;

        auto disagreements = 0;
        for (auto const from : graded_widths<Block>()) {
                for (auto const to : graded_widths<Block>()) {
                        for (auto const value : { false, true }) {
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

// Boost's append: a whole block at once, split across two where the width is not aligned; at width zero the floor block takes it.
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
                auto const blocks = std::array{ striped<Block>(), static_cast<Block>(~striped<Block>()), Block{1} };
                b.append(blocks.begin(), blocks.end());
                for (auto const value : blocks) {
                        append_to(m, value);
                }
                disagreements += static_cast<int>(b != from_model<T>(m));
                disagreements += static_cast<int>(b.size() != n + (4 * test::digits_v<Block>));
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

// Width zero, one block, all padding: the object a default constructor makes. [design.md#default-construction]
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
// No hole in front of the blocks at any alignment: the width takes theirs where they out-align a size_t, so the class is its two members and nothing else, which is what -Wpadded asks of it. [design.md#padding]
BOOST_AUTO_TEST_CASE(TheWidthFillsWhatWouldOtherwisePadTheBlocks)
{
        // The width slot is a size_t, or the blocks' alignment where that is wider.
        constexpr auto tiles = [](std::size_t whole, std::size_t blocks, std::size_t block_align) {
                return whole == blocks + std::ranges::max(sizeof(std::size_t), block_align);
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

// The third storage: a run-time width under a compile-time capacity, the sweep unchanged over it, and growth past the capacity a bad_alloc. [design.md#growth]
BOOST_AUTO_TEST_CASE(AnInplaceVectorIsARunTimeWidthUnderAStaticCapacity)
{
        using T = xstd::detail::bits::contiguous_bit_inplace_vector<std::uint8_t, 24>;
        static_assert(not T::has_static_size);
        static_assert(xstd::detail::bits::contiguous_block_container<std::inplace_vector<std::uint8_t, 3>>);

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
}
#endif

// Each entry reaches the member it names, and every call stays inside the kept contracts. [design.md#the-cheapest-contract]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheTraitsForwardToTheStorage, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        static_assert(xstd::bit_storage<xstd::bit_traits<T>, T>);
        static_assert(xstd::static_bit_extent<xstd::bit_traits<T>, T>);
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
                traits::unchecked_assign(c, i, true);
                BOOST_CHECK(traits::at(c, i));
                BOOST_CHECK_EQUAL(traits::count(c), 1UZ);
                BOOST_CHECK_EQUAL(traits::find_first(c), i);
                BOOST_CHECK_EQUAL(traits::find_prev(c, i + 1UZ), i);
                BOOST_CHECK_EQUAL(traits::find_next(c, i), N);

                traits::unchecked_assign(c, i, false);
                BOOST_CHECK(not traits::at(c, i));
                BOOST_CHECK_EQUAL(traits::count(c), 0UZ);
        }
}

// The two entries the readings cannot synthesize, in their own case: insert can grow where the storage allows, and fill is bulk. [design.md#what-the-trait-reconciles]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheTraitsInsertAndFill, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
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

// The invariant on all three readings: the block-wise answer is the standard algorithm's, or it is wrong. [design.md#the-ordering-invariant]
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
                        if (std::lexicographical_compare_three_way(sx.begin(), sx.end(), sy.begin(), sy.end()) != x.set_three_way(y)) {
                                ++n;
                        }
                        // No comparator: vector<bool>'s proxy converts to bool, which is what makes it three_way_comparable.
                        auto const qx = reference(x);
                        auto const qy = reference(y);
                        if (std::lexicographical_compare_three_way(qx.begin(), qx.end(), qy.begin(), qy.end()) != x.sequence_three_way(y)) {
                                ++n;
                        }
                        // The bitset reading is the sequence reading traversed from the top, which is the bit string's order.
                        if (std::lexicographical_compare_three_way(qx.rbegin(), qx.rend(), qy.rbegin(), qy.rend()) != x.bitset_three_way(y)) {
                                ++n;
                        }
                }
        }
        return n;
}

}       // namespace

// All three orderings, at every static extent, against the algorithms that define them. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE_TEMPLATE(AllThreeOrderingsAgreeWithTheirReading, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        BOOST_CHECK_EQUAL(disagreements(T()), 0);
}

// The same at a run-time width, which shares no instantiation with the static one. [design.md#per-instantiation-slots]
BOOST_AUTO_TEST_CASE_TEMPLATE(AllThreeOrderingsAgreeAtARunTimeWidth, Block, test::word_types)
{
        using T = xstd::detail::bits::contiguous_bit_vector<Block>;
        constexpr auto D = test::digits_v<Block>;

        auto disagreed = 0;
        for (auto const n : { 0UZ, 1UZ, D - 1, D, D + 1, (2 * D) - 1, 2 * D, (2 * D) + 1, 3 * D }) {
                disagreed += disagreements(T(n));
        }
        BOOST_CHECK_EQUAL(disagreed, 0);
}

// Two pairs that separate the three readings pairwise: {0} against {1}, and {0,1} against {1}. [design.md#two-readings-disagree]
BOOST_AUTO_TEST_CASE(TheThreeOrderingsDisagree)
{
        using T = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>;

        using orderings = std::tuple<std::strong_ordering, std::strong_ordering, std::strong_ordering>;
        constexpr auto compare = [](std::initializer_list<std::size_t> p, std::initializer_list<std::size_t> q) -> orderings {
                auto x = T();
                for (auto const i : p) { x.set(i); }
                auto y = T();
                for (auto const i : q) { y.set(i); }
                return { x.set_three_way(y), x.sequence_three_way(y), x.bitset_three_way(y) };
        };

        // {0} against {1}: [0] < [1]; [1,0] > [0,1]; "01" < "10".
        constexpr auto singletons = compare({ 0 }, { 1 });
        static_assert(std::get<0>(singletons) == std::strong_ordering::less);
        static_assert(std::get<1>(singletons) == std::strong_ordering::greater);
        static_assert(std::get<2>(singletons) == std::strong_ordering::less);

        // {0,1} against {1}: [0,1] < [1]; [1,1] > [0,1]; "11" > "10".
        constexpr auto prefix = compare({ 0, 1 }, { 1 });
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
        BOOST_CHECK(x.set_three_way(y) == std::strong_ordering::greater);

        // And with something above that position, the clause no longer applies.
        auto z = T();
        z.set(8);
        BOOST_CHECK(x.set_three_way(z) == std::strong_ordering::less);
}

// Three named entries, so a caller says which reading it means rather than being handed one. [design.md#two-readings-disagree]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheTraitsNameAllThreeOrderings, T, test::graded_extents<xstd::detail::bits::contiguous_bit_array>)
{
        using traits = xstd::bit_traits<T>;

        // Over the same probes, so a width with nothing to differ at is covered by the same code as any other.
        auto const values = probes(T());
        for (auto const& x : values) {
                for (auto const& y : values) {
                        BOOST_CHECK(traits::set_three_way(x, y)      == x.set_three_way(y));
                        BOOST_CHECK(traits::sequence_three_way(x, y) == x.sequence_three_way(y));
                        BOOST_CHECK(traits::bitset_three_way(x, y)   == x.bitset_three_way(y));
                }
        }
}

// Dependent, so a storage without an allocator answers false; the alias spells the typedef without a typename, which clang-tidy 22 reads as redundant.
template<class X>
using allocator_of = X::allocator_type;

template<class X>
constexpr bool has_allocator = requires (X const& x) { sizeof(allocator_of<X>); x.get_allocator(); };

// The allocator where the blocks have one, and max_size in bits at both widths. [design.md#a-strict-extension]
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

// A word read and written at any position, and the ranged forms over it: both at a static width and at a run-time one. [design.md#the-blit]
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
        for (auto const i : { 0UZ, 3UZ, 7UZ, 8UZ, 12UZ, 15UZ, 19UZ }) {
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
        for (auto const i : { 0UZ, 3UZ, 7UZ, 8UZ, 12UZ, 15UZ, 19UZ, 23UZ }) {
                b.set(i);
        }
        return b;
}

// One start and length through set, flip and reset, each against the model; a function rather than a loop body so the case that sweeps it stays under readability-function-cognitive-complexity's threshold.
template<class T>
auto check_ranged_forms(std::size_t n, std::size_t len)
        -> void
{
        auto e = word_sample<T>();
        auto r = reference(e);

        e.set(n, len, true);
        for (auto i = n; i < n + len; ++i) { r[i] = true; }
        BOOST_CHECK(reference(e) == r);

        e.flip(n, len);
        for (auto i = n; i < n + len; ++i) { r[i] = not r[i]; }
        BOOST_CHECK(reference(e) == r);

        e.set(n, len, false);
        for (auto i = n; i < n + len; ++i) { r[i] = false; }
        BOOST_CHECK(reference(e) == r);
}

}       // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(WordsAreReadAndWrittenAtAnyPosition, T, WordTypes)
{
        auto const c = word_sample<T>();
        // Blocks: 0b1000'1001, 0b1001'0001, 0b0000'1000.
        BOOST_CHECK_EQUAL(c.word_at(0UZ),  0b1000'1001);
        BOOST_CHECK_EQUAL(c.word_at(8UZ),  0b1001'0001);
        BOOST_CHECK_EQUAL(c.word_at(3UZ),  0b0011'0001);
        BOOST_CHECK_EQUAL(c.word_at(12UZ), 0b1000'1001);
        BOOST_CHECK_EQUAL(c.word_at(16UZ), 0b0000'1000);
        BOOST_CHECK_EQUAL(c.word_at(17UZ), 0b0000'0100);

        // set_word lands the masked bits and nothing else, across two blocks and into the tail, which stays clear.
        auto d = word_sample<T>();
        d.set_word(3UZ, 0b1111'1111, 0b0001'1110);
        auto m = reference(c);
        for (auto const i : { 4UZ, 5UZ, 6UZ, 7UZ }) { m[i] = true; }
        BOOST_CHECK(reference(d) == m);
        d.set_word(5UZ, 0b0000'0000, 0b0111'1000);
        for (auto const i : { 8UZ, 9UZ, 10UZ, 11UZ }) { m[i] = false; }
        BOOST_CHECK(reference(d) == m);
        d.set_word(16UZ, 0b1111'1111, 0b1111'1111);
        for (auto const i : { 16UZ, 17UZ, 18UZ, 19UZ }) { m[i] = true; }
        BOOST_CHECK(reference(d) == m);
        BOOST_CHECK_EQUAL(d.block(2), 0b0000'1111);
}

// The ranged forms: every start and length, whole words and partial ones, against the model.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheRangedFormsGoAWordAtATime, T, WordTypes)
{
        constexpr auto D = 8UZ;
        for (auto const n : { 0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 15UZ }) {
                for (auto const len : { 0UZ, 1UZ, D - 1, D, D + 1, 20UZ - n }) {
                        if (n + len <= 20UZ) {
                                check_ranged_forms<T>(n, len);
                        }
                }
        }
}

// Three blocks with no tail, so a shift's destination block is exactly the splice and nothing masks it afterwards.
using AlignedWordTypes = std::tuple<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 24>, xstd::detail::bits::contiguous_bit_vector<std::uint8_t>>;

// The identity that lets one primitive serve all three sites: a right shift's destination block is word_at at that position of the operand, and a left shift's is the same read one block lower. [design.md#the-funnel-shift]
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
                        BOOST_CHECK_EQUAL(r.block(i), c.word_at((i * D) + n));
                }

                auto l = c;
                l <<= n;
                for (auto i = n_blocks + 1UZ; i <= last; ++i) {
                        BOOST_CHECK_EQUAL(l.block(i), c.word_at((i * D) - n));
                }
        }
}

BOOST_AUTO_TEST_SUITE_END()
