//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits.hpp>            // bit_array, bit_set, bit_static_set, bit_vector, bitset, dynamic_bitset, and the inplace column
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <compare>                  // three_way_comparable
#include <concepts>                 // copyable, default_initializable, movable, swappable, totally_ordered
#include <cstddef>                  // size_t
#include <memory_resource>          // polymorphic_allocator
#include <ranges>                   // swap
#include <scoped_allocator>         // scoped_allocator_adaptor
#include <type_traits>              // is_nothrow_move_assignable_v, is_nothrow_move_constructible_v
#include <utility>                  // swap

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
        static_assert(is_regular_container<xstd::bit_static_set<N>>());
        static_assert(is_regular_container<xstd::bit_set>());
        static_assert(is_regular_container<xstd::bit_array<N>>());
        static_assert(is_regular_container<xstd::bit_vector>());
        static_assert(is_regular_container<xstd::bitset<N>>());
        static_assert(is_regular_container<xstd::dynamic_bitset>());
#ifdef __cpp_lib_inplace_vector

        static_assert(is_regular_container<xstd::bit_inplace_set<N>>());
        static_assert(is_regular_container<xstd::bit_inplace_vector<N>>());
        static_assert(is_regular_container<xstd::inplace_bitset<N>>());

#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheAllocatorFollowsTheColumnAndNotTheRow)
{
        // The dynamic column allocates, so all three rows of it answer.
        static_assert(allocator_aware<xstd::bit_set>());
        static_assert(allocator_aware<xstd::bit_vector>());
        static_assert(allocator_aware<xstd::dynamic_bitset>());

        // The static column is a std::array, which has no allocator for any row to show.
        static_assert(not_allocator_aware<xstd::bit_static_set<N>>());
        static_assert(not_allocator_aware<xstd::bit_array<N>>());
        static_assert(not_allocator_aware<xstd::bitset<N>>());

        // The inplace column holds its blocks inline, so it has none either.
#ifdef __cpp_lib_inplace_vector

        static_assert(not_allocator_aware<xstd::bit_inplace_set<N>>());
        static_assert(not_allocator_aware<xstd::bit_inplace_vector<N>>());
        static_assert(not_allocator_aware<xstd::inplace_bitset<N>>());

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
        auto a = xstd::bit_static_set<N>();
        auto b = xstd::bit_static_set<N>();
        a.insert(1UZ);
        b.insert(2UZ);

        a.swap(b);
        BOOST_CHECK(a.contains(2UZ) and not a.contains(1UZ));
        BOOST_CHECK(b.contains(1UZ) and not b.contains(2UZ));

        swap(a, b);
        BOOST_CHECK(a.contains(1UZ) and not a.contains(2UZ));
        BOOST_CHECK(b.contains(2UZ) and not b.contains(1UZ));
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
