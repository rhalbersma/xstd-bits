//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_EXHAUSTIVE_HPP
#define TEST_SET_EXHAUSTIVE_HPP

#include <xstd/bits/detail/ownership.hpp> // owned_storage
#include <algorithm>                      // max, min
#include <array>                          // array
#include <cassert>                        // assert
#include <cstddef>                        // size_t
#include <initializer_list>               // initializer_list
#include <ranges>                         // iota, to
#include <span>                           // dynamic_extent

#ifdef _MSC_VER

// xstd::bit_fixed_set<0> gives bogus "unreachable code" warnings
#pragma warning(disable : 4702)

#endif

namespace test::set {

inline constexpr auto L1 = 128UZ;
inline constexpr auto L2 = 64UZ;
inline constexpr auto L3 = 32UZ;
inline constexpr auto L4 = 16UZ;

// A width the type fixes, which a default-constructed object already has.
template<class X>
concept static_width = requires { typename xstd::bits::detail::owned_storage<X>::bits_type; } and (xstd::bits::detail::owned_storage<X>::bits_type::extent != std::dynamic_extent);

// A run-time width under a capacity in the type, which a sweep past it would meet as an exception.
template<class X>
concept static_capacity = requires { typename xstd::bits::detail::owned_storage<X>::bits_type; } and xstd::bits::detail::owned_storage<X>::bits_type::has_static_capacity;

// A static width is its own limit, a capacity caps the sweep's, and an unbounded one takes the sweep's.
template<class X, std::size_t Limit>
inline constexpr auto limit_v = [] -> std::size_t {
        if constexpr (static_width<X>) {
                // NOLINTNEXTLINE(readability-static-accessed-through-instance): a function on the standard's owners.
                return X().max_size();
        } else if constexpr (static_capacity<X>) {
                return std::ranges::min(xstd::bits::detail::owned_storage<X>::bits_type::static_capacity(), Limit);
        } else {
                return Limit;
        }
}();

namespace on0 {

template<class X>
constexpr auto empty_set(auto fun)
{
        X a;
        assert(a.empty());
        fun(a);
}

template<class X, std::size_t N = limit_v<X, L1>>
constexpr auto full_set(auto fun, std::size_t width = N)
{
        auto a = std::views::iota(0UZ, width) | std::ranges::to<X>(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        assert(a.size() == width);
        fun(a);
}

template<class X>
constexpr auto empty_set_pair(auto fun)
{
        X a;
        assert(a.empty());
        X b;
        assert(b.empty());
        fun(a, b);
}

} // namespace on0

namespace on1 {

template<class X, std::size_t N = limit_v<X, L1>>
constexpr auto all_valid(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                fun(i);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
constexpr auto all_cardinality_sets(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width + 1)) {
                auto a = std::views::iota(0UZ, i) | std::ranges::to<X>(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                assert(a.size() == i);
                fun(a);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
constexpr auto all_singleton_arrays(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                auto a = std::array{i}; // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                assert(a.size() == 1);
                fun(a);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
constexpr auto all_singleton_sets(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                auto a = X({i}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                assert(a.size() == 1);
                fun(a);
        }
}

} // namespace on1

namespace on2 {

template<class X, std::size_t N = limit_v<X, L2>>
constexpr auto all_doubleton_arrays(auto fun, std::size_t width = N)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = std::array{i, j}; // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        assert(a.size() == 2);
                        fun(a);
                }
        }
}

template<class X, std::size_t N = limit_v<X, L2>>
constexpr auto all_doubleton_sets(auto fun, std::size_t width = N)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = X({i, j}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        assert(a.size() == 2);
                        fun(a);
                }
        }
}

template<class X, std::size_t N = limit_v<X, L2>>
constexpr auto all_singleton_set_pairs(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                for (auto const j : std::views::iota(0UZ, width)) {
                        auto a = X({i}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        assert(a.size() == 1);
                        auto b = X({j}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        assert(b.size() == 1);
                        fun(a, b);
                }
        }
}

} // namespace on2

namespace on3 {

template<class X, std::size_t N = limit_v<X, L3>>
constexpr auto all_singleton_set_triples(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                for (auto const j : std::views::iota(0UZ, width)) {
                        for (auto const k : std::views::iota(0UZ, width)) {
                                auto a = X({i}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                assert(a.size() == 1);
                                auto b = X({j}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                assert(b.size() == 1);
                                auto c = X({k}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                assert(c.size() == 1);
                                fun(a, b, c);
                        }
                }
        }
}

} // namespace on3

namespace on4 {

template<class X, std::size_t N = limit_v<X, L4>>
constexpr auto all_doubleton_set_pairs(auto fun, std::size_t width = N)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                for (auto const n : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                        for (auto const i : std::views::iota(0UZ, j)) {
                                for (auto const m : std::views::iota(0UZ, n)) {
                                        auto a = X({i, j}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                        assert(a.size() == 2);
                                        auto b = X({m, n}); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                        assert(b.size() == 2);
                                        fun(a, b);
                                }
                        }
                }
        }
}

} // namespace on4

} // namespace test::set

#endif // TEST_SET_EXHAUSTIVE_HPP
