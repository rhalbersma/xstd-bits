//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_COMPOSABLE_HPP
#define TEST_SET_COMPOSABLE_HPP

#include <boost/test/unit_test.hpp>        // BOOST_CHECK, BOOST_CHECK_EQUAL
#include <range/v3/view/set_algorithm.hpp> // set_difference, set_intersection, set_symmetric_difference, set_union
#include <algorithm>                       // includes
#include <cstddef>                         // size_t
#include <ranges>                          // filter, to, transform

namespace test::set::composable {

// Each containment is std::ranges::includes(r1, r2), which asks whether r2 lies within r1, read from one side.

struct subset
{
        template<class X>
        auto operator()(const X& a, const X& b) const noexcept
        {
                if constexpr (requires { a.is_subset_of(b); }) {
                        BOOST_CHECK_EQUAL(a.is_subset_of(b), std::ranges::includes(b, a, a.value_comp()));
                }
        }
};

struct proper_subset
{
        template<class X>
        auto operator()(const X& a, const X& b) const noexcept
        {
                if constexpr (requires { a.is_proper_subset_of(b); }) {
                        BOOST_CHECK_EQUAL(a.is_proper_subset_of(b), std::ranges::includes(b, a, a.value_comp()) and not std::ranges::includes(a, b, a.value_comp()));
                }
        }
};

struct superset
{
        template<class X>
        auto operator()(const X& a, const X& b) const noexcept
        {
                if constexpr (requires { a.is_superset_of(b); }) {
                        BOOST_CHECK_EQUAL(a.is_superset_of(b), std::ranges::includes(a, b, a.value_comp()));
                }
        }
};

struct proper_superset
{
        template<class X>
        auto operator()(const X& a, const X& b) const noexcept
        {
                if constexpr (requires { a.is_proper_superset_of(b); }) {
                        BOOST_CHECK_EQUAL(a.is_proper_superset_of(b), std::ranges::includes(a, b, a.value_comp()) and not std::ranges::includes(b, a, a.value_comp()));
                }
        }
};

// The four below insert through ranges::to, which past max_size() is std::length_error, so none is noexcept.

struct set_union
{
        template<class X>
        auto operator()(const X& a, const X& b) const
        {
                if constexpr (requires { a | b; }) {
                        BOOST_CHECK((a | b) == (::ranges::views::set_union(a, b, a.value_comp()) | std::ranges::to<X>()));
                }
        }
};

struct set_intersection
{
        template<class X>
        auto operator()(const X& a, const X& b) const
        {
                if constexpr (requires { a & b; }) {
                        BOOST_CHECK((a & b) == (::ranges::views::set_intersection(a, b, a.value_comp()) | std::ranges::to<X>()));
                }
        }
};

struct set_difference
{
        template<class X>
        auto operator()(const X& a, const X& b) const
        {
                if constexpr (requires { a - b; }) {
                        BOOST_CHECK((a - b) == (::ranges::views::set_difference(a, b, a.value_comp()) | std::ranges::to<X>()));
                }
        }
};

struct set_symmetric_difference
{
        template<class X>
        auto operator()(const X& a, const X& b) const
        {
                if constexpr (requires { a ^ b; }) {
                        BOOST_CHECK((a ^ b) == (::ranges::views::set_symmetric_difference(a, b, a.value_comp()) | std::ranges::to<X>()));
                }
        }
};

struct increment_modulo
{
        template<class X>
        auto operator()(const X& a, std::size_t n) const
        {
                if constexpr (requires { a << n; }) {
                        auto const N = a.max_size();
                        BOOST_CHECK(
                                (a << n) == (a | std::views::transform([=](auto x) { return x + n; }) | std::views::filter([=](auto x) { return x < N; }) | std::ranges::to<X>())
                        );
                }
        }
};

struct decrement_modulo
{
        template<class X>
        auto operator()(const X& a, std::size_t n) const
        {
                if constexpr (requires { a >> n; }) {
                        auto const N = a.max_size();
                        BOOST_CHECK(
                                (a >> n) == (a | std::views::filter([=](auto x) { return x >= n; }) | std::views::transform([=](auto x) { return x - n; }) | std::views::filter([=](auto x) { return x < N; }) | std::ranges::to<X>())
                        );
                }
        }
};

} // namespace test::set::composable

#endif // TEST_SET_COMPOSABLE_HPP
