//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/allocator.hpp>             // basic_guarantee, keeps_a_ledger, ledger, ledger_allocator, non_propagating, propagating, strong_guarantee, user_allocator
#include <test/flat_set.hpp>                        // is_flat_set
#include <test/for_each_type.hpp>                   // for_each_type
#include <test/sequence/factory.hpp>                // make_sequence, stripes
#include <test/spec/rejection.hpp>                  // has_allocator_constructor, has_allocator_extended_copy, has_allocator_extended_move, has_allocator_type, has_get_allocator
#include <test/spec/sequence.hpp>                   // all
#include <test/spec/set.hpp>                        // all, const_views
#include <test/spec/span.hpp>                       // all
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_key_traits.hpp>             // bit_key_traits
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                    // array
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // less
#include <initializer_list>                         // initializer_list
#include <iterator>                                 // prev
#include <memory>                                   // allocator, allocator_traits, uses_allocator_v
#include <memory_resource>                          // memory_resource, monotonic_buffer_resource, polymorphic_allocator, unsynchronized_pool_resource
#include <new>                                      // bad_alloc
#include <ranges>                                   // from_range, iota
#include <scoped_allocator>                         // scoped_allocator_adaptor
#include <set>                                      // pmr::set, set
#include <tuple>                                    // tuple, tuple_cat
#include <type_traits>                              // is_constructible_v, is_nothrow_constructible_v
#include <utility>                                  // declval, move
#include <vector>                                   // pmr::vector, vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerAllocReqmts)

