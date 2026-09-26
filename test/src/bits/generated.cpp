//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits.hpp>            // bit_array, bit_set, bit_fixed_set, bit_vector, bitset, dynamic_bitset, and the bounded column
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <compare>                  // three_way_comparable
#include <concepts>                 // copyable, default_initializable, movable, ranges::swap, swappable, totally_ordered
#include <cstddef>                  // size_t
#include <memory_resource>          // polymorphic_allocator
#include <scoped_allocator>         // scoped_allocator_adaptor
#include <type_traits>              // is_nothrow_move_assignable_v, is_nothrow_move_constructible_v, is_trivially_copyable_v
#include <utility>                  // move, swap
#include <vector>                   // vector

// What the compiler generates for each cell, held to the table rather than to whichever cell was read last.
BOOST_AUTO_TEST_SUITE(Generated)

namespace {

inline constexpr auto N = 128UZ;

template<class T>
concept has_swap_member = requires (T& a, T& b) { a.swap(b); };
template<class T>
concept has_swap_free = requires (T& a, T& b) { swap(a, b); };
template<class T>
concept has_get_allocator = requires (T const& a) { a.get_allocator(); };

// Every cell answers the same to all of these; only the allocator differs, and by column.
template<class T>
constexpr auto is_regular_container()
        -> bool
{
        static_assert(std::default_initializable<T>);
        static_assert(std::copyable<T>);
        static_assert(std::movable<T>);
        static_assert(std::totally_ordered<T>);
        static_assert(std::three_way_comparable<T>);

        // Not merely swappable through the implicit moves: the storage's own exchange, member and free.
        static_assert(has_swap_member<T>);
        static_assert(has_swap_free<T>);
        static_assert(std::swappable<T>);

        // A move that could throw would cost every growing container its strong guarantee.
        static_assert(std::is_nothrow_move_constructible_v<T>);
        static_assert(std::is_nothrow_move_assignable_v<T>);
        return true;
}

// The allocator varies by column, not by row: a storage that allocates has one, a static width has none.
template<class T>
constexpr auto allocator_aware()
        -> bool
{
        static_assert(has_get_allocator<T>);
        return true;
}

template<class T>
constexpr auto not_allocator_aware()
        -> bool
{
        static_assert(not has_get_allocator<T>);
        return true;
}

template<class T>
concept free_swap_is_nothrow = requires (T& a, T& b) { requires noexcept(swap(a, b)); };
template<class T>
concept member_swap_is_nothrow = requires (T& a, T& b) { requires noexcept(a.swap(b)); };
template<class T>
concept std_swap_is_nothrow = requires (T& a, T& b) { requires noexcept(std::swap(a, b)); };
template<class T>
concept cpo_swap_is_nothrow = requires (T& a, T& b) { requires noexcept(std::ranges::swap(a, b)); };

// Values come out exchanged whichever swap ran, so the two are told apart by noexcept instead. The
// allocator has to make std itself an associated namespace, which is why it is the adaptor and not the
// polymorphic_allocator inside it: ADL associates the innermost enclosing namespace, and std::pmr does
// not reach std::swap. It propagates on neither swap nor move assignment and is not always equal, so
// move assignment may allocate and throw where an exchange of pointers cannot. A nothrow free swap is
// therefore the library's own; std::swap over these cells is potentially throwing.
template<class T>
constexpr auto free_swap_is_not_std_swap()
        -> bool
{
        static_assert(member_swap_is_nothrow<T>);
        static_assert(not std_swap_is_nothrow<T>);
        static_assert(free_swap_is_nothrow<T>);

        // The customization point reaches it too. Its own ADL step declares a deleted swap template to keep
        // std::swap out, and that template is an exact match: a swap the library declared one class further
        // up would lose to it on the conversion and take the whole ADL step down with it, leaving the moves.
        static_assert(cpo_swap_is_nothrow<T>);
        return true;
}

} // namespace

BOOST_AUTO_TEST_CASE(EveryCellIsARegularContainer)
{
        static_assert(is_regular_container<xstd::bit_fixed_set<N>>());
        static_assert(is_regular_container<xstd::bit_set>());
        static_assert(is_regular_container<xstd::bit_array<N>>());
        static_assert(is_regular_container<xstd::bit_vector>());
        static_assert(is_regular_container<xstd::bitset<N>>());
        static_assert(is_regular_container<xstd::dynamic_bitset>());
#ifdef __cpp_lib_inplace_vector

        static_assert(is_regular_container<xstd::bit_bounded_set<N>>());
        static_assert(is_regular_container<xstd::bit_bounded_vector<N>>());
        static_assert(is_regular_container<xstd::bounded_bitset<N>>());

#endif
        BOOST_CHECK(true);
}

