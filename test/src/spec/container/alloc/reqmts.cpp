//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/concepts.hpp>                 // set_size_t_allocator, set_size_t_ranges_allocator
#include <xstd/bits/bit_set.hpp>                 // basic_bit_set, bit_set
#include <xstd/bits/ext/boost/bit_small_set.hpp> // basic_bit_small_set
#include <boost/test/unit_test.hpp>              // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                 // array
#include <concepts>                              // same_as
#include <cstddef>                               // size_t
#include <cstdint>                               // uint64_t, uint8_t
#include <functional>                            // less
#include <memory>                                // allocator, allocator_traits
#include <memory_resource>                       // memory_resource, monotonic_buffer_resource, polymorphic_allocator, unsynchronized_pool_resource
#include <ranges>                                // from_range
#include <set>                                   // set
#include <tuple>                                 // tuple, tuple_cat
#include <type_traits>                           // bool_constant, false_type, is_constructible_v, true_type
#include <utility>                               // declval, move
#include <vector>                                // pmr::vector, vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Container)
BOOST_AUTO_TEST_SUITE(Alloc)
BOOST_AUTO_TEST_SUITE(Reqmts)

namespace {

// Stateful, so two instances can differ, and propagating on every assignment and swap or on none of them.
template<class T, bool Propagates>
class tagged_allocator
{
        int m_tag = 0;

public:
        using value_type = T;
        using propagate_on_container_copy_assignment = std::bool_constant<Propagates>;
        using propagate_on_container_move_assignment = std::bool_constant<Propagates>;
        using propagate_on_container_swap = std::bool_constant<Propagates>;
        using is_always_equal = std::false_type;

        template<class U>
        struct rebind
        {
                using other = tagged_allocator<U, Propagates>;
        };

        [[nodiscard]] tagged_allocator() = default;

        [[nodiscard]] constexpr explicit tagged_allocator(int tag) noexcept
                : m_tag(tag)
        {}

        template<class U>
        [[nodiscard]] constexpr explicit(false) tagged_allocator(tagged_allocator<U, Propagates> const& other) noexcept
                : m_tag(other.tag())
        {}

        [[nodiscard]] constexpr auto tag() const noexcept
                -> int
        {
                return m_tag;
        }

        [[nodiscard]] auto allocate(std::size_t n)
                -> T*
        {
                return std::allocator<T>().allocate(n);
        }

        auto deallocate(T* p, std::size_t n) noexcept
                -> void
        {
                std::allocator<T>().deallocate(p, n);
        }