namespace {

using test::container::basic_guarantee;
using test::container::keeps_a_ledger;
using test::container::ledger;
using test::container::ledger_allocator;
using test::container::non_propagating;
using test::container::propagating;
using test::container::strong_guarantee;
using test::container::user_allocator;

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
using allocator_aware = std::tuple<std::set<std::size_t, std::less<std::size_t>, Allocator<std::size_t>>, xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, Allocator<std::uint8_t>>, xstd::basic_bit_set<std::size_t, std::uint64_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, Allocator<std::uint64_t>>, xstd::basic_bit_small_set<std::size_t, std::uint8_t, 9, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, Allocator<std::uint8_t>>, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 64, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, Allocator<std::uint64_t>>>;

using Types = decltype(std::tuple_cat(std::declval<allocator_aware<propagating>>(), std::declval<allocator_aware<non_propagating>>(), std::declval<allocator_aware<std::pmr::polymorphic_allocator>>()));

// The small columns, whose allocator_type is Boost's wrapper around the one they were declared with.
template<class X>
inline constexpr auto wraps_allocator = false;

template<class Block, std::size_t N, class Compare, class Allocator>
inline constexpr auto wraps_allocator<xstd::basic_bit_small_set<std::size_t, Block, N, xstd::bit_key_traits<std::size_t>, Compare, Allocator>> = true;

template<class Block, std::size_t N, class Allocator>
inline constexpr auto wraps_allocator<xstd::basic_bit_small_vector<Block, N, Allocator>> = true;

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

// std::vector<bool> is the model, over bool; the bit sequences take the allocator rebound to their Block.
template<template<class> class Allocator>
using sequence_allocator_aware = std::tuple<std::vector<bool, Allocator<bool>>, xstd::basic_bit_vector<std::uint8_t, Allocator<std::uint8_t>>, xstd::basic_bit_vector<std::uint64_t, Allocator<std::uint64_t>>, xstd::basic_bit_small_vector<std::uint8_t, 9, Allocator<std::uint8_t>>, xstd::basic_bit_small_vector<std::uint64_t, 64, Allocator<std::uint64_t>>>;

using SequenceTypes = decltype(std::tuple_cat(std::declval<sequence_allocator_aware<propagating>>(), std::declval<sequence_allocator_aware<non_propagating>>(), std::declval<sequence_allocator_aware<std::pmr::polymorphic_allocator>>()));

using all = decltype(std::tuple_cat(std::declval<Types>(), std::declval<SequenceTypes>()));

// The allocator members, present together where the type names an allocator and absent together where it names none.
template<class X>
auto check_allocator_members()
        -> void
{
        // std::flat_set names none and hands an allocator argument to its key container.
        if constexpr (not test::is_flat_set<X>) {
                static_assert(test::spec::has_get_allocator<X> == test::spec::has_allocator_type<X>);
                static_assert(test::spec::has_allocator_constructor<X> == test::spec::has_allocator_type<X>);
                if constexpr (not test::spec::has_allocator_type<X>) {
                        static_assert(not test::spec::has_allocator_extended_copy<X>);
                        static_assert(not test::spec::has_allocator_extended_move<X>);
                }
        }
}

// One more element than the value holds: a key past its largest, or a bool at the end.
template<class X>
auto grow_by_one(X& x)
        -> void
{
        if constexpr (requires { typename X::key_type; }) {
                x.insert(x.empty() ? 0UZ : *std::prev(x.end()) + 1UZ);
        } else {
                x.push_back(true);
        }
}

// One more element before the first: a key past the largest for a set, which has no position to choose.
template<class X>
auto insert_one(X& x)
        -> void
{
        if constexpr (requires { typename X::key_type; }) {
                x.insert(x.empty() ? 0UZ : *std::prev(x.end()) + 1UZ);
        } else {
                x.insert(x.cbegin(), false);
        }
}

// Grown up to the storage it holds without allocating, then one past it, a failed insertion changes nothing.
template<class X>
auto check_failed_insertion()
        -> void
{
        auto book   = ledger();
        auto spare  = ledger();
        auto x      = X(ledger_allocator<X>(book));
        book.budget = 0;
        for ([[maybe_unused]] auto const i : std::views::iota(0UZ, 2048UZ)) {
                // A copy under book would itself allocate where MSVC's debug containers keep a proxy.
                auto const before = X(x, ledger_allocator<X>(spare));
                try {
                        grow_by_one(x);
                } catch (std::bad_alloc const&) {
                        BOOST_CHECK(x == before); // [container.reqmts]/66
                        try {
                                insert_one(x);
                        } catch (std::bad_alloc const&) {
                                BOOST_CHECK(x == before); // [container.reqmts]/66
                        }
                        return;
                }
        }
}

// Two values under allocator a: one inside the small columns' inline blocks, and one that spills them onto the heap.
template<class X>
[[nodiscard]] auto samples(typename X::allocator_type const& a)
        -> std::array<X, 2>
{
        if constexpr (requires { typename X::key_type; }) {
                auto const narrow = std::vector<std::size_t>{1, 2, 3};
                auto const wide   = std::vector<std::size_t>{0, 100, 1000};
                return {X(narrow.begin(), narrow.end(), a), X(wide.begin(), wide.end(), a)};
        } else {
                auto const narrow = test::sequence::make_sequence<std::vector<bool>>(9UZ, test::sequence::stripes);
                auto const wide   = test::sequence::make_sequence<std::vector<bool>>(1000UZ, test::sequence::stripes);
                return {X(narrow.begin(), narrow.end(), a), X(wide.begin(), wide.end(), a)};
        }
}

// A value unlike either sample, to assign over and swap with.
template<class X>
[[nodiscard]] auto other(typename X::allocator_type const& a)
        -> X
{
        if constexpr (requires { typename X::key_type; }) {
                return X({5}, a);
        } else {
                return X({true}, a);
        }
}

// A set's allocator-extended list and range constructors.
template<class X>
auto check_listed_and_ranged(typename X::allocator_type const& m)
        -> void
{
        auto const listed = X({3, 1}, m);
        BOOST_CHECK(listed == X({1, 3}));
        BOOST_CHECK(listed.get_allocator() == m);
        auto const keys   = std::vector<std::size_t>{0, 100, 1000};
        auto const ranged = X(std::from_range, keys, m);
        BOOST_CHECK(ranged == X(keys.begin(), keys.end()));
        BOOST_CHECK(ranged.get_allocator() == m);
}

// A sequence's allocator-extended count constructor.
template<class X>
auto check_counted(typename X::allocator_type const& m, auto value)
        -> void
{
        auto const counted = X(1000UZ, value, m);
        BOOST_CHECK(counted == X(1000UZ, value));
        BOOST_CHECK(counted.get_allocator() == m);
}

} // namespace

