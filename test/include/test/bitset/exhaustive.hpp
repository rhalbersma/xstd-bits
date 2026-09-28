//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BITSET_EXHAUSTIVE_HPP
#define TEST_BITSET_EXHAUSTIVE_HPP

#include <test/bitset/factory.hpp>        // make_bitset
#include <test/dynamic.hpp>               // dynamic
#include <test/set/exhaustive.hpp>        // static_capacity
#include <xstd/bits/detail/ownership.hpp> // owned_storage
#include <algorithm>                      // max, min
#include <cassert>                        // assert
#include <cstddef>                        // size_t
#include <ranges>                         // iota

#ifdef _MSC_VER

// std::bitset<0> and xstd::bit_fixed_set<0> give bogus "unreachable code" warnings
#pragma warning(disable : 4702)

#endif

namespace test::bitset {

// A static width is its own limit, a capacity caps the sweep's, and an unbounded run-time width takes the sweep's.
template<class X, std::size_t Limit>
inline constexpr auto limit_v = [] -> std::size_t {
        if constexpr (not dynamic<X>) {
                return X().size();
        } else if constexpr (test::set::static_capacity<X>) {
                return std::ranges::min(xstd::bits::detail::owned_storage<X>::bits_type::static_capacity(), Limit);
        } else {
                return Limit;
        }
}();

inline constexpr auto L0 = 128UZ;
inline constexpr auto L1 = 64UZ;
inline constexpr auto L2 = 32UZ;
inline constexpr auto L3 = 16UZ;
inline constexpr auto L4 = 8UZ;

namespace on0 {

template<class X, auto N = limit_v<X, L0>>
constexpr auto empty_set(auto fun, std::size_t width = N)
{
        auto a = make_bitset<X>(width); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        assert(a.none());
        fun(a);
}

template<class X, auto N = limit_v<X, L0>>
constexpr auto full_set(auto fun, std::size_t width = N)
{
        auto a = make_bitset<X>(width, true); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        assert(a.all());
        fun(a);
}

template<class X, auto N = limit_v<X, L0>>
constexpr auto empty_set_pair(auto fun, std::size_t width = N)
{
        auto a = make_bitset<X>(width); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        assert(a.none());
        auto b = make_bitset<X>(width); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        assert(b.none());
        fun(a, b);
}

} // namespace on0

namespace on1 {

template<class X, auto N = limit_v<X, L1>>
constexpr auto all_valid(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                fun(i);
        }
}

template<class X, auto N = limit_v<X, L1>>
constexpr auto any_value(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width + 1)) {
                fun(i);
        }
}

template<class X, auto N = limit_v<X, L1>>
constexpr auto all_cardinality_sets(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width + 1)) {
                auto a = make_bitset<X>(width);
                for (auto const j : std::views::iota(0UZ, i)) {
                        a.set(j);
                }
                assert(a.count() == i);
                fun(a);
        }
}

template<class X, auto N = limit_v<X, L1>>
constexpr auto all_singleton_sets(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                auto a = make_bitset<X>(width);
                a.set(i);
                assert(a.count() == 1);
                fun(a);
        }
}

} // namespace on1

namespace on2 {

template<class X, auto N = limit_v<X, L2>>
constexpr auto all_singleton_set_pairs(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                for (auto const j : std::views::iota(0UZ, width)) {
                        auto a = make_bitset<X>(width);
                        a.set(i);
                        assert(a.count() == 1);
                        auto b = make_bitset<X>(width);
                        b.set(j);
                        assert(b.count() == 1);
                        fun(a, b);
                }
        }
}

template<class X, auto N = limit_v<X, L2>>
constexpr auto all_doubleton_sets(auto fun, std::size_t width = N)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = make_bitset<X>(width);
                        a.set(i);
                        a.set(j);
                        assert(a.count() == 2);
                        fun(a);
                }
        }
}

} // namespace on2

namespace on3 {

template<class X, auto N = limit_v<X, L3>>
constexpr auto all_singleton_set_triples(auto fun, std::size_t width = N)
{
        for (auto const i : std::views::iota(0UZ, width)) {
                for (auto const j : std::views::iota(0UZ, width)) {
                        for (auto const k : std::views::iota(0UZ, width)) {
                                auto a = make_bitset<X>(width);
                                a.set(i);
                                assert(a.count() == 1);
                                auto b = make_bitset<X>(width);
                                b.set(j);
                                assert(b.count() == 1);
                                auto c = make_bitset<X>(width);
                                c.set(k);
                                assert(c.count() == 1);
                                fun(a, b, c);
                        }
                }
        }
}

template<class X, auto N = limit_v<X, L3>>
constexpr auto all_triplet_sets(auto fun, std::size_t width = N)
{
        for (auto const k : std::views::iota(2UZ, std::ranges::max(width, 2UZ))) {
                for (auto const j : std::views::iota(1UZ, k)) {
                        for (auto const i : std::views::iota(0UZ, j)) {
                                auto a = make_bitset<X>(width);
                                a.set(i);
                                a.set(j);
                                a.set(k);
                                assert(a.count() == 3);
                                fun(a);
                        }
                }
        }
}

} // namespace on3

namespace on4 {

template<class X, auto N = limit_v<X, L4>>
constexpr auto all_doubleton_set_pairs(auto fun, std::size_t width = N)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                for (auto const n : std::views::iota(1UZ, std::ranges::max(width, 1UZ))) {
                        for (auto const i : std::views::iota(0UZ, j)) {
                                for (auto const m : std::views::iota(0UZ, n)) {
                                        auto a = make_bitset<X>(width);
                                        a.set(i);
                                        a.set(j);
                                        assert(a.count() == 2);
                                        auto b = make_bitset<X>(width);
                                        b.set(m);
                                        b.set(n);
                                        assert(b.count() == 2);
                                        fun(a, b);
                                }
                        }
                }
        }
}

} // namespace on4

} // namespace test::bitset

#endif // TEST_BITSET_EXHAUSTIVE_HPP
