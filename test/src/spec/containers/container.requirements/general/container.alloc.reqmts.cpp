//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/factory.hpp>                // make_sequence, stripes
#include <test/set/concepts.hpp>                    // set_size_t_allocator, set_size_t_ranges_allocator
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/dynamic_bitset.hpp>             // basic_dynamic_bitset, dynamic_bitset
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <xstd/bits/ext/boost/small_bitset.hpp>     // basic_small_bitset
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                    // array
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // less
#include <initializer_list>                         // initializer_list
#include <memory>                                   // allocator, allocator_traits, uses_allocator_v
#include <memory_resource>                          // memory_resource, monotonic_buffer_resource, polymorphic_allocator, unsynchronized_pool_resource
#include <ranges>                                   // from_range
#include <set>                                      // set
#include <tuple>                                    // tuple, tuple_cat
#include <type_traits>                              // bool_constant, false_type, is_constructible_v, is_nothrow_constructible_v, true_type
#include <utility>                                  // declval, move
#include <vector>                                   // pmr::vector, vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerAllocReqmts)

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

template<class Block, std::size_t N, class Allocator>
struct user_allocator<xstd::basic_small_bitset<Block, N, Allocator>>
{
        using type = Allocator;
};

template<class Block, std::size_t N, class Allocator>
struct user_allocator<xstd::basic_bit_small_vector<Block, N, Allocator>>
{
        using type = Allocator;
};

template<class X>
[[nodiscard]] auto allocator(int tag)
        -> X::allocator_type
{
        return typename X::allocator_type(make_allocator<typename user_allocator<X>::type>(tag));
}

// Declared only, for the concept below to call in an unevaluated operand.
template<class C>
[[maybe_unused]] auto accept(C)
        -> void;

// Copy-list-initialization from the braced list: what an explicit constructor refuses.
template<class C, class... Args>
concept list_converts_from = requires (Args... args) { accept<C>({args...}); };

// Keys inside the small set's inline blocks, and keys that spill it onto the heap.
[[nodiscard]] auto samples()
        -> std::array<std::vector<std::size_t>, 2>
{
        return {{{1, 2, 3}, {0, 100, 1000}}};
}

// The allocator-aware bitsets, whose model boost::dynamic_bitset has no allocator-extended copy or move.
template<template<class> class Allocator>
using bitset_allocator_aware = std::tuple<xstd::basic_dynamic_bitset<std::uint8_t, Allocator<std::uint8_t>>, xstd::basic_dynamic_bitset<std::uint64_t, Allocator<std::uint64_t>>, xstd::basic_small_bitset<std::uint8_t, 9, Allocator<std::uint8_t>>, xstd::basic_small_bitset<std::uint64_t, 64, Allocator<std::uint64_t>>>;

using BitsetTypes = decltype(std::tuple_cat(std::declval<bitset_allocator_aware<propagating>>(), std::declval<bitset_allocator_aware<non_propagating>>(), std::declval<bitset_allocator_aware<std::pmr::polymorphic_allocator>>()));

// A width inside the small bitset's inline blocks, and one that spills it onto the heap.
[[nodiscard]] auto widths()
        -> std::array<std::size_t, 2>
{
        return {9, 1000};
}

// std::vector<bool> is the model, over bool; the bit sequences take the allocator rebound to their Block.
template<template<class> class Allocator>
using sequence_allocator_aware = std::tuple<std::vector<bool, Allocator<bool>>, xstd::basic_bit_vector<std::uint8_t, Allocator<std::uint8_t>>, xstd::basic_bit_vector<std::uint64_t, Allocator<std::uint64_t>>, xstd::basic_bit_small_vector<std::uint8_t, 9, Allocator<std::uint8_t>>, xstd::basic_bit_small_vector<std::uint64_t, 64, Allocator<std::uint64_t>>>;

using SequenceTypes = decltype(std::tuple_cat(std::declval<sequence_allocator_aware<propagating>>(), std::declval<sequence_allocator_aware<non_propagating>>(), std::declval<sequence_allocator_aware<std::pmr::polymorphic_allocator>>()));