// The implicit moves ask the storage, whose move assignment may throw under a polymorphic allocator.
BOOST_AUTO_TEST_CASE(TheMovesAreAsNothrowAsTheStorages)
{
        using allocator_type = std::pmr::polymorphic_allocator<std::size_t>;

        static_assert(std::is_nothrow_move_constructible_v<xstd::basic_bit_set<std::size_t, allocator_type>>);
        static_assert(std::is_nothrow_move_constructible_v<xstd::basic_bit_vector<std::size_t, allocator_type>>);
        static_assert(std::is_nothrow_move_constructible_v<xstd::basic_dynamic_bitset<std::size_t, allocator_type>>);

        static_assert(not std::is_nothrow_move_assignable_v<xstd::basic_bit_set<std::size_t, allocator_type>>);
        static_assert(not std::is_nothrow_move_assignable_v<xstd::basic_bit_vector<std::size_t, allocator_type>>);
        static_assert(not std::is_nothrow_move_assignable_v<xstd::basic_dynamic_bitset<std::size_t, allocator_type>>);

        // The static column's four stay trivial, as its blocks' are.
        static_assert(std::is_trivially_copyable_v<xstd::bit_fixed_set<N>>);
        static_assert(std::is_trivially_copyable_v<xstd::bit_array<N>>);
        static_assert(std::is_trivially_copyable_v<xstd::bitset<N>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheAllocatorFollowsTheColumnAndNotTheRow)
{
        // The dynamic column allocates, so all three rows of it answer.
        static_assert(allocator_aware<xstd::bit_set>());
        static_assert(allocator_aware<xstd::bit_vector>());
        static_assert(allocator_aware<xstd::dynamic_bitset>());

        // The static column is a std::array, which has no allocator for any row to show.
        static_assert(not_allocator_aware<xstd::bit_fixed_set<N>>());
        static_assert(not_allocator_aware<xstd::bit_array<N>>());
        static_assert(not_allocator_aware<xstd::bitset<N>>());

        // The bounded column holds its blocks inline, so it has none either.
#ifdef __cpp_lib_inplace_vector

        static_assert(not_allocator_aware<xstd::bit_bounded_set<N>>());
        static_assert(not_allocator_aware<xstd::bit_bounded_vector<N>>());
        static_assert(not_allocator_aware<xstd::bounded_bitset<N>>());

#endif
        BOOST_CHECK(true);
}

// Exchanged values say nothing about which overload did it, so the allocating column says so by noexcept.
// The six static cells name no allocator, which keeps std out of their associated namespaces entirely.
BOOST_AUTO_TEST_CASE(TheFreeSwapIsTheLibrarysAndNotStdSwap)
{
        using block_type = std::size_t;
        using allocator_type = std::scoped_allocator_adaptor<std::pmr::polymorphic_allocator<block_type>>;

        static_assert(free_swap_is_not_std_swap<xstd::basic_bit_set<block_type, allocator_type>>());
        static_assert(free_swap_is_not_std_swap<xstd::basic_bit_vector<block_type, allocator_type>>());
        static_assert(free_swap_is_not_std_swap<xstd::basic_dynamic_bitset<block_type, allocator_type>>());
        BOOST_CHECK(true);
}

// A swap falling back on the implicit moves would still compile every assertion above, so moves are checked.
BOOST_AUTO_TEST_CASE(SwapExchangesTheValues)
{
        auto a = xstd::bit_fixed_set<N>();
        auto b = xstd::bit_fixed_set<N>();
        a.insert(1UZ);
        b.insert(2UZ);

        a.swap(b);
        BOOST_CHECK(a.contains(2UZ) and not a.contains(1UZ));
        BOOST_CHECK(b.contains(1UZ) and not b.contains(2UZ));

        swap(a, b);
        BOOST_CHECK(a.contains(1UZ) and not a.contains(2UZ));
        BOOST_CHECK(b.contains(2UZ) and not b.contains(1UZ));
}

namespace {

// Each reading's own way of saying empty and of growing by one, asked of an owner whose value was moved away.
template<class T>
auto a_moved_from_sequence_grows_again()
        -> bool
{
        auto source = T(100UZ, true);
        auto const target = std::move(source);
        auto const empty = source.empty(); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): the moved-from state is the check.
        source.push_back(true);            // NOLINT(clang-analyzer-cplusplus.Move): growing the moved-from state is the check.
        return empty and target.size() == 100UZ and source.size() == 1UZ and source[0];
}

template<class T>
auto a_moved_from_set_grows_again()
        -> bool
{
        auto source = T();
        source.insert(100UZ);
        auto const target = std::move(source);
        auto const empty = source.empty(); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): the moved-from state is the check.
        source.insert(3UZ);                // NOLINT(clang-analyzer-cplusplus.Move): growing the moved-from state is the check.
        return empty and target.contains(100UZ) and source.size() == 1UZ and source.contains(3UZ);
}