        template<class U>
        [[nodiscard]] friend constexpr auto operator==(tagged_allocator const& lhs, tagged_allocator<U, Propagates> const& rhs) noexcept
                -> bool
        {
                return lhs.tag() == rhs.tag();
        }
};

template<class T>
using propagating = tagged_allocator<T, true>;

template<class T>
using non_propagating = tagged_allocator<T, false>;

// Two resources to tell polymorphic allocators apart by, never destroyed, so no exit-time destructor frees under a set.
[[nodiscard]] auto resource(int tag)
        -> std::pmr::memory_resource*
{
        static auto& pools = *new std::array<std::pmr::unsynchronized_pool_resource, 2>();
        return &pools.at(static_cast<std::size_t>(tag % 2));
}

// The allocator a user writes, from which each column's allocator_type is built.
template<class Allocator>
[[nodiscard]] auto make_allocator(int tag)
        -> Allocator
{
        if constexpr (std::is_constructible_v<Allocator, std::pmr::memory_resource*>) {
                return Allocator(resource(tag));
        } else {
                return Allocator(tag);
        }
}

// std::set is the model, over the allocator's own value_type; the bit sets take it rebound to their Block.
template<template<class> class Allocator>
using allocator_aware = std::tuple<std::set<std::size_t, std::less<std::size_t>, Allocator<std::size_t>>, xstd::basic_bit_set<std::uint8_t, Allocator<std::uint8_t>>, xstd::basic_bit_set<std::uint64_t, Allocator<std::uint64_t>>, xstd::basic_bit_small_set<std::uint8_t, 9, Allocator<std::uint8_t>>, xstd::basic_bit_small_set<std::uint64_t, 64, Allocator<std::uint64_t>>>;

using Types = decltype(std::tuple_cat(std::declval<allocator_aware<propagating>>(), std::declval<allocator_aware<non_propagating>>(), std::declval<allocator_aware<std::pmr::polymorphic_allocator>>()));

// The allocator the column was declared with, which the small set wraps in one of Boost's own.
template<class X>
struct user_allocator
{
        using type = X::allocator_type;
};

template<class Block, std::size_t N, class Allocator>
struct user_allocator<xstd::basic_bit_small_set<Block, N, Allocator>>
{
        using type = Allocator;
};

template<class X>
[[nodiscard]] auto allocator(int tag)
        -> X::allocator_type
{
        return typename X::allocator_type(make_allocator<typename user_allocator<X>::type>(tag));
}

// Keys inside the small set's inline blocks, and keys that spill it onto the heap.
[[nodiscard]] auto samples()
        -> std::array<std::vector<std::size_t>, 2>
{
        return {{{1, 2, 3}, {0, 100, 1000}}};
}

} // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(AnswersEveryAllocatorLineOfTheSynopsis, T, Types)
{
        static_assert(test::set::set_size_t_allocator<T>);
#ifdef __cpp_lib_containers_ranges

        static_assert(test::set::set_size_t_ranges_allocator<T>);

#endif
        static_assert(std::same_as<decltype(std::declval<T const&>().get_allocator()), typename T::allocator_type>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AnAllocatorExtendedConstructorUsesTheAllocatorGiven, T, Types)
{
        auto const m = allocator<T>(1);

        auto const u = T(m);
        BOOST_CHECK(u.empty());
        BOOST_CHECK(u.get_allocator() == m);

        auto const listed = T({3, 1}, m);
        BOOST_CHECK(listed == T({1, 3}));
        BOOST_CHECK(listed.get_allocator() == m);

        for (auto const& keys : samples()) {
                auto const t = T(keys.begin(), keys.end(), allocator<T>(0));

                auto const copy = T(t, m);
                BOOST_CHECK(copy == t);
                BOOST_CHECK(copy.get_allocator() == m);

                auto rv = t;
                auto const moved = T(std::move(rv), m);
                BOOST_CHECK(moved == t);
                BOOST_CHECK(moved.get_allocator() == m);

                auto const ranged = T(std::from_range, keys, m);
                BOOST_CHECK(ranged == t);
                BOOST_CHECK(ranged.get_allocator() == m);
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CopyAndMoveConstructionTakeTheAllocatorTheTraitsSay, T, Types)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        for (auto const& keys : samples()) {
                auto const t = T(keys.begin(), keys.end(), allocator<T>(1));

                auto const copy = t; // NOLINT(performance-unnecessary-copy-initialization): the copy constructor is what is under test
                BOOST_CHECK(copy == t);
                BOOST_CHECK(copy.get_allocator() == traits::select_on_container_copy_construction(t.get_allocator()));

                auto rv = T(t, t.get_allocator());
                auto const moved = T(std::move(rv));
                BOOST_CHECK(moved == t);
                BOOST_CHECK(moved.get_allocator() == t.get_allocator());
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AssignmentPropagatesTheAllocatorWhereTheTraitsSaySo, T, Types)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = allocator<T>(1);
        for (auto const& keys : samples()) {
                auto const source = T(keys.begin(), keys.end(), b);

                auto copied = T({5}, a);
                copied = source;
                BOOST_CHECK(copied == source);
                BOOST_CHECK(copied.get_allocator() == (traits::propagate_on_container_copy_assignment::value ? b : a));

                auto rv = source;
                auto moved = T({5}, a);
                moved = std::move(rv);
                BOOST_CHECK(moved == source);
                BOOST_CHECK(moved.get_allocator() == (traits::propagate_on_container_move_assignment::value ? b : a));
        }
}

// Swapping unequal allocators that do not propagate is undefined, so those are swapped under one allocator.
BOOST_AUTO_TEST_CASE_TEMPLATE(SwapExchangesTheAllocatorsWhereTheTraitsSaySo, T, Types)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = traits::propagate_on_container_swap::value ? allocator<T>(1) : a;
        for (auto const& keys : samples()) {
                auto x = T(keys.begin(), keys.end(), a);
                auto y = T({5}, b);
                auto const x1 = x;
                auto const y1 = y;
                x.swap(y);
                BOOST_CHECK(x == y1 and y == x1);
                BOOST_CHECK(x.get_allocator() == b and y.get_allocator() == a);
                swap(x, y);
                BOOST_CHECK(x == x1 and y == y1);
                BOOST_CHECK(x.get_allocator() == a and y.get_allocator() == b);
        }
}

// A memory_resource* converts to the polymorphic allocator, as [container.alloc.reqmts] has it.
BOOST_AUTO_TEST_CASE(AMemoryResourceConvertsToThePolymorphicAllocator)
{
        using pmr_bit_set = xstd::basic_bit_set<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto mr = std::pmr::monotonic_buffer_resource();

        auto const s = pmr_bit_set({1, 2}, &mr);
        BOOST_CHECK(s.get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(s.size(), 2UZ);
}

// Uses-allocator construction: an allocator-aware container hands each element its own allocator, converted.
BOOST_AUTO_TEST_CASE(AnAllocatorAwareContainerPassesItsAllocatorOn)
{
        using pmr_bit_set = xstd::basic_bit_set<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto mr = std::pmr::monotonic_buffer_resource();
        auto sets = std::pmr::vector<pmr_bit_set>(&mr);

        sets.emplace_back();
        BOOST_CHECK(sets.back().get_allocator().resource() == &mr);
}

// A rebound std::allocator converts, as it does for std::set.
BOOST_AUTO_TEST_CASE(AConvertibleAllocatorArgumentIsTaken)
{
        static_assert(std::is_constructible_v<std::set<std::size_t>, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::bit_set, std::allocator<int>>);
        BOOST_CHECK(true);
}

// std::set's comparator arguments are accepted alongside the allocator, and std::less having no state, change nothing.
BOOST_AUTO_TEST_CASE(TheComparatorArgumentsAreAcceptedAlongsideTheAllocator)
{
        auto const comp = xstd::bit_set::key_compare(); // NOLINT(modernize-use-transparent-functors): std::set<std::size_t>::key_compare
        BOOST_CHECK(xstd::bit_set({3, 1}, comp, std::allocator<std::size_t>()) == xstd::bit_set({1, 3}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
