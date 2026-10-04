//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/composable.hpp>  // decrement_modulo, increment_modulo
#include <test/spec/input.hpp>      // context
#include <test/spec/rejection.hpp>  // has_shift_left, has_shift_left_assign, has_shift_right, has_shift_right_assign
#include <test/spec/set.hpp>        // const_views, keyed_sets_with_singletons, owners, sets, views
#include <test/spec/view.hpp>       // owner_t
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                  // size_t
#include <limits>                   // numeric_limits

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Shift)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

// xstd set: constexpr X operator<<(const X& lhs, size_t n);
BOOST_AUTO_TEST_CASE(ShiftLeft)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                // A left shift translates every key and keeps those below max_size(), whatever the owner.
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        composable::increment_modulo()(a, k);
                }
        });
        // A view shifts in place, and copies no keys into a value it could return, by this library's design.
        test::for_each_type<test::spec::set::views>([]<class T> -> void {
                static_assert(test::spec::has_shift_left_assign<T> and test::spec::has_shift_left<test::spec::owner_t<T>>);
                static_assert(not test::spec::has_shift_left<T>);
        });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_shift_left_assign<T>); });
}

// xstd set: constexpr X operator>>(const X& lhs, size_t n);
BOOST_AUTO_TEST_CASE(ShiftRight)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        composable::decrement_modulo()(a, k);
                }
        });
        // A view shifts in place, and copies no keys into a value it could return, by this library's design.
        test::for_each_type<test::spec::set::views>([]<class T> -> void {
                static_assert(test::spec::has_shift_right_assign<T> and test::spec::has_shift_right<test::spec::owner_t<T>>);
                static_assert(not test::spec::has_shift_right<T>);
        });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_shift_right_assign<T>); });
}

namespace {

// Both shifts by n, where the owner has them; n is at least max_size(), so neither leaves a key behind.
template<class T>
auto check_empties(T const& a, std::size_t n) -> void
{
        if constexpr (requires (T x) { x <<= 1UZ; }) {
                auto b = a;
                b <<= n;
                BOOST_CHECK(b.empty());
        }
        if constexpr (requires (T x) { x >>= 1UZ; }) {
                auto c = a;
                c >>= n;
                BOOST_CHECK(c.empty());
        }
}

} // namespace

// A shift by max_size() or more carries every key past it, as with std::bitset's [bitset.members], at any width.
BOOST_AUTO_TEST_CASE(ShiftingByMaxSizeOrMoreEmpties)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                constexpr auto top = std::numeric_limits<std::size_t>::max();
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        // Saturated: near the top of size_t, N + k would wrap onto a distance below max_size().
                        auto const N    = a.max_size();
                        auto const past = [=](std::size_t k) -> std::size_t { return k > top - N ? top : N + k; };
                        for (auto const n : {N, past(1UZ), past(N + 3UZ), top}) {
                                check_empties(a, n);
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