// [container.alloc.reqmts]/1,4: typename X::allocator_type
BOOST_AUTO_TEST_CASE(AllocatorType)
{
        test::for_each_type<all>([]<class T> -> void {
                // The allocator the column was declared with, or for the small columns Boost's wrapper around it.
                using A              = user_allocator<T>::type;
                using allocator_type = T::allocator_type;
                static_assert(std::same_as<allocator_type, A> or wraps_allocator<T>);                    // [container.alloc.reqmts]/4
                BOOST_CHECK(T(allocator<T>(1)).get_allocator() == allocator_type(make_allocator<A>(1))); // [container.alloc.reqmts]/1
        });
}

// [container.alloc.reqmts]/6: c.get_allocator()
BOOST_AUTO_TEST_CASE(GetAllocator)
{
        test::for_each_type<all>([]<class T> -> void {
                static_assert(std::same_as<decltype(std::declval<T const&>().get_allocator()), typename T::allocator_type>); // [container.alloc.reqmts]/6
                BOOST_CHECK(T(allocator<T>(1)).get_allocator() == allocator<T>(1));                                          // [container.alloc.reqmts]/6
        });
}

// [container.alloc.reqmts]/9: X u; X u = X();
BOOST_AUTO_TEST_CASE(DefaultConstructor)
{
        test::for_each_type<all>([]<class T> -> void {
                T u;
                auto const u1 = T();
                BOOST_CHECK(u.empty() and u1.empty());                                                                                 // [container.alloc.reqmts]/9
                BOOST_CHECK(u.get_allocator() == typename T::allocator_type() and u1.get_allocator() == typename T::allocator_type()); // [container.alloc.reqmts]/9
        });
}

// [container.alloc.reqmts]/11: X u(m);
BOOST_AUTO_TEST_CASE(AllocatorConstructor)
{
        test::for_each_type<all>([]<class T> -> void {
                static_assert(requires (T::allocator_type a) { T(a); });
                auto const m = allocator<T>(1);
                auto const u = T(m);
                BOOST_CHECK(u.empty());              // [container.alloc.reqmts]/11
                BOOST_CHECK(u.get_allocator() == m); // [container.alloc.reqmts]/11
        });
}

// [container.alloc.reqmts]/14: X u(t, m);
BOOST_AUTO_TEST_CASE(CopyWithAllocator)
{
        test::for_each_type<all>([]<class T> -> void {
                static_assert(requires (T const cc, T::allocator_type a) { T(cc, a); });
                auto const m = allocator<T>(1);
                for (auto const& t : samples<T>(allocator<T>(0))) {
                        auto const u = T(t, m);
                        BOOST_CHECK(u == t);                                                                               // [container.alloc.reqmts]/14
                        BOOST_CHECK(u.get_allocator() == m);                                                               // [container.alloc.reqmts]/14
                        BOOST_CHECK(basic_guarantee(t, [](T& x) -> void { static_cast<void>(T(x, x.get_allocator())); })); // [container.reqmts]/25
                }
        });
}

// [container.alloc.reqmts]/16: X u(rv);
BOOST_AUTO_TEST_CASE(MoveConstructor)
{
        test::for_each_type<all>([]<class T> -> void {
                // The allocator moves with the elements, and a copy asks the traits which one to take.
                using traits = std::allocator_traits<typename T::allocator_type>;
                for (auto const& t : samples<T>(allocator<T>(1))) {
                        auto rv      = T(t, t.get_allocator());
                        auto const u = T(std::move(rv));
                        BOOST_CHECK(u == t);                                 // [container.alloc.reqmts]/16
                        BOOST_CHECK(u.get_allocator() == t.get_allocator()); // [container.alloc.reqmts]/16

                        auto const copy = t; // NOLINT(performance-unnecessary-copy-initialization): the copy constructor is what is under test
                        BOOST_CHECK(copy == t);
                        BOOST_CHECK(copy.get_allocator() == traits::select_on_container_copy_construction(t.get_allocator())); // [container.reqmts]/64
                }
        });
}

