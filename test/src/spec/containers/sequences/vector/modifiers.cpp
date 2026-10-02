//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/allocator.hpp> // basic_guarantee, ledger, ledger_allocator, strong_guarantee
#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // alternating, mem_erase_keeps_prefix, mem_insert_keeps_prefix, nth, single_pass, single_pass_iterator
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // ledgered, positions, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <algorithm>                    // equal
#include <cstddef>                      // size_t
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Modifiers)

using namespace test::sequence;
using test::container::basic_guarantee;
using test::container::strong_guarantee;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

namespace {

template<class X>
inline constexpr auto is_std_vector_v = false;

template<class Allocator>
inline constexpr auto is_std_vector_v<std::vector<bool, Allocator>> = true;

#ifdef _MSVC_STL_VERSION

// MSVC's STL inserts or appends a single pass a bool at a time through every member, insert_range included.
inline constexpr auto std_vector_single_pass_has_no_effects = false;
inline constexpr auto std_vector_single_pass_insert_range_has_no_effects = false;

#elifdef __GLIBCXX__

// libstdc++ inserts or appends a single pass a bool at a time, except insert_range, which collects it first.
inline constexpr auto std_vector_single_pass_has_no_effects = false;
inline constexpr auto std_vector_single_pass_insert_range_has_no_effects = true;

#else

inline constexpr auto std_vector_single_pass_has_no_effects = true;
inline constexpr auto std_vector_single_pass_insert_range_has_no_effects = true;

#endif

// No effects where the library gives them, and the basic guarantee of the departure where not.
template<bool HasNoEffects, class X, class Op>
[[nodiscard]] auto single_pass_guarantee(X const& a, Op op)
        -> bool
{
        if constexpr (HasNoEffects or not is_std_vector_v<X>) {
                return strong_guarantee(a, op);
        } else {
                return basic_guarantee(a, op);
        }
}

// One insertion at position p in each shape, from a single bool to a range past a small sequence's inline blocks.
template<class X>
auto check_insertions_have_no_effects(X const& a, std::size_t p)
        -> void
{
        auto const more = alternating(100UZ);
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert(nth(x, p), true)); }));                     // [vector.modifiers]/2
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert(nth(x, p), more.size(), true)); }));        // [vector.modifiers]/2
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert(nth(x, p), more.begin(), more.end())); })); // [vector.modifiers]/2
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert(nth(x, p), {true, false, true})); }));      // [vector.modifiers]/2
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.emplace(nth(x, p), true)); }));                    // [vector.modifiers]/2
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.emplace_back(true)); }));                          // [vector.modifiers]/2
        BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { x.push_back(true); }));                                                // [vector.modifiers]/2
        auto const first = single_pass_iterator(more.cbegin());
        auto const last = single_pass_iterator(more.cend());
        BOOST_CHECK(single_pass_guarantee<std_vector_single_pass_has_no_effects>(a, [&](X& x) -> void { static_cast<void>(x.insert(nth(x, p), first, last)); })); // [vector.modifiers]/2
}

// The range members, where the standard library has them, from a sized range and from a single pass.
template<class X>
auto check_range_insertions_have_no_effects(X const& a, std::size_t p)
        -> void
{
        auto const more = alternating(100UZ);
        if constexpr (requires (X& x) { x.append_range(more); }) {
                BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert_range(nth(x, p), more)); }));                                                                       // [vector.modifiers]/2
                BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { x.append_range(more); }));                                                                                                     // [vector.modifiers]/2
                BOOST_CHECK(single_pass_guarantee<std_vector_single_pass_insert_range_has_no_effects>(a, [&](X& x) -> void { static_cast<void>(x.insert_range(nth(x, p), single_pass(more))); })); // [vector.modifiers]/2
                BOOST_CHECK(single_pass_guarantee<std_vector_single_pass_has_no_effects>(a, [&](X& x) -> void { x.append_range(single_pass(more)); }));                                            // [vector.modifiers]/2
        }
}

// A bool's copy, move and assignment throw nothing, so no erase may throw, and none asks the allocator for anything.
template<class X>
[[nodiscard]] auto erases_without_allocating(X const& a, std::size_t first, std::size_t last, bool single)
        -> bool
{
        auto book = test::container::ledger();
        auto x = X(a, test::container::ledger_allocator<X>(book));
        book.budget = 0;
        try {
                static_cast<void>(single ? x.erase(nth(x, first)) : x.erase(nth(x, first), nth(x, last)));
        } catch (...) {
                return false;
        }
        return book.refusals == 0 and std::equal(x.cbegin(), nth(x, first), a.cbegin()) and std::equal(nth(x, first), x.cend(), nth(a, last), a.cend());
}

template<class X>
auto check_erasures_allocate_nothing(X const& a, std::size_t p)
        -> void
{
        if (p < a.size()) {
                BOOST_CHECK(erases_without_allocating(a, p, p + 1UZ, true)); // [vector.modifiers]/5
        }
        BOOST_CHECK(erases_without_allocating(a, p, a.size(), false)); // [vector.modifiers]/5
        BOOST_CHECK(erases_without_allocating(a, 0UZ, p, false));      // [vector.modifiers]/5
}

} // namespace

// [vector.modifiers]/2: constexpr iterator insert(const_iterator position, const T& x);
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_insert_keeps_prefix()(a, p);
                }
        });
        test::for_each_type<test::spec::sequence::ledgered>([]<class T> -> void {
                // A bool raises nothing of its own, so every insertion an allocation refuses has no effects.
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        check_insertions_have_no_effects(a, p);
                        check_range_insertions_have_no_effects(a, p);
                }
        });
}

// [vector.modifiers]/4-5: constexpr iterator erase(const_iterator position);
BOOST_AUTO_TEST_CASE(Erase)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_erase_keeps_prefix()(a, p);
                }
        });
        test::for_each_type<test::spec::sequence::ledgered>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        check_erasures_allocate_nothing(a, p);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