template<class T>
auto a_moved_from_bitset_grows_again()
        -> bool
{
        auto source = T(100UZ);
        source.set();
        auto const target = std::move(source);
        auto const empty = source.size() == 0UZ; // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): the moved-from state is the check.
        source.push_back(true);                  // NOLINT(clang-analyzer-cplusplus.Move): growing the moved-from state is the check.
        return empty and target.all() and source.size() == 1UZ and source.test(0UZ);
}

} // namespace

// A moved-from owner at a run-time width is valid and empty, and grows again from there.
BOOST_AUTO_TEST_CASE(AMovedFromRunTimeWidthIsEmptyAndGrowsAgain)
{
        BOOST_CHECK(a_moved_from_sequence_grows_again<xstd::bit_vector>());
        BOOST_CHECK(a_moved_from_set_grows_again<xstd::bit_set>());
        BOOST_CHECK(a_moved_from_bitset_grows_again<xstd::dynamic_bitset>());
#ifdef __cpp_lib_inplace_vector

        BOOST_CHECK(a_moved_from_sequence_grows_again<xstd::bit_bounded_vector<N>>());
        BOOST_CHECK(a_moved_from_set_grows_again<xstd::bit_bounded_set<N>>());
        BOOST_CHECK(a_moved_from_bitset_grows_again<xstd::bounded_bitset<N>>());

#endif
}

namespace {

template<class T>
concept has_extract = requires (T&& t) { std::move(t).extract(); };

template<class T>
concept has_replace = requires (T& t, T::block_container_type&& blocks) { t.replace(std::move(blocks)); };

// Two blocks in, the same two blocks out, and the owner left empty behind them, whichever reading it is.
template<class T>
auto blocks_go_in_and_come_out_whole()
        -> bool
{
        auto const original = typename T::block_container_type{0b1011UZ, 1UZ << 63U};
        auto owner = T();
        auto blocks = original;
        owner.replace(std::move(blocks));
        auto const filled = not owner.empty();
        auto const out = std::move(owner).extract();
        auto const emptied = owner.empty(); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): extract leaves width zero, which is the check.
        return filled and out == original and emptied;
}

} // namespace

// flat_set's extract and replace, at every run-time width: the blocks are the representation, in and out.
BOOST_AUTO_TEST_CASE(ARunTimeWidthHandsItsBlocksOutAndTakesThemBack)
{
        BOOST_CHECK(blocks_go_in_and_come_out_whole<xstd::bit_vector>());
        BOOST_CHECK(blocks_go_in_and_come_out_whole<xstd::bit_set>());
        BOOST_CHECK(blocks_go_in_and_come_out_whole<xstd::dynamic_bitset>());
#ifdef __cpp_lib_inplace_vector

        BOOST_CHECK(blocks_go_in_and_come_out_whole<xstd::bit_bounded_vector<N>>());
        BOOST_CHECK(blocks_go_in_and_come_out_whole<xstd::bit_bounded_set<N>>());
        BOOST_CHECK(blocks_go_in_and_come_out_whole<xstd::bounded_bitset<N>>());

#endif

        // Every position of the blocks is one: the sequence and the bitset are two whole blocks wide.
        auto v = xstd::bit_vector();
        v.replace(std::vector<std::size_t>{1UZ, 0UZ});
        BOOST_CHECK_EQUAL(v.size(), 128UZ);
        BOOST_CHECK(v[0] and not v[1]);

        // A static width has nothing to hand over, and a view does not own what it would hand.
        static_assert(not has_extract<xstd::bit_array<N>> and not has_replace<xstd::bit_array<N>>);
        static_assert(not has_extract<xstd::bit_fixed_set<N>> and not has_replace<xstd::bit_fixed_set<N>>);
        static_assert(not has_extract<xstd::bitset<N>> and not has_replace<xstd::bitset<N>>);
        static_assert(not has_extract<decltype(xstd::bit_span(v))> and not has_replace<decltype(xstd::bit_span(v))>);
        static_assert(has_extract<xstd::bit_vector> and has_replace<xstd::bit_vector>);
}

BOOST_AUTO_TEST_CASE(TheBitsetSwapIsTheOneBoostHasAndStdDoesNot)
{
        auto a = xstd::bitset<N>();
        auto b = xstd::bitset<N>();
        a.set(1UZ);
        b.set(2UZ);

        a.swap(b);
        BOOST_CHECK(a.test(2UZ) and not a.test(1UZ));
        BOOST_CHECK(b.test(1UZ) and not b.test(2UZ));

        swap(a, b);
        BOOST_CHECK(a.test(1UZ) and not a.test(2UZ));
        BOOST_CHECK(b.test(2UZ) and not b.test(1UZ));
}

BOOST_AUTO_TEST_SUITE_END()
