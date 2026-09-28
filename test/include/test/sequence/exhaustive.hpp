//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_EXHAUSTIVE_HPP
#define TEST_SEQUENCE_EXHAUSTIVE_HPP

#include <test/dynamic.hpp>          // dynamic
#include <test/sequence/factory.hpp> // limit_v, make_sequence, stripes
#include <cstddef>                   // size_t
#include <ranges>                    // iota

namespace test::sequence {

inline constexpr auto L0 = 128UZ;
inline constexpr auto L1 = 64UZ;
inline constexpr auto L2 = 32UZ;
inline constexpr auto L3 = 16UZ;

namespace on0 {

// Value-initialized: no positions at a run-time width, every position clear at a static one.
template<class X>
auto empty_sequence(auto fun)
{
        auto a = X(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        fun(a);
}

template<class X, std::size_t N = limit_v<X, L0>>
auto full_sequence(auto fun)
{
        auto a = make_sequence<X>(N, [](std::size_t) -> bool { return true; }); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        fun(a);
}

template<class X>
auto empty_sequence_pair(auto fun)
{
        auto a = X(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        auto b = X(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
        fun(a, b);
}

} // namespace on0

namespace on1 {

template<class X, std::size_t N = limit_v<X, L1>>
auto all_valid(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                fun(i);
        }
}

// Every width up to the limit where the width is the object's, and the type's own where it is not.
template<class X, std::size_t N = limit_v<X, L1>>
auto all_widths(auto fun)
{
        if constexpr (test::dynamic<X>) {
                for (auto const n : std::views::iota(0UZ, N + 1UZ)) {
                        auto a = make_sequence<X>(n, stripes); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        fun(a);
                }
        } else {
                auto a = make_sequence<X>(N, stripes); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

// The first i positions set, for every i from none to all.
template<class X, std::size_t N = limit_v<X, L1>>
auto all_prefix_sequences(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N + 1UZ)) {
                auto a = make_sequence<X>(N, [i](std::size_t j) -> bool { return j < i; }); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

template<class X, std::size_t N = limit_v<X, L1>>
auto all_singleton_sequences(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                auto a = make_sequence<X>(N, [i](std::size_t j) -> bool { return j == i; }); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        }
}

// Every sequence the linear sweeps build one at a time: each width, each prefix and each singleton.
template<class X>
auto all_sequences(auto fun)
{
        all_widths<X>(fun);
        all_prefix_sequences<X>(fun);
        all_singleton_sequences<X>(fun);
}

} // namespace on1

namespace on2 {

template<class X, std::size_t N = limit_v<X, L2>>
auto all_singleton_sequence_pairs(auto fun)
{
        for (auto const i : std::views::iota(0UZ, N)) {
                for (auto const j : std::views::iota(0UZ, N)) {
                        auto a = make_sequence<X>(N, [i](std::size_t k) -> bool { return k == i; }); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        auto b = make_sequence<X>(N, [j](std::size_t k) -> bool { return k == j; }); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                        fun(a, b);
                }
        }
}

// Two prefixes of one pattern, so either can be the other's prefix, at every pair of widths the objects can have.
template<class X, std::size_t N = limit_v<X, L2>>
auto all_width_pairs(auto fun)
{
        on1::all_widths<X, N>([&](auto const& a) {
                on1::all_widths<X, N>([&](auto const& b) {
                        auto x = a; // NOLINT(misc-const-correctness,performance-unnecessary-copy-initialization): handed to fun, which some functors take by non-const reference
                        auto y = b; // NOLINT(misc-const-correctness,performance-unnecessary-copy-initialization): handed to fun, which some functors take by non-const reference
                        fun(x, y);
                });
        });
}

// Every pair the quadratic sweeps build: two singletons at one width, and two widths of one pattern.
template<class X, std::size_t N = limit_v<X, L2>>
auto all_sequence_pairs(auto fun)
{
        all_singleton_sequence_pairs<X, N>(fun);
        all_width_pairs<X, N>(fun);
}

// Every width, and every position from the first to one past the last, which is where an insertion can go.
template<class X, std::size_t N = limit_v<X, L2>>
auto all_width_positions(auto fun)
{
        on1::all_widths<X, N>([&](auto const& a) {
                for (auto const p : std::views::iota(0UZ, a.size() + 1UZ)) {
                        fun(a, p);
                }
        });
}

} // namespace on2

namespace on3 {

// Every width, position and count, the count running to the limit whatever room the width leaves.
template<class X, std::size_t N = limit_v<X, L3>>
auto all_width_position_counts(auto fun)
{
        on2::all_width_positions<X, N>([&](auto const& a, std::size_t p) {
                for (auto const k : std::views::iota(0UZ, N + 1UZ)) {
                        fun(a, p, k);
                }
        });
}

} // namespace on3

} // namespace test::sequence

#endif // TEST_SEQUENCE_EXHAUSTIVE_HPP