// [container.alloc.reqmts]/19: X u(rv, m);
BOOST_AUTO_TEST_CASE(MoveWithAllocator)
{
        test::for_each_type<all>([]<class T> -> void {
                static_assert(requires (T o, T::allocator_type a) { T(std::move(o), a); });
                auto const m = allocator<T>(1);
                for (auto const& t : samples<T>(allocator<T>(0))) {
                        auto rv      = t;
                        auto const u = T(std::move(rv), m);
                        BOOST_CHECK(u == t);                 // [container.alloc.reqmts]/19
                        BOOST_CHECK(u.get_allocator() == m); // [container.alloc.reqmts]/19
                }
        });
}

namespace {

template<class X>
inline constexpr auto is_std_set_v = false;

template<class Key, class Compare, class Allocator>
inline constexpr auto is_std_set_v<std::set<Key, Compare, Allocator>> = true;

// The standard's models, held to the basic guarantee it gives; ours allocate before they change anything.
template<class X>
inline constexpr auto is_std_model_v = is_std_set_v<X>;

template<class Allocator>
inline constexpr auto is_std_model_v<std::vector<bool, Allocator>> = true;

#ifdef _LIBCPP_VERSION

// libc++'s std::set reuses its nodes to assign into, and a refusal part way leaves it corrupt.
inline constexpr auto std_set_assigning_in_is_valid_after_a_refusal = false;

#else

inline constexpr auto std_set_assigning_in_is_valid_after_a_refusal = true;

#endif

// An lvalue on x's ledger under an allocator tagged alike or not; a named template spares MSVC a C1001.
template<class X>
auto check_copying_in(X const& t, typename X::allocator_type const& a, int tag)
        -> void
{
        auto const copy_in = [&](X& x) -> void {
                auto const source = X(t, ledger_allocator<X>(*x.get_allocator().book(), tag));
                x                 = source;
        };
        if constexpr (is_std_model_v<X>) {
                BOOST_CHECK(basic_guarantee(other<X>(a), copy_in));
        } else {
                BOOST_CHECK(strong_guarantee(other<X>(a), copy_in));
        }
}

} // namespace

// [container.alloc.reqmts]/21,23: a = t
BOOST_AUTO_TEST_CASE(CopyAssignment)
{
        test::for_each_type<all>([]<class T> -> void {
                using traits = std::allocator_traits<typename T::allocator_type>;
                static_assert(requires (T c, T const cc) { { c = cc } -> std::same_as<T&>; }); // [container.alloc.reqmts]/21
                auto const a = allocator<T>(0);
                auto const b = allocator<T>(1);
                for (auto const& t : samples<T>(b)) {
                        auto u = other<T>(a);
                        u      = t;
                        BOOST_CHECK(u == t);                                                                               // [container.alloc.reqmts]/23
                        BOOST_CHECK(u.get_allocator() == (traits::propagate_on_container_copy_assignment::value ? b : a)); // [container.reqmts]/64
                        BOOST_CHECK(basic_guarantee(other<T>(a), [&](T& x) -> void { x = T(t, x.get_allocator()); }));     // [container.reqmts]/25
                        if constexpr (keeps_a_ledger<T> and (std_set_assigning_in_is_valid_after_a_refusal or not is_std_set_v<T>)) {
                                check_copying_in(t, a, 0); // [container.reqmts]/25
                                check_copying_in(t, a, 1); // [container.reqmts]/25
                        }
                }
        });
}

namespace {

// Unequal under one ledger, so a staying allocator moves elements one by one; a named template spares MSVC a C1001.
template<class X>
auto check_moving_in_element_by_element(X const& t, typename X::allocator_type const& a)
        -> void
{
        auto const move_in = [&](X& x) -> void { x = X(t, ledger_allocator<X>(*x.get_allocator().book(), 1)); };
        if constexpr (is_std_model_v<X>) {
                BOOST_CHECK(basic_guarantee(other<X>(a), move_in));
        } else {
                BOOST_CHECK(strong_guarantee(other<X>(a), move_in));
        }
}

} // namespace