// A width inside the small sequence's inline blocks, and one that spills it onto the heap.
template<class X>
[[nodiscard]] auto sequence_samples(typename X::allocator_type const& a)
        -> std::array<X, 2>
{
        auto const narrow = test::sequence::make_sequence<std::vector<bool>>(9UZ, test::sequence::stripes);
        auto const wide = test::sequence::make_sequence<std::vector<bool>>(1000UZ, test::sequence::stripes);
        return {X(narrow.begin(), narrow.end(), a), X(wide.begin(), wide.end(), a)};
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

BOOST_AUTO_TEST_CASE_TEMPLATE(ABitsetsAllocatorExtendedConstructorUsesTheAllocatorGiven, T, BitsetTypes)
{
        static_assert(std::same_as<decltype(std::declval<T const&>().get_allocator()), typename T::allocator_type>);
        auto const m = allocator<T>(1);

        auto const u = T(m);
        BOOST_CHECK(u.empty());
        BOOST_CHECK(u.get_allocator() == m);

        for (auto const n : widths()) {
                auto const t = T(n, 5ULL, allocator<T>(0));

                auto const counted = T(n, 5ULL, m);
                BOOST_CHECK(counted == t);
                BOOST_CHECK(counted.get_allocator() == m);

                auto const copy = T(t, m);
                BOOST_CHECK(copy == t);
                BOOST_CHECK(copy.get_allocator() == m);

                auto rv = t;
                auto const moved = T(std::move(rv), m);
                BOOST_CHECK(moved == t);
                BOOST_CHECK(moved.get_allocator() == m);
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ABitsetsCopyAndMoveConstructionTakeTheAllocatorTheTraitsSay, T, BitsetTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        for (auto const n : widths()) {
                auto const t = T(n, 5ULL, allocator<T>(1));

                auto const copy = t; // NOLINT(performance-unnecessary-copy-initialization): the copy constructor is what is under test
                BOOST_CHECK(copy == t);
                BOOST_CHECK(copy.get_allocator() == traits::select_on_container_copy_construction(t.get_allocator()));

                auto rv = T(t, t.get_allocator());
                auto const moved = T(std::move(rv));
                BOOST_CHECK(moved == t);
                BOOST_CHECK(moved.get_allocator() == t.get_allocator());
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ABitsetsAssignmentPropagatesTheAllocatorWhereTheTraitsSaySo, T, BitsetTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = allocator<T>(1);
        for (auto const n : widths()) {
                auto const source = T(n, 5ULL, b);

                auto copied = T(3, 1ULL, a);
                copied = source;
                BOOST_CHECK(copied == source);
                BOOST_CHECK(copied.get_allocator() == (traits::propagate_on_container_copy_assignment::value ? b : a));

                auto rv = source;
                auto moved = T(3, 1ULL, a);
                moved = std::move(rv);
                BOOST_CHECK(moved == source);
                BOOST_CHECK(moved.get_allocator() == (traits::propagate_on_container_move_assignment::value ? b : a));
        }
}

// Swapping unequal allocators that do not propagate is undefined, so those are swapped under one allocator.
BOOST_AUTO_TEST_CASE_TEMPLATE(ABitsetsSwapExchangesTheAllocatorsWhereTheTraitsSaySo, T, BitsetTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = traits::propagate_on_container_swap::value ? allocator<T>(1) : a;
        for (auto const n : widths()) {
                auto x = T(n, 5ULL, a);
                auto y = T(3, 1ULL, b);
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

        using pmr_dynamic_bitset = xstd::basic_dynamic_bitset<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto const b = pmr_dynamic_bitset(&mr);
        BOOST_CHECK(b.get_allocator().resource() == &mr);
}

// Uses-allocator construction: an allocator-aware container hands each element its own allocator, converted.
BOOST_AUTO_TEST_CASE(AnAllocatorAwareContainerPassesItsAllocatorOn)
{
        using pmr_bit_set = xstd::basic_bit_set<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto mr = std::pmr::monotonic_buffer_resource();
        auto sets = std::pmr::vector<pmr_bit_set>(&mr);

        sets.emplace_back();
        BOOST_CHECK(sets.back().get_allocator().resource() == &mr);

        using pmr_dynamic_bitset = xstd::basic_dynamic_bitset<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto bitsets = std::pmr::vector<pmr_dynamic_bitset>(&mr);

        bitsets.emplace_back();
        BOOST_CHECK(bitsets.back().get_allocator().resource() == &mr);
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

// [container.alloc.reqmts]/6-7: c.get_allocator()
BOOST_AUTO_TEST_SUITE(GetAllocator)

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheAllocatorTypeForEverySequence, T, SequenceTypes)
{
        static_assert(std::same_as<decltype(std::declval<T const&>().get_allocator()), typename T::allocator_type>); // [container.alloc.reqmts]/6
        BOOST_CHECK(T(allocator<T>(1)).get_allocator() == allocator<T>(1));
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/8-10: X u
BOOST_AUTO_TEST_SUITE(DefaultConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(ValueInitializesTheAllocatorForEverySequence, T, SequenceTypes)
{
        auto const u = T();
        BOOST_CHECK(u.empty());                                         // [container.alloc.reqmts]/9
        BOOST_CHECK(u.get_allocator() == typename T::allocator_type()); // [container.alloc.reqmts]/9
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/11-12: X u(m)
BOOST_AUTO_TEST_SUITE(AllocatorConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(UsesTheAllocatorGivenForEverySequence, T, SequenceTypes)
{
        auto const m = allocator<T>(1);
        auto const u = T(m);
        BOOST_CHECK(u.empty());              // [container.alloc.reqmts]/11
        BOOST_CHECK(u.get_allocator() == m); // [container.alloc.reqmts]/11
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/13-15: X u(t, m)
BOOST_AUTO_TEST_SUITE(CopyWithAllocator)

BOOST_AUTO_TEST_CASE_TEMPLATE(CopiesUnderTheAllocatorGivenForEverySequence, T, SequenceTypes)
{
        auto const m = allocator<T>(1);
        for (auto const& t : sequence_samples<T>(allocator<T>(0))) {
                auto const u = T(t, m);
                BOOST_CHECK(u == t);                 // [container.alloc.reqmts]/14
                BOOST_CHECK(u.get_allocator() == m); // [container.alloc.reqmts]/14
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/16-17: X u(rv)
BOOST_AUTO_TEST_SUITE(MoveConstructor)

// The allocator moves with the elements, and a copy asks the traits which one to take.
BOOST_AUTO_TEST_CASE_TEMPLATE(TakesTheAllocatorAlongForEverySequence, T, SequenceTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        for (auto const& t : sequence_samples<T>(allocator<T>(1))) {
                auto rv = T(t, t.get_allocator());
                auto const u = T(std::move(rv));
                BOOST_CHECK(u == t);                                 // [container.alloc.reqmts]/16
                BOOST_CHECK(u.get_allocator() == t.get_allocator()); // [container.alloc.reqmts]/16

                auto const copy = t; // NOLINT(performance-unnecessary-copy-initialization): the copy constructor is what is under test
                BOOST_CHECK(copy == t);
                BOOST_CHECK(copy.get_allocator() == traits::select_on_container_copy_construction(t.get_allocator())); // [container.reqmts]/64
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/18-20: X u(rv, m)
BOOST_AUTO_TEST_SUITE(MoveWithAllocator)

BOOST_AUTO_TEST_CASE_TEMPLATE(MovesUnderTheAllocatorGivenForEverySequence, T, SequenceTypes)
{
        auto const m = allocator<T>(1);
        for (auto const& t : sequence_samples<T>(allocator<T>(0))) {
                auto rv = t;
                auto const u = T(std::move(rv), m);
                BOOST_CHECK(u == t);                 // [container.alloc.reqmts]/19
                BOOST_CHECK(u.get_allocator() == m); // [container.alloc.reqmts]/19
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/641-24: a = t
BOOST_AUTO_TEST_SUITE(CopyAssignment)

BOOST_AUTO_TEST_CASE_TEMPLATE(PropagatesTheAllocatorWhereTheTraitsSaySoForEverySequence, T, SequenceTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = allocator<T>(1);
        for (auto const& t : sequence_samples<T>(b)) {
                auto u = T({true}, a);
                u = t;
                BOOST_CHECK(u == t); // [container.reqmts]/643
                BOOST_CHECK(u.get_allocator() == (traits::propagate_on_container_copy_assignment::value ? b : a));
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/645-29: a = rv
BOOST_AUTO_TEST_SUITE(MoveAssignment)

BOOST_AUTO_TEST_CASE_TEMPLATE(PropagatesTheAllocatorWhereTheTraitsSaySoForEverySequence, T, SequenceTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = allocator<T>(1);
        for (auto const& t : sequence_samples<T>(b)) {
                auto rv = t;
                auto u = T({true}, a);
                u = std::move(rv);
                BOOST_CHECK(u == t); // [container.reqmts]/648
                BOOST_CHECK(u.get_allocator() == (traits::propagate_on_container_move_assignment::value ? b : a));
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/30-32: a.swap(b)
BOOST_AUTO_TEST_SUITE(Swap)

// Swapping unequal allocators that do not propagate is undefined, so those are swapped under one allocator.
BOOST_AUTO_TEST_CASE_TEMPLATE(ExchangesTheAllocatorsWhereTheTraitsSaySoForEverySequence, T, SequenceTypes)
{
        using traits = std::allocator_traits<typename T::allocator_type>;
        auto const a = allocator<T>(0);
        auto const b = traits::propagate_on_container_swap::value ? allocator<T>(1) : a;
        for (auto const& t : sequence_samples<T>(a)) {
                auto x = T(t, a);
                auto y = T({true}, b);
                auto const x1 = x;
                auto const y1 = y;
                x.swap(y);
                BOOST_CHECK(x == y1 and y == x1); // [container.alloc.reqmts]/31
                BOOST_CHECK(x.get_allocator() == b and y.get_allocator() == a);
                swap(x, y);
                BOOST_CHECK(x == x1 and y == y1);
                BOOST_CHECK(x.get_allocator() == a and y.get_allocator() == b);
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [container.alloc.reqmts]/3: the allocator arguments each container takes
BOOST_AUTO_TEST_SUITE(AllocatorArguments)

// A memory_resource* converts to the polymorphic allocator, as for std::vector<bool>.
BOOST_AUTO_TEST_CASE(AMemoryResourceConvertsToThePolymorphicAllocatorOfASequence)
{
        using pmr_bit_vector = xstd::basic_bit_vector<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto mr = std::pmr::monotonic_buffer_resource();

        auto const v = pmr_bit_vector(3, true, &mr);
        BOOST_CHECK(v.get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
}

// Uses-allocator construction: an allocator-aware container hands each element its own allocator, converted.
BOOST_AUTO_TEST_CASE(AnAllocatorAwareContainerPassesItsAllocatorOnToASequence)
{
        using pmr_bit_vector = xstd::basic_bit_vector<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        static_assert(std::uses_allocator_v<pmr_bit_vector, std::pmr::polymorphic_allocator<pmr_bit_vector>>);

        auto mr = std::pmr::monotonic_buffer_resource();
        auto vectors = std::pmr::vector<pmr_bit_vector>(&mr);
        vectors.emplace_back(3);

        BOOST_CHECK(vectors.back().get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(vectors.back().size(), 3UZ);
}

// A rebound std::allocator converts, and a braced {} is a value-initialized allocator, as for std::vector<bool>.
BOOST_AUTO_TEST_CASE(AConvertibleOrEmptyAllocatorArgumentIsTakenByASequence)
{
        static_assert(std::is_constructible_v<std::vector<bool>, std::size_t, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::bit_vector, std::size_t, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::dynamic_bitset, std::allocator<int>>);

        auto const v = xstd::bit_vector(3, true, {});
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
}

// std::vector<bool>'s and boost's count constructors are explicit with the allocator as without it.
BOOST_AUTO_TEST_CASE(TheCountConstructorsAreExplicit)
{
        static_assert(not list_converts_from<std::vector<bool>, std::size_t, std::allocator<bool>>);
        static_assert(not list_converts_from<xstd::bit_vector, std::size_t, std::allocator<std::size_t>>);
        static_assert(not list_converts_from<xstd::dynamic_bitset, std::size_t, unsigned long long, std::allocator<std::size_t>>);
        BOOST_CHECK(true);
}

// The allocator-only constructors cannot throw, as [vector.bool.pspc] has it; a width in the type takes no allocator.
BOOST_AUTO_TEST_CASE(OnlyARunTimeWidthTakesAnAllocator)
{
        static_assert(std::is_nothrow_constructible_v<std::vector<bool>, std::allocator<bool> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_vector, std::allocator<std::size_t> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::dynamic_bitset, std::allocator<std::size_t> const&>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, std::allocator<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, std::initializer_list<bool>, std::allocator<std::size_t>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
