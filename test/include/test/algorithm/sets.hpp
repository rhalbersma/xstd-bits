//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_ALGORITHM_SETS_HPP
#define TEST_ALGORITHM_SETS_HPP

#ifdef _MSC_VER

// xstd::bit_fixed_set<0> gives bogus "unreachable code" warnings, ahead of the containers this header includes
#pragma warning(disable : 4702)

#endif

#include <test/sequence/factory.hpp>             // stripes
#include <test/set/exhaustive.hpp>               // limit_v, static_width
#include <test/spec/set.hpp>                     // dynamic, fixed, inplace, small, user_storage
#include <xstd/bits/bit_set/bit_fixed_set.hpp>   // basic_bit_fixed_set
#include <xstd/bits/bit_set/bit_key_mapping.hpp> // bit_key_mapping
#include <xstd/bits/bit_set/bit_set.hpp>         // basic_bit_set
#include <algorithm>                             // min
#include <array>                                 // array
#include <cstddef>                               // size_t
#include <cstdint>                               // uint64_t, uint8_t
#include <functional>                            // greater
#include <ranges>                                // iota
#include <set>                                   // set
#include <tuple>                                 // tuple, tuple_cat
#include <utility>                               // declval
#include <vector>                                // vector

// Sets of every column, keyed by patterns, for the algorithms to be checked against std::set over the same keys.
namespace test::algorithm {

// The descending column of ours, at a static width either side of a block boundary and at a run-time one.
using descending_sets = std::tuple<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 17, xstd::bit_key_mapping<std::size_t>, std::greater<>>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 65, xstd::bit_key_mapping<std::size_t>, std::greater<>>, xstd::basic_bit_set<std::size_t, std::vector<std::uint8_t>, std::dynamic_extent, xstd::bit_key_mapping<std::size_t>, std::greater<>>>;

// Every column of ours the set clauses sweep, their standard models left out, and the descending ones besides.
using sets = decltype(std::tuple_cat(std::declval<test::spec::set::fixed>(), std::declval<test::spec::set::dynamic>(), std::declval<test::spec::set::inplace>(), std::declval<test::spec::set::small>(), std::declval<test::spec::set::user_storage>(), std::declval<descending_sets>()));

// The widest run-time width an input takes, past two blocks of the widest builtin Block.
inline constexpr auto set_limit = 130UZ;

// The keys a set of X holds, as std::set orders them under X's comparator.
template<class X>
[[nodiscard]] auto model_of(X const& x)
        -> std::set<std::size_t, typename X::key_compare>
{
        return {x.begin(), x.end()};
}

// Every iterator of s from begin() to end(), stepped one at a time: what an iterator operation is checked against.
template<class X>
[[nodiscard]] auto iterators_of(X const& s)
        -> std::vector<typename X::const_iterator>
{
        auto nrv = std::vector<typename X::const_iterator>();
        for (auto it = s.begin(); it != s.end(); ++it) {
                nrv.push_back(it);
        }
        nrv.push_back(s.end());
        return nrv;
}

// The indices worth starting or stopping at among n + 1 iterators: both ends, their neighbours and the middle.
[[nodiscard]] inline auto stops(std::size_t n)
        -> std::vector<std::size_t>
{
        auto nrv = std::vector<std::size_t>();
        for (auto const i : {0UZ, 1UZ, 2UZ, n / 2UZ, n - 1UZ, n}) {
                if (i <= n) {
                        nrv.push_back(i);
                }
        }
        return nrv;
}

// The widths an input of X takes: its own where the type fixes it, else the edges of the narrow and wide blocks.
template<class X>
[[nodiscard]] constexpr auto set_widths()
        -> std::array<std::size_t, 6>
{
        constexpr auto limit = test::set::limit_v<X, set_limit>;
        auto nrv             = std::array{0UZ, 1UZ, 9UZ, 64UZ, 65UZ, 130UZ};
        for (auto& n : nrv) {
                n = test::set::static_width<X> ? limit : std::ranges::min(n, limit);
        }
        return nrv;
}

// The patterns a set holds below width n: empty and full, striped either way, one key alone or absent, and runs.
template<class F>
constexpr auto for_each_key_pattern(std::size_t n, F f)
        -> void
{
        f([](std::size_t) -> bool { return false; });
        f([](std::size_t) -> bool { return true; });
        f([](std::size_t i) -> bool { return test::sequence::stripes(i); });
        f([](std::size_t i) -> bool { return not test::sequence::stripes(i); });
        for (auto const p : {0UZ, n / 2UZ, n - 1UZ}) {
                f([p](std::size_t i) -> bool { return i == p; });
                f([p](std::size_t i) -> bool { return i != p; });
        }
        f([n](std::size_t i) -> bool { return n / 3UZ <= i; });
}

// A set of X holding the keys below n that pred names.
template<class X>
[[nodiscard]] auto make_set(std::size_t n, auto pred)
        -> X
{
        auto x = X();
        for (auto const i : std::views::iota(0UZ, n)) {
                if (pred(i)) {
                        x.insert(i);
                }
        }
        return x;
}

// f(x) for every input of X, each width once over a static width, under every pattern.
template<class X, class F>
auto for_each_set(F f)
        -> void
{
        constexpr auto ns       = set_widths<X>();
        constexpr auto distinct = test::set::static_width<X> ? 1UZ : ns.size();
        for (auto const k : std::views::iota(0UZ, distinct)) {
                for_each_key_pattern(ns[k], [&](auto pred) -> void { f(make_set<X>(ns[k], pred)); });
        }
}

} // namespace test::algorithm

#endif // TEST_ALGORITHM_SETS_HPP
