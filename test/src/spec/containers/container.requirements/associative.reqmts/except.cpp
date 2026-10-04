//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/allocator.hpp> // basic_guarantee
#include <test/for_each_type.hpp>       // for_each_type
#include <test/set/primitives.hpp>      // mem_clear_erase_nothrow, mem_insert_or_nothing, mem_swap_nothrow
#include <test/spec/input.hpp>          // context
#include <test/spec/set.hpp>            // all, fixed_listed_sets, held_width_v, keyed_sets, ledgered, owners, pairs, sets
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                     // swap
#include <cstddef>                      // size_t
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(AssociativeReqmts)
BOOST_AUTO_TEST_SUITE(Except)

using namespace test::set;
using test::container::basic_guarantee;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

namespace {

// A key past the ones a set holds without allocating, so that inserting it grows even a small set onto the heap.
template<class X>
[[nodiscard]] auto spilled(std::size_t k)
        -> std::size_t
{
        return test::spec::set::held_width_v<X> + 1000UZ + k;
}

// Each key moved past the ones a set holds without allocating.
template<class X>
[[nodiscard]] auto spilled(std::vector<std::size_t> const& keys)
        -> std::vector<std::size_t>
{
        auto result = std::vector<std::size_t>();
        for (auto const k : keys) {
                result.push_back(spilled<X>(k));
        }
        return result;
}

} // namespace

// [associative.reqmts.except]/1: a.clear(), a.erase(k)
BOOST_AUTO_TEST_CASE(ClearErase)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        mem_clear_erase_nothrow()(a, k);
                }
        });
}

// [associative.reqmts.except]/2: a.insert(t), a.insert(p, t), a.emplace(args), a.emplace_hint(p, args)
BOOST_AUTO_TEST_CASE(InsertEmplace)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // A key past what the set can hold is the one insertion an xstd set refuses by throwing.
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_insert_or_nothing()(a, a.max_size());
                }
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        mem_insert_or_nothing()(a, k);
                }
        });
        test::for_each_type<test::spec::set::ledgered>([]<class T> -> void {
                // Every allocation refused in turn, at the keys a set holds without growing and at one past them.
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        mem_insert_or_nothing()(a, k);
                }
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_insert_or_nothing()(a, spilled<T>(a.size()));
                }
        });
}

// [associative.reqmts.except]/3: a.swap(b), swap(a, b)
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                static_assert(requires (T x, T y) {
                        { x.swap(y) } noexcept;
                        { swap(x, y) } noexcept;
                        { std::ranges::swap(x, y) } noexcept;
                }); // [associative.reqmts.except]/3
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_swap_nothrow()(a, b);
                }
        });
}

// [container.reqmts]/25: a.insert(i, j), a.insert_range(rg) and a.insert(il), each allocation refused in turn
BOOST_AUTO_TEST_CASE(ARefusedRangeInsertionLeavesAValidSet)
{
        test::for_each_type<test::spec::set::ledgered>([]<class T> -> void {
                // No strong guarantee, so only a valid set; a sample with a key per refusal would make this quadratic.
                for (auto const [from, a, keys] : inputs::fixed_listed_sets<T>()) {
                        auto const on_failure = context(from, a, keys);
                        auto const more       = spilled<T>(keys);
                        BOOST_CHECK(basic_guarantee(a, [&](T& x) -> void { x.insert(more.begin(), more.end()); }));                   // [container.reqmts]/25
                        BOOST_CHECK(basic_guarantee(a, [&](T& x) -> void { x.insert_range(more); }));                                 // [container.reqmts]/25
                        BOOST_CHECK(basic_guarantee(a, [&](T& x) -> void { x.insert({spilled<T>(0UZ), spilled<T>(keys.size())}); })); // [container.reqmts]/25
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
