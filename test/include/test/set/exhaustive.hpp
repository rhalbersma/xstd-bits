//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_EXHAUSTIVE_HPP
#define TEST_SET_EXHAUSTIVE_HPP

#include <xstd/bits/bit_traits.hpp> // static_bit_extent
#include <xstd/bits/ownership.hpp>  // owned_storage
#include <algorithm>                // max
#include <array>                    // array
#include <cassert>                  // assert
#include <cstddef>                  // size_t
#include <initializer_list>         // initializer_list
#include <ranges>                   // iota, to

#ifdef _MSC_VER
        // xstd::bit_static_set<0> gives bogus "unreachable code" warnings
        __pragma(warning(disable: 4702))
#endif

namespace test::set {

inline constexpr auto L1 = 128UZ;
inline constexpr auto L2 =  64UZ;
inline constexpr auto L3 =  32UZ;
inline constexpr auto L4 =  16UZ;

// A static width is its own limit; a growing one, ours or the standard library's, takes the sweep's.
template<class X>
concept static_width = requires { typename xstd::owned_storage<X>::bits_type; } and xstd::static_bit_extent<typename X::traits_type, typename xstd::owned_storage<X>::bits_type>;

template<class X, std::size_t Limit>
inline constexpr auto limit_v = [] -> std::size_t {
        if constexpr (static_width<X>) {
                return X().max_size();
        } else {
                return Limit;
        }
}();

namespace on0 {

template<class X>
auto empty_set(auto fun)
{
        X a; assert(a.empty());
        fun(a);
}

template<class X, std::size_t N = limit_v<X, L1>>
auto full_set(auto fun)
{
        auto a = std::views::iota(0UZ, N) | std::ranges::to<X>(); assert(a.size() == N);
        fun(a);
}

template<class X>
auto empty_set_pair(auto fun)
{
        X a; assert(a.empty());
        X b; assert(b.empty());
        fun(a, b);
}

}       // namespace on0

namespace on1 {

template<class X, std::size_t N = limit_v<X, L1>>
auto all_valid(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                fun(i);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
auto all_cardinality_sets(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N + 1)) {
                auto a = std::views::iota(0UZ, i) | std::ranges::to<X>(); assert(a.size() == i);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
auto all_singleton_arrays(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                auto a = std::array{ i }; assert(a.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
auto all_singleton_ilists(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                auto a = { i }; assert(a.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
auto all_singleton_sets(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                auto a = X({ i }); assert(a.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

}       // namespace on1

namespace on2 {

template<class X, std::size_t N = limit_v<X, L2>>
auto all_doubleton_arrays(auto fun)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = std::array{ i, j }; assert(a.size() == 2);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        fun(a);
                }
        }
}

template<class X, std::size_t N = limit_v<X, L2>>
auto all_doubleton_ilists(auto fun)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = { i, j }; assert(a.size() == 2);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        fun(a);
                }
        }
}

template<class X, std::size_t N = limit_v<X, L2>>
auto all_doubleton_sets(auto fun)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                for (auto const i : std::views::iota(0UZ, j)) {
                        auto a = X({ i, j }); assert(a.size() == 2);
                        fun(a);
                }
        }
}

template<class X, std::size_t N = limit_v<X, L2>>
auto all_singleton_set_pairs(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                for (auto const j : std::views::iota(0UZ, N)) {
                        auto a = X({ i }); assert(a.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        auto b = X({ j }); assert(b.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        fun(a, b);
                }
        }
}

}       // namespace on2

namespace on3 {

template<class X, std::size_t N = limit_v<X, L3>>
auto all_singleton_set_triples(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                for (auto const j : std::views::iota(0UZ, N)) {
                        for (auto const k : std::views::iota(0UZ, N)) {
                                auto a = X({ i }); assert(a.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                auto b = X({ j }); assert(b.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                auto c = X({ k }); assert(c.size() == 1);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                fun(a, b, c);
                        }
                }
        }
}

}       // namespace on3

namespace on4 {

template<class X, std::size_t N = limit_v<X, L4>>
auto all_doubleton_set_pairs(auto fun)
{
        for (auto const j : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                for (auto const n : std::views::iota(1UZ, std::ranges::max(N, 1UZ))) {
                        for (auto const i : std::views::iota(0UZ, j)) {
                                for (auto const m : std::views::iota(0UZ, n)) {
                                        auto a = X({ i, j }); assert(a.size() == 2);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                        auto b = X({ m, n }); assert(b.size() == 2);  // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                        fun(a, b);
                                }
                        }
                }
        }
}

}       // namespace on4

} // namespace test::set

#endif // TEST_SET_EXHAUSTIVE_HPP