// [container.alloc.reqmts]/25,27-28: a = rv
BOOST_AUTO_TEST_CASE(MoveAssignment)
{
        test::for_each_type<all>([]<class T> -> void {
                using traits = std::allocator_traits<typename T::allocator_type>;
                static_assert(requires (T c, T o) { { c = std::move(o) } -> std::same_as<T&>; }); // [container.alloc.reqmts]/25
                auto const a = allocator<T>(0);
                auto const b = allocator<T>(1);
                for (auto const& t : samples<T>(b)) {
                        auto rv = t;
                        auto u  = other<T>(a);
                        u       = std::move(rv);
                        BOOST_CHECK_EQUAL(u.size(), t.size());                                                             // [container.alloc.reqmts]/27
                        BOOST_CHECK(u == t);                                                                               // [container.alloc.reqmts]/28
                        BOOST_CHECK(u.get_allocator() == (traits::propagate_on_container_move_assignment::value ? b : a)); // [container.reqmts]/64
                        if constexpr (keeps_a_ledger<T> and (std_set_assigning_in_is_valid_after_a_refusal or not is_std_set_v<T>)) {
                                check_moving_in_element_by_element(t, a); // [container.reqmts]/25
                        }
                }
        });
}

// [container.alloc.reqmts]/30-31: a.swap(b)
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<all>([]<class T> -> void {
                // Swapping unequal allocators that do not propagate is undefined, so those swap under one allocator.
                using traits = std::allocator_traits<typename T::allocator_type>;
                static_assert(requires (T c) { { c.swap(c) } -> std::same_as<void>; }); // [container.alloc.reqmts]/30
                auto const a = allocator<T>(0);
                auto const b = traits::propagate_on_container_swap::value ? allocator<T>(1) : a;
                for (auto const& t : samples<T>(a)) {
                        auto x        = T(t, a);
                        auto y        = other<T>(b);
                        auto const x1 = x;
                        auto const y1 = y;
                        x.swap(y);
                        BOOST_CHECK(x == y1 and y == x1);                               // [container.alloc.reqmts]/31
                        BOOST_CHECK(x.get_allocator() == b and y.get_allocator() == a); // [container.reqmts]/65
                        swap(x, y);
                        BOOST_CHECK(x == x1 and y == y1);
                        BOOST_CHECK(x.get_allocator() == a and y.get_allocator() == b);
                }
        });
}

// [container.reqmts]/25: a.~X() gives back every allocation it made
BOOST_AUTO_TEST_CASE(DestructorDeallocates)
{
        test::for_each_type<all>([]<class T> -> void {
                if constexpr (keeps_a_ledger<T>) {
                        auto book = ledger();
                        for (auto const& t : samples<T>(allocator<T>(0))) {
                                {
                                        auto const u = T(t, ledger_allocator<T>(book));
                                        BOOST_CHECK(u == t);
                                }
                                BOOST_CHECK_EQUAL(book.live, 0LL); // [container.reqmts]/25
                        }
                }
        });
}

// [container.reqmts]/66: a single element that cannot be allocated leaves the container as it was
BOOST_AUTO_TEST_CASE(AFailedInsertionHasNoEffects)
{
        test::for_each_type<all>([]<class T> -> void {
                if constexpr (keeps_a_ledger<T>) {
                        check_failed_insertion<T>();
                }
        });
}

// [container.reqmts]/64: the allocator-extended constructors each column adds to the ones above
BOOST_AUTO_TEST_CASE(AllocatorArguments)
{
        test::for_each_type<all>([]<class T> -> void {
                auto const m = allocator<T>(1);
                if constexpr (requires { typename T::key_type; }) {
                        static_assert(requires (T::allocator_type a, T::key_compare const comp, std::initializer_list<typename T::value_type> il, T::value_type const* first, T::value_type const* last) {
                                T(comp, a);
                                T(first, last, a);
                                T(first, last, comp, a);
                                T(il, a);
                                T(il, comp, a);
                                T(std::from_range, il, a);
                                T(std::from_range, il, comp, a);
                        });
                        check_listed_and_ranged<T>(m);
                } else {
                        static_assert(requires (T::allocator_type a, std::initializer_list<bool> il) { T(il, a); });
                        check_counted<T>(m, true);
                }
        });
}

