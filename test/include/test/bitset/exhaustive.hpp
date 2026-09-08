//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BITSET_EXHAUSTIVE_HPP
#define TEST_BITSET_EXHAUSTIVE_HPP

#include <test/bitset/factory.hpp> // make_bitset
#include <test/dynamic.hpp>        // dynamic
#include <algorithm>               // max
#include <cassert>                 // assert
#include <ranges>                  // iota

#ifdef _MSC_VER
        // std::bitset<0> and xstd::bit_static_set<0> give bogus "unreachable code" warnings
        __pragma(warning(disable: 4702))
#endif

namespace test::bitset {

template<class X, auto Limit>
inline constexpr auto limit_v = dynamic<X> ? Limit : X().size();

inline constexpr auto L0 = 128UZ;
inline constexpr auto L1 =  64UZ;
inline constexpr auto L2 =  32UZ;
inline constexpr auto L3 =  16UZ;
inline constexpr auto L4 =   8UZ;

namespace on0 {

template<class X, auto N = limit_v<X, L0>>
auto empty_set(auto fun)
{
        auto a = make_bitset<X>(N); assert(a.none());  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        fun(a);
}

template<class X, auto N = limit_v<X, L0>>
auto full_set(auto fun)
{
        auto a = make_bitset<X>(N, true); assert(a.all());  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        fun(a);
}

template<class X, auto N = limit_v<X, L0>>
auto empty_set_pair(auto fun)
{
        auto a = make_bitset<X>(N); assert(a.none());  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        auto b = make_bitset<X>(N); assert(b.none());  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        fun(a, b);
}

}       // namespace on0

namespace on1 {

template<class X, auto N = limit_v<X, L1>>
auto all_valid(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                fun(i);
        }
}

template<class X, auto N = limit_v<X, L1>>
auto any_value(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N + 1)) {
                fun(i);
        }
}

template<class X, auto N = limit_v<X, L1>>
auto all_cardinality_sets(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N + 1)) {
                auto a = make_bitset<X>(N);
                for (auto const j : std::views::iota(0UZ, i)) {
                        a.set(j);
                }
                assert(a.count() == i);
                fun(a);
        }
}

template<class X, auto N = limit_v<X, L1>>
auto all_singleton_sets(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                auto a = make_bitset<X>(N); a.set(i); assert(a.count() == 1);
                fun(a);
        }
}

}       // namespace on1

namespace on2 {

template<class X, auto N = limit_v<X, L2>>
auto all_singleton_set_pairs(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                for (auto const j : std::views::iota(0UZ, N)) {
                        auto a = make_bitset<X>(N); a.set(i); assert(a.count() == 1);
                        auto b = make_bitset<X>(N); b.set(j); assert(b.count() == 1);
                        fun(a, b);
                }
        }
}

template<class X, auto N = limit_v<X, L2>>
auto all_doubleton_sets(auto fun)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = make_bitset<X>(N); a.set(i); a.set(j); assert(a.count() == 2);
                        fun(a);
                }
        }
}

}       // namespace on2

namespace on3 {

template<class X, auto N = limit_v<X, L3>>
auto all_singleton_set_triples(auto fun)
{
        for (auto i : std::views::iota(0UZ, N)) {
                for (auto j : std::views::iota(0UZ, N)) {
                        for (auto k : std::views::iota(0UZ, N)) {
                                auto a = make_bitset<X>(N); a.set(i); assert(a.count() == 1);
                                auto b = make_bitset<X>(N); b.set(j); assert(b.count() == 1);
                                auto c = make_bitset<X>(N); c.set(k); assert(c.count() == 1);
                                fun(a, b, c);
                        }
                }
        }
}

template<class X, auto N = limit_v<X, L3>>
auto all_triplet_sets(auto fun)
{
        for (auto const k : std::views::iota(2UZ, std::ranges::max(N, 2UZ))) {
                for (auto const j : std::views::iota(1UZ, k)) {
                        for (auto const i : std::views::iota(0UZ, j)) {
                                auto a = make_bitset<X>(N); a.set(i); a.set(j); a.set(k); assert(a.count() == 3);
                                fun(a);
                        }
                }
        }
}

}       // namespace on3

namespace on4 {

template<class X, auto N = limit_v<X, L4>>
auto all_doubleton_set_pairs(auto fun)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                for (auto const n : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                        for (auto const i : std::views::iota(0UZ, j)) {
                                for (auto const m : std::views::iota(0UZ, n)) {
                                        auto a = make_bitset<X>(N); a.set(i); a.set(j); assert(a.count() == 2);
                                        auto b = make_bitset<X>(N); b.set(m); b.set(n); assert(b.count() == 2);
                                        fun(a, b);
                                }
                        }
                }
        }
}

}       // namespace on4

} // namespace test::bitset

#endif // TEST_BITSET_EXHAUSTIVE_HPP