// [container.reqmts]/64: a memory_resource* converts to the polymorphic allocator.
BOOST_AUTO_TEST_CASE(AMemoryResourceConvertsToThePolymorphicAllocator)
{
        using pmr_bit_set = xstd::basic_bit_set<std::size_t, std::size_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, std::pmr::polymorphic_allocator<std::size_t>>; // NOLINT(modernize-use-transparent-functors): the default comparator, named to reach the allocator
        auto mr           = std::pmr::monotonic_buffer_resource();

        auto const s = pmr_bit_set({1, 2}, &mr);
        BOOST_CHECK(s.get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(s.size(), 2UZ);
}

// [container.reqmts]/64: uses-allocator construction hands each element the container's allocator.
BOOST_AUTO_TEST_CASE(AnAllocatorAwareContainerPassesItsAllocatorOn)
{
        using pmr_bit_set = xstd::basic_bit_set<std::size_t, std::size_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, std::pmr::polymorphic_allocator<std::size_t>>; // NOLINT(modernize-use-transparent-functors): the default comparator, named to reach the allocator
        auto mr           = std::pmr::monotonic_buffer_resource();
        auto sets         = std::pmr::vector<pmr_bit_set>(&mr);

        sets.emplace_back();
        BOOST_CHECK(sets.back().get_allocator().resource() == &mr);
}

namespace {

// A scoped_allocator_adaptor hands its inner allocator to an element it constructs, which must take it as converted.
template<class T, class... Args>
auto check_scoped_construction(Args... args)
        -> void
{
        using inner_allocator  = std::pmr::polymorphic_allocator<std::size_t>;
        using scoped_allocator = std::scoped_allocator_adaptor<std::allocator<T>, inner_allocator>;
        auto mr                = std::pmr::monotonic_buffer_resource();
        auto elements          = std::vector<T, scoped_allocator>(scoped_allocator(std::allocator<T>(), inner_allocator(&mr)));
        elements.emplace_back(args...);
        BOOST_CHECK(elements.back().get_allocator().resource() == &mr);
}

} // namespace

// [container.reqmts]/64: uses-allocator construction through a scoped_allocator_adaptor, in each reading.
BOOST_AUTO_TEST_CASE(AScopedAllocatorAdaptorPassesItsAllocatorOn)
{
        using pmr_allocator = std::pmr::polymorphic_allocator<std::size_t>;
        check_scoped_construction<std::pmr::set<std::size_t>>();
        check_scoped_construction<xstd::basic_bit_set<std::size_t, std::size_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, pmr_allocator>>(); // NOLINT(modernize-use-transparent-functors): the default comparator, named to reach the allocator
        check_scoped_construction<std::pmr::vector<bool>>(3UZ, true);
        check_scoped_construction<xstd::basic_bit_vector<std::size_t, pmr_allocator>>(3UZ, true);
}

// [container.reqmts]/64: a rebound std::allocator converts, as it does for std::set.
BOOST_AUTO_TEST_CASE(AConvertibleAllocatorArgumentIsTaken)
{
        static_assert(std::is_constructible_v<std::set<std::size_t>, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::bit_set, std::allocator<int>>);
        BOOST_CHECK(true);
}

// [container.reqmts]/64: std::set's comparator arguments are taken alongside the allocator, and change nothing.
BOOST_AUTO_TEST_CASE(TheComparatorArgumentsAreAcceptedAlongsideTheAllocator)
{
        auto const comp = xstd::bit_set::key_compare(); // NOLINT(modernize-use-transparent-functors): std::set<std::size_t>::key_compare
        BOOST_CHECK(xstd::bit_set({3, 1}, comp, std::allocator<std::size_t>()) == xstd::bit_set({1, 3}));
}

// [container.reqmts]/64: {} is a default allocator after a comparator, as for std::set, and after a count and a value.
BOOST_AUTO_TEST_CASE(AnEmptyBraceIsADefaultAllocator)
{
        auto const comp = xstd::bit_set::key_compare(); // NOLINT(modernize-use-transparent-functors): std::set<std::size_t>::key_compare
        BOOST_CHECK(std::set<std::size_t>({3, 1}, comp, {}) == std::set<std::size_t>({1, 3}));
        BOOST_CHECK(xstd::bit_set({3, 1}, comp, {}) == xstd::bit_set({1, 3}));
}

// [container.reqmts]/64: a memory_resource* converts to a sequence's polymorphic allocator.
BOOST_AUTO_TEST_CASE(AMemoryResourceConvertsToThePolymorphicAllocatorOfASequence)
{
        using pmr_bit_vector = xstd::basic_bit_vector<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        auto mr              = std::pmr::monotonic_buffer_resource();

        auto const v = pmr_bit_vector(3, true, &mr);
        BOOST_CHECK(v.get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
}

// [container.reqmts]/64: uses-allocator construction hands each sequence the container's allocator.
BOOST_AUTO_TEST_CASE(AnAllocatorAwareContainerPassesItsAllocatorOnToASequence)
{
        using pmr_bit_vector = xstd::basic_bit_vector<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
        static_assert(std::uses_allocator_v<pmr_bit_vector, std::pmr::polymorphic_allocator<pmr_bit_vector>>);

        auto mr      = std::pmr::monotonic_buffer_resource();
        auto vectors = std::pmr::vector<pmr_bit_vector>(&mr);
        vectors.emplace_back(3);

        BOOST_CHECK(vectors.back().get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(vectors.back().size(), 3UZ);

        vectors.emplace_back();
        BOOST_CHECK(vectors.back().get_allocator().resource() == &mr);
        BOOST_CHECK(vectors.back().empty());
}

// [container.reqmts]/64: a rebound std::allocator converts, and {} is a default one, as for std::vector<bool>.
BOOST_AUTO_TEST_CASE(AConvertibleOrEmptyAllocatorArgumentIsTakenByASequence)
{
        static_assert(std::is_constructible_v<std::vector<bool>, std::size_t, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::bit_vector, std::size_t, std::allocator<int>>);

        auto const v = xstd::bit_vector(3, true, {});
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
}

// [container.reqmts]/64: the count constructors are explicit with the allocator as without it.
BOOST_AUTO_TEST_CASE(TheCountConstructorsAreExplicit)
{
        static_assert(not list_converts_from<std::vector<bool>, std::size_t, std::allocator<bool>>);
        static_assert(not list_converts_from<xstd::bit_vector, std::size_t, std::allocator<std::size_t>>);
        BOOST_CHECK(true);
}

// [container.reqmts]/64: the allocator-only constructors cannot throw, and a width in the type takes no allocator.
BOOST_AUTO_TEST_CASE(OnlyARunTimeWidthTakesAnAllocator)
{
        static_assert(std::is_nothrow_constructible_v<std::vector<bool>, std::allocator<bool> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_vector, std::allocator<std::size_t> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::basic_bit_vector<std::uint8_t>, std::allocator<std::uint8_t> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::basic_bit_small_vector<std::uint64_t, 64>, xstd::basic_bit_small_vector<std::uint64_t, 64>::allocator_type const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::basic_bit_small_set<std::size_t, std::uint64_t, 64>, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 64>::allocator_type const&>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, std::allocator<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, std::initializer_list<bool>, std::allocator<std::size_t>>);
        BOOST_CHECK(true);
}

// [container.alloc.reqmts]/1: every container is allocator-aware but array and inplace_vector, and a view is not one
BOOST_AUTO_TEST_CASE(OnlyAnAllocatorAwareCandidateTakesAnAllocator)
{
        // The fixed and bounded columns of every reading take no allocator argument, and no view does.
        test::for_each_type<test::spec::set::all>([]<class T> -> void { check_allocator_members<T>(); });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { check_allocator_members<T>(); });
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void { check_allocator_members<T>(); });
        test::for_each_type<test::spec::span::all>([]<class T> -> void { check_allocator_members<T>(); });
        static_assert(test::spec::has_allocator_extended_copy<xstd::bit_vector> and test::spec::has_allocator_extended_move<xstd::bit_vector>);
        static_assert(test::spec::has_allocator_extended_copy<xstd::bit_set> and test::spec::has_allocator_extended_move<xstd::bit_set>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
