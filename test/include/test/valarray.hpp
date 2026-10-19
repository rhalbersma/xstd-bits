//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_VALARRAY_HPP
#define TEST_VALARRAY_HPP

#include <xstd/bits/bit/bit_convert.hpp>                   // bit_convert
#include <xstd/bits/bit_algorithm/bit_all_of.hpp>          // bit_all_of
#include <xstd/bits/bit_algorithm/bit_any_of.hpp>          // bit_any_of
#include <xstd/bits/bit_algorithm/bit_count.hpp>           // bit_count
#include <xstd/bits/bit_algorithm/bit_mismatch.hpp>        // bit_mismatch
#include <xstd/bits/bit_algorithm/bit_none_of.hpp>         // bit_none_of
#include <xstd/bits/bit_algorithm/bit_reverse.hpp>         // bit_reverse
#include <xstd/bits/bit_algorithm/bit_rotate.hpp>          // bit_rotate
#include <xstd/bits/bit_ranges/from_blocks.hpp>            // from_blocks, from_blocks_t
#include <xstd/bits/bit_span/bit_span.hpp>                 // bit_span
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>   // basic_bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_gslice.hpp>           // bit_gslice
#include <xstd/bits/bit_valarray/bit_inplace_valarray.hpp> // basic_bit_inplace_valarray
#include <xstd/bits/bit_valarray/bit_slice.hpp>            // bit_slice
#include <xstd/bits/bit_valarray/bit_valarray.hpp>         // basic_bit_valarray
#include <xstd/bits/bit_vector.hpp>                        // bit_vector
#include <xstd/ints/limits.hpp>                            // numeric_limits
#include <boost/test/unit_test.hpp>                        // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_REQUIRE_EQUAL, BOOST_TEST_CONTEXT
#include <algorithm>                                       // adjacent_find, all_of, count, equal, min, mismatch, reverse, sort
#include <array>                                           // array
#include <bitset>                                          // bitset
#include <climits>                                         // INT_MAX, INT_MIN
#include <concepts>                                        // constructible_from, convertible_to, copyable, equality_comparable, same_as
#include <cstddef>                                         // ptrdiff_t, size_t
#include <cstdint>                                         // uint64_t
#include <format>                                          // format
#include <functional>                                      // hash
#include <iterator>                                        // contiguous_iterator
#include <ranges>                                          // begin, contiguous_range, end, iota, iterator_t, random_access_range, range_value_t, size, sized_range, ssize, to
#include <span>                                            // dynamic_extent
#include <type_traits>                                     // is_default_constructible_v
#include <utility>                                         // declval, move, to_underlying
#include <vector>                                          // vector

// Every member and operator of the four valarrays, each held to the same operation on a std::vector<bool>.
namespace test::valarray {

using model = std::vector<bool>;

// The size a type fixes, or its capacity, or neither.
template<class V>
inline constexpr auto fixed_size = std::dynamic_extent;

template<class Block, std::size_t N>
inline constexpr auto fixed_size<xstd::basic_bit_fixed_valarray<Block, N>> = N;

template<class V>
inline constexpr auto capacity = std::dynamic_extent;

template<class Block, std::size_t N>
inline constexpr auto capacity<xstd::basic_bit_inplace_valarray<Block, N>> = N;

template<class V>
inline constexpr bool is_fixed = fixed_size<V> != std::dynamic_extent;

// Sizes across block boundaries: the type's own where it fixes one, else up to its capacity or a few blocks.
template<class V>
[[nodiscard]] auto sizes()
        -> std::vector<std::size_t>
{
        if constexpr (is_fixed<V>) {
                return {fixed_size<V>};
        } else {
                auto result = std::vector<std::size_t>();
                for (auto const n : {0UZ, 1UZ, 2UZ, 7UZ, 8UZ, 9UZ, 17UZ, 63UZ, 64UZ, 65UZ, 127UZ, 128UZ, 129UZ, 200UZ}) {
                        if (n <= capacity<V>) {
                                result.push_back(n);
                        }
                }
                if (capacity<V> != std::dynamic_extent and (result.empty() or result.back() != capacity<V>)) {
                        result.push_back(capacity<V>);
                }
                return result;
        }
}

// Clear, full, alternating either way, two scrambles, and the last bit alone either way, at every size.
[[nodiscard]] inline auto patterns(std::size_t n)
        -> std::vector<model>
{
        auto result = std::vector<model>(8UZ, model(n));
        auto state  = std::uint64_t{0x9E37'79B9'7F4A'7C15};
        for (auto const i : std::views::iota(0UZ, n)) {
                state ^= state << 13U;
                state ^= state >> 7U;
                state ^= state << 17U;
                result[1][i] = true;
                result[2][i] = i % 2UZ == 0UZ;
                result[3][i] = i % 2UZ == 1UZ;
                result[4][i] = (state & 1U) != 0U;
                result[5][i] = (state & 6U) == 6U;
                result[6][i] = i + 1UZ == n;
                result[7][i] = i + 1UZ != n;
        }
        return result;
}

template<class V>
[[nodiscard]] auto model_of(V const& v)
        -> model
{
        auto result = model();
        for (auto const i : std::views::iota(0UZ, v.size())) {
                result.push_back(v[i]);
        }
        return result;
}

// The array holding the model's bools, written through the subscript's proxy.
template<class V>
[[nodiscard]] auto make(model const& m)
        -> V
{
        auto v = V();
        if constexpr (not is_fixed<V>) {
                v = V(m.size());
        }
        for (auto const i : std::views::iota(0UZ, m.size())) {
                v[i] = m[i];
        }
        return v;
}

// Equal elements, and no bit set past them: a count over whole blocks would see one in the padding.
template<class V>
auto check_equal(V const& v, model const& m)
        -> void
{
        BOOST_CHECK_EQUAL(v.size(), m.size());
        BOOST_CHECK(model_of(v) == m);
        BOOST_CHECK_EQUAL(xstd::bit_count(v), std::ranges::count(m, true));
}

template<class F>
[[nodiscard]] auto elementwise(model const& a, model const& b, F f)
        -> model
{
        auto result = model(a.size());
        for (auto const i : std::views::iota(0UZ, a.size())) {
                result[i] = f(a[i], b[i]);
        }
        return result;
}

// Each pair of arrays of one size, with each pair of its patterns.
template<class V, class F>
auto for_each_pair(F f)
        -> void
{
        for (auto const n : sizes<V>()) {
                auto const ps = patterns(n);
                for (auto const& a : ps) {
                        for (auto const& b : ps) {
                                BOOST_TEST_CONTEXT("size " << n)
                                {
                                        f(a, b);
                                }
                        }
                }
        }
}

template<class V, class F>
auto for_each_pattern(F f)
        -> void
{
        for (auto const n : sizes<V>()) {
                for (auto const& a : patterns(n)) {
                        BOOST_TEST_CONTEXT("size " << n)
                        {
                                f(a);
                        }
                }
        }
}

// Not [valarray]'s by intent: the type is std::valarray<bool>'s shape, not a container's or a sequence's.
template<class V>
auto check_type()
        -> void
{
        static_assert(std::same_as<typename V::value_type, bool>);
        static_assert(std::same_as<typename V::size_type, std::size_t>);
        static_assert(std::copyable<V>);
        static_assert(std::is_default_constructible_v<V>);
        static_assert(std::ranges::random_access_range<V>);
        static_assert(std::ranges::random_access_range<V const>);
        static_assert(std::ranges::sized_range<V>);
        static_assert(not std::ranges::contiguous_range<V>);
        static_assert(not std::contiguous_iterator<std::ranges::iterator_t<V>>);
        static_assert(not std::equality_comparable<V>);
        static_assert(not requires (V const& v) { std::hash<V>()(v); });
        static_assert(not requires (V& v) { v.begin(); });
        static_assert(not requires (V& v) { v.count(); });
        static_assert(not requires (V& v) { v.flip(); });
        static_assert(not requires (V& v) { v.all(); });
        static_assert(not requires (V& v) { v.any(); });
        static_assert(not requires (V& v) { v.none(); });
        static_assert(not requires (V& v) { +v; });
        static_assert(not requires (V& v) { -v; });
        static_assert(not requires (V& v) { v + v; });
        static_assert(not requires (V& v) { v += v; });
        static_assert(not requires (V& v) { v <=> v; });
        static_assert(std::same_as<decltype(std::declval<V&>()[0UZ]), typename V::reference>);
        static_assert(std::same_as<decltype(std::declval<V const&>()[0UZ]), bool>);
        static_assert(std::same_as<decltype(std::declval<V const&>() == std::declval<V const&>()), V>);
        static_assert(std::same_as<decltype(std::declval<V const&>() < true), V>);
        static_assert(std::same_as<decltype(false >= std::declval<V const&>()), V>);
        static_assert(std::same_as<decltype(not std::declval<V const&>()), V>);
        static_assert(std::same_as<decltype(std::declval<V const&>().sum()), bool>);
        static_assert(noexcept(std::declval<V&>().swap(std::declval<V&>())));
        static_assert(noexcept(swap(std::declval<V&>(), std::declval<V&>())));
}

// [valarray.cons]: by default empty, or the type's size of false.
template<class V>
auto check_default()
        -> void
{
        check_equal(V(), model(is_fixed<V> ? fixed_size<V> : 0UZ));
}

// [valarray.cons]: a count of false, and a value repeated a count of times, the value first.
template<class V>
auto check_count()
        -> void
{
        if constexpr (not is_fixed<V>) {
                static_assert(not std::convertible_to<std::size_t, V>);
                for (auto const n : sizes<V>()) {
                        check_equal(V(n), model(n));
                        check_equal(V(false, n), model(n, false));
                        check_equal(V(true, n), model(n, true));
                }
        }
}

// [valarray.cons]: a list of bools, which at a fixed size leads and leaves the rest false.
template<class V>
auto check_list()
        -> void
{
        constexpr auto room = is_fixed<V> ? fixed_size<V> : std::ranges::min(3UZ, capacity<V>);
        if constexpr (room == 0UZ) {
                check_equal(V({}), model());
        } else if constexpr (room < 3UZ) {
                auto m = model(is_fixed<V> ? fixed_size<V> : 1UZ);
                m[0]   = true;
                check_equal(V({true}), m);
        } else {
                auto m = model(is_fixed<V> ? fixed_size<V> : 3UZ);
                m[0]   = true;
                m[2]   = true;
                check_equal(V({true, false, true}), m);
                auto v = V();
                v      = {true, false, true};
                check_equal(v, m);
        }
}

// Not in [valarray]: blocks as positions, the lowest bit of the first block element zero.
template<class V>
auto check_from_blocks()
        -> void
{
        using blocks_type     = V::block_container_type;
        using block_type      = std::ranges::range_value_t<blocks_type>;
        constexpr auto digits = static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits);
        if constexpr (std::constructible_from<V, xstd::from_blocks_t, blocks_type>) {
                check_equal(V(xstd::from_blocks, blocks_type()), model(is_fixed<V> ? fixed_size<V> : 0UZ));
                if constexpr (is_fixed<V> and fixed_size<V> != 0UZ) {
                        auto blocks = blocks_type();
                        blocks[0]   = block_type{1};
                        auto m      = model(fixed_size<V>);
                        m[0]        = true;
                        check_equal(V(xstd::from_blocks, blocks), m);
                } else if constexpr (not is_fixed<V> and capacity<V> >= digits) {
                        auto blocks = blocks_type();
                        blocks.push_back(block_type{1});
                        auto m = model(digits);
                        m[0]   = true;
                        check_equal(V(xstd::from_blocks, std::move(blocks)), m);
                }
        }
}

// [valarray.assign]: copies and moves take the other's size; a scalar fills, keeping the size.
template<class V>
auto check_assign()
        -> void
{
        for_each_pair<V>([](model const& a, model const& b) -> void {
                auto x       = make<V>(a);
                auto const y = make<V>(b);
                x            = y;
                check_equal(x, b);
                auto z = make<V>(a);
                x      = std::move(z);
                check_equal(x, a);
                x = true;
                check_equal(x, model(a.size(), true));
                x = false;
                check_equal(x, model(a.size(), false));
        });
}

// [valarray.access]: the proxy writes one element, and the const subscript reads it.
template<class V>
auto check_subscript()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto v = make<V>(a);
                auto m = a;
                for (auto const i : std::views::iota(0UZ, a.size())) {
                        v[i] = not v[i];
                        m[i] = not m[i];
                        BOOST_CHECK_EQUAL(static_cast<V const&>(v)[i], m[i]);
                }
                check_equal(v, m);
        });
}

// [valarray.members]: sum is or, min is and, max is or, each defined on an empty array.
template<class V>
auto check_reductions()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto const v   = make<V>(a);
                auto const any = std::ranges::count(a, true) != 0;
                auto const all = std::ranges::count(a, false) == 0;
                BOOST_CHECK_EQUAL(v.sum(), any);
                BOOST_CHECK_EQUAL(v.min(), all);
                BOOST_CHECK_EQUAL(v.max(), any);
        });
}

[[nodiscard]] inline auto shifted(model const& a, long long n)
        -> model
{
        auto result  = model(a.size());
        auto const s = static_cast<long long>(a.size());
        for (auto const i : std::views::iota(0LL, s)) {
                if (i + n >= 0 and i + n < s) {
                        result[static_cast<std::size_t>(i)] = a[static_cast<std::size_t>(i + n)];
                }
        }
        return result;
}

[[nodiscard]] inline auto rotated(model const& a, long long n)
        -> model
{
        auto result  = model(a.size());
        auto const s = static_cast<long long>(a.size());
        for (auto const i : std::views::iota(0LL, s)) {
                result[static_cast<std::size_t>(i)] = a[static_cast<std::size_t>((((i + n) % s) + s) % s)];
        }
        return result;
}

// [valarray.members]: shift fills with false and cshift wraps round, element i taking element i + n.
template<class V>
auto check_shift()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto const v = make<V>(a);
                auto const s = static_cast<int>(a.size());
                for (auto const n : std::views::iota(-s - 2, s + 3)) {
                        BOOST_TEST_CONTEXT("n " << n)
                        {
                                check_equal(v.shift(n), shifted(a, n));
                                check_equal(v.cshift(n), rotated(a, n));
                        }
                }
                for (auto const n : {INT_MIN, INT_MIN + 1, INT_MAX}) {
                        check_equal(v.shift(n), model(a.size()));
                        check_equal(v.cshift(n), rotated(a, n));
                }
        });
}

[[nodiscard]] inline auto identity(bool x)
        -> bool
{
        return x;
}

[[nodiscard]] inline auto negation(bool const& x)
        -> bool
{
        return not x;
}

// [valarray.members]: apply, by value and by reference, through each of the four functions of one bool.
template<class V>
auto check_apply()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto const v = make<V>(a);
                auto m       = a;
                m.flip();
                check_equal(v.apply(identity), a);
                check_equal(v.apply(negation), m);
                check_equal(v.apply([](bool) -> bool { return true; }), model(a.size(), true));
                check_equal(v.apply([](bool const&) -> bool { return false; }), model(a.size(), false));
        });
}

// [valarray.members]: a run-time size resized holds the value at every element, the old ones included.
template<class V>
auto check_resize()
        -> void
{
        if constexpr (not is_fixed<V>) {
                for_each_pattern<V>([](model const& a) -> void {
                        for (auto const n : sizes<V>()) {
                                auto v = make<V>(a);
                                v.resize(n);
                                check_equal(v, model(n));
                                v = make<V>(a);
                                v.resize(n, true);
                                check_equal(v, model(n, true));
                        }
                });
        }
}

// [valarray.members] and [valarray.special]: both swaps exchange values and sizes.
template<class V>
auto check_swap()
        -> void
{
        for_each_pair<V>([](model const& a, model const& b) -> void {
                auto x = make<V>(a);
                auto y = make<V>(b);
                x.swap(y);
                check_equal(x, b);
                check_equal(y, a);
                swap(x, y);
                check_equal(x, a);
                check_equal(y, b);
        });
}

// [valarray.unary] at bool: ! and ~ both negate every element, where valarray<bool>'s ~ promotes.
template<class V>
auto check_unary()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto const v = make<V>(a);
                auto m       = a;
                m.flip();
                check_equal(not v, m);
                check_equal(~v, m);
                check_equal(~~v, a);
        });
}

// [valarray.cassign]'s logical three, over arrays of one size and over a scalar.
template<class V>
auto check_compound()
        -> void
{
        for_each_pair<V>([](model const& a, model const& b) -> void {
                auto const y = make<V>(b);
                auto x       = make<V>(a);
                check_equal(x &= y, elementwise(a, b, [](bool p, bool q) -> bool { return p and q; }));
                x = make<V>(a);
                check_equal(x |= y, elementwise(a, b, [](bool p, bool q) -> bool { return p or q; }));
                x = make<V>(a);
                check_equal(x ^= y, elementwise(a, b, [](bool p, bool q) -> bool { return p != q; }));
        });
        for_each_pattern<V>([](model const& a) -> void {
                for (auto const t : {false, true}) {
                        auto const b = model(a.size(), t);
                        auto x       = make<V>(a);
                        check_equal(x &= t, elementwise(a, b, [](bool p, bool q) -> bool { return p and q; }));
                        x = make<V>(a);
                        check_equal(x |= t, elementwise(a, b, [](bool p, bool q) -> bool { return p or q; }));
                        x = make<V>(a);
                        check_equal(x ^= t, elementwise(a, b, [](bool p, bool q) -> bool { return p != q; }));
                }
        });
}

// Not [valarray.cassign]'s: std::bitset's shifts, << moving element i to i + n and >> moving i + n to i.
template<class V>
auto check_bitset_shifts()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto const v = make<V>(a);
                for (auto const n : std::views::iota(0UZ, a.size() + 3UZ)) {
                        BOOST_TEST_CONTEXT("n " << n)
                        {
                                auto const s = static_cast<long long>(n);
                                check_equal(v << n, shifted(a, -s));
                                check_equal(v >> n, shifted(a, s));
                                auto x = v;
                                check_equal(x <<= n, shifted(a, -s));
                                x = v;
                                check_equal(x >>= n, shifted(a, s));
                        }
                }
                check_equal(v << static_cast<std::size_t>(-1), model(a.size()));
                check_equal(v >> static_cast<std::size_t>(-1), model(a.size()));
        });
}

// One binary operator against its model, over two arrays and over each scalar on either side.
template<class V, class Op, class F>
auto check_binary(model const& a, model const& b, Op op, F f)
        -> void
{
        auto const x = make<V>(a);
        auto const y = make<V>(b);
        check_equal(op(x, y), elementwise(a, b, f));
        for (auto const t : {false, true}) {
                auto const c = model(a.size(), t);
                check_equal(op(x, t), elementwise(a, c, f));
                check_equal(op(t, y), elementwise(c, b, f));
        }
}

// [valarray.binary]'s logical five, the operands both evaluated, and && and || the same as & and |.
template<class V>
auto check_logical()
        -> void
{
        for_each_pair<V>([](model const& a, model const& b) -> void {
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x & y; }, [](bool p, bool q) -> bool { return p and q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x | y; }, [](bool p, bool q) -> bool { return p or q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x ^ y; }, [](bool p, bool q) -> bool { return p != q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x and y; }, [](bool p, bool q) -> bool { return p and q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x or y; }, [](bool p, bool q) -> bool { return p or q; });
        });
}

// [valarray.comparison]'s six, each an array of answers rather than one.
template<class V>
auto check_comparison()
        -> void
{
        for_each_pair<V>([](model const& a, model const& b) -> void {
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x == y; }, [](bool p, bool q) -> bool { return p == q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x != y; }, [](bool p, bool q) -> bool { return p != q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x < y; }, [](bool p, bool q) -> bool { return p < q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x > y; }, [](bool p, bool q) -> bool { return p > q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x <= y; }, [](bool p, bool q) -> bool { return p <= q; });
                check_binary<V>(a, b, [](auto const& x, auto const& y) { return x >= y; }, [](bool p, bool q) -> bool { return p >= q; });
        });
}

// [valarray.range]: begin and end as non-members, which range-for and std::ranges find.
template<class V>
auto check_range()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto v = make<V>(a);
                BOOST_CHECK(std::ranges::equal(v, a));
                BOOST_CHECK(std::ranges::equal(static_cast<V const&>(v), a));
                BOOST_CHECK_EQUAL(std::ranges::size(v), a.size());
                BOOST_CHECK(std::ranges::end(v) - std::ranges::begin(v) == static_cast<std::ptrdiff_t>(a.size()));
                for (auto&& e : v) {
                        e = true;
                }
                check_equal(v, model(a.size(), true));
                auto n = 0UZ;
                for (auto const e : static_cast<V const&>(v)) {
                        n += e ? 1UZ : 0UZ;
                }
                BOOST_CHECK_EQUAL(n, a.size());
        });
}

// The free algorithms read and write through the storage, as over the sequences.
template<class V>
auto check_algorithms()
        -> void
{
        for_each_pair<V>([](model const& a, model const& b) -> void {
                auto const n = std::ranges::count(a, true);
                auto v       = make<V>(a);
                BOOST_CHECK_EQUAL(xstd::bit_count(v), n);
                BOOST_CHECK_EQUAL(xstd::bit_all_of(v), n == std::ranges::ssize(a));
                BOOST_CHECK_EQUAL(xstd::bit_any_of(v), n != 0);
                BOOST_CHECK_EQUAL(xstd::bit_none_of(v), n == 0);
                auto const w = make<V>(b);
                BOOST_CHECK(xstd::bit_mismatch(v, w).in1 - std::ranges::begin(v) == std::ranges::mismatch(a, b).in1 - a.begin());
                auto m = a;
                std::ranges::reverse(m);
                xstd::bit_reverse(v);
                check_equal(v, m);
                if (not a.empty()) {
                        xstd::bit_rotate(v, std::ranges::begin(v) + 1);
                        check_equal(v, rotated(m, 1));
                }
        });
}

// bit_convert keeps every position, to and from a sequence, and a view reads and writes the array's elements.
template<class V>
auto check_interop()
        -> void
{
        for_each_pattern<V>([](model const& a) -> void {
                auto v = make<V>(a);
                check_equal(xstd::bit_convert<xstd::bit_vector>(v), a);
                check_equal(xstd::bit_convert<V>(xstd::bit_convert<xstd::bit_vector>(v)), a);
                if constexpr (is_fixed<V>) {
                        auto const b = xstd::bit_convert<std::bitset<fixed_size<V>>>(v);
                        for (auto const i : std::views::iota(0UZ, a.size())) {
                                BOOST_CHECK_EQUAL(b[i], a[i]);
                        }
                        check_equal(xstd::bit_convert<V>(b), a);
                }
                BOOST_CHECK_EQUAL(std::format("{}", v), std::format("{}", a));
                auto view = xstd::bit_span(v);
                BOOST_CHECK(std::ranges::equal(view, a));
                if (not a.empty()) {
                        view[0] = not a[0];
                        BOOST_CHECK_EQUAL(static_cast<V const&>(v)[0], not a[0]);
                }
        });
}

// Fewer sizes than the operators run at, each across a block boundary of the narrow blocks, and the capacity.
template<class V>
[[nodiscard]] auto subscript_sizes()
        -> std::vector<std::size_t>
{
        if constexpr (is_fixed<V>) {
                return {fixed_size<V>};
        } else {
                auto result = std::vector<std::size_t>();
                for (auto const n : {0UZ, 1UZ, 9UZ, 65UZ, 130UZ}) {
                        if (n <= capacity<V>) {
                                result.push_back(n);
                        }
                }
                if (capacity<V> != std::dynamic_extent and result.back() != capacity<V>) {
                        result.push_back(capacity<V>);
                }
                return result;
        }
}

// The arrays a subscript reads and writes: alternating and scrambled, the other patterns being the masks' to vary.
template<class V, class F>
auto for_each_subscripted(F f)
        -> void
{
        for (auto const n : subscript_sizes<V>()) {
                auto const ps = patterns(n);
                for (auto const& a : {ps[2], ps[4]}) {
                        BOOST_TEST_CONTEXT("size " << n)
                        {
                                f(a);
                        }
                }
        }
}

// The standard type as the reference column: what std::valarray<bool> does with the same elements, ours must do too.
using reference_array = std::valarray<bool>;

[[nodiscard]] inline auto reference_of(model const& m)
        -> reference_array
{
        auto result = reference_array(m.size());
        for (auto const i : std::views::iota(0UZ, m.size())) {
                result[i] = m[i];
        }
        return result;
}

[[nodiscard]] inline auto model_of(reference_array const& r)
        -> model
{
        return r | std::ranges::to<model>();
}

template<class V>
using block_t = std::ranges::range_value_t<typename V::block_container_type>;

// The array a proxy is handed: any owner of the same blocks, here the one on the heap, which takes any size.
template<class V>
using operand_t = xstd::basic_bit_valarray<block_t<V>>;

// What a const subscript gathers into: an owner of run-time size its own type, the fixed one the inplace one.
template<class V>
struct gathered
{
        using type = V;
};

template<class Block, std::size_t N>
struct gathered<xstd::basic_bit_fixed_valarray<Block, N>>
{
        using type = xstd::basic_bit_inplace_valarray<Block, N>;
};

template<class V>
using gathered_t = gathered<V>::type;

// How many elements a gather holds: the fixed size or the capacity, past which the inplace owner throws.
template<class V>
inline constexpr auto gather_capacity = is_fixed<V> ? fixed_size<V> : capacity<V>;

enum class write : unsigned char
{
        assign,
        and_assign,
        or_assign,
        xor_assign,
};

inline constexpr auto writes = std::array{write::assign, write::and_assign, write::or_assign, write::xor_assign};

// One write through a proxy, ours or std::valarray<bool>'s, from an array or a scalar alike.
template<class Proxy, class A>
auto apply(write w, Proxy&& p, A const& a)
        -> void
{
        if (w == write::assign) {
                p = a;
        } else if (w == write::and_assign) {
                p &= a;
        } else if (w == write::or_assign) {
                p |= a;
        } else {
                p ^= a;
        }
}

// The write itself, position by position and in order, which is what a position named twice takes.
[[nodiscard]] inline auto written(write w, model a, std::vector<std::size_t> const& positions, model const& b)
        -> model
{
        for (auto const j : std::views::iota(0UZ, positions.size())) {
                auto const pos = positions[j];
                if (w == write::assign) {
                        a[pos] = b[j];
                } else if (w == write::and_assign) {
                        a[pos] = a[pos] and b[j];
                } else if (w == write::or_assign) {
                        a[pos] = a[pos] or b[j];
                } else {
                        a[pos] = a[pos] != b[j];
                }
        }
        return a;
}

[[nodiscard]] inline auto distinct(std::vector<std::size_t> positions)
        -> bool
{
        std::ranges::sort(positions);
        return std::ranges::adjacent_find(positions) == positions.end();
}

// std::valarray<bool> answers at distinct positions, and not at a stride of zero, whose writes libstdc++ skips.
enum class reference : bool
{
        defined,
        undefined,
};

// Every write a proxy takes, from arrays, a proxy and scalars, against std::valarray<bool> where it answers.
template<class V, class Select, class SelectReference>
auto check_writes(model const& a, std::vector<std::size_t> const& positions, Select select, SelectReference select_reference, reference column = reference::defined)
        -> void
{
        auto const count = positions.size();
        auto const ps    = patterns(count);
        for (auto const w : writes) {
                BOOST_TEST_CONTEXT("write " << static_cast<int>(std::to_underlying(w)))
                {
                        for (auto const& b : {ps[4], model(count, true)}) {
                                auto const expected = written(w, a, positions, b);
                                if (column == reference::defined and distinct(positions)) {
                                        auto reference = reference_of(a);
                                        apply(w, select_reference(reference), reference_of(b));
                                        BOOST_CHECK(model_of(reference) == expected);
                                }

                                auto x = make<V>(a);
                                apply(w, select(x), make<operand_t<V>>(b));
                                check_equal(x, expected);

                                // From a proxy, which gathers what it selects before any position is written.
                                x      = make<V>(a);
                                auto y = make<operand_t<V>>(b);
                                apply(w, select(x), y[std::slice(0UZ, count, 1UZ)]);
                                check_equal(x, expected);
                        }
                        for (auto const t : {false, true}) {
                                auto x = make<V>(a);
                                apply(w, select(x), t);
                                check_equal(x, written(w, a, positions, model(count, t)));
                        }
                }
        }
}

// The const subscript's gather, the non-const one's proxy read into an owner by construction and by assignment.
template<class V, class Selector, class ReferenceSelector = Selector>
auto check_reads(model const& a, Selector const& selector, ReferenceSelector const& reference_selector)
        -> void
{
        auto const reference = reference_of(a);
        auto const expected  = model_of(reference[reference_selector]);
        auto const v         = make<V>(a);
        static_assert(std::same_as<decltype(v[selector]), gathered_t<V>>);
        check_equal(v[selector], expected);
        auto x = make<V>(a);
        if (not is_fixed<V> or expected.size() == fixed_size<V>) {
                auto const w = V(x[selector]);
                check_equal(w, expected);
                auto z = make<V>(a);
                z      = x[selector];
                check_equal(z, expected);
                // Gathered before it is assigned, so the array may be read from itself.
                x = x[selector];
                check_equal(x, expected);
        }
        check_equal(gathered_t<V>(make<V>(a)[selector]), expected);
}

// Slices of every stride class at starts across block boundaries: the strides below a block, one, none, and past one.
[[nodiscard]] inline auto slices(std::size_t n)
        -> std::vector<std::slice>
{
        auto result = std::vector<std::slice>{std::slice(0UZ, 0UZ, 1UZ), std::slice(n, 0UZ, 3UZ)};
        for (auto const start : {0UZ, 1UZ, 7UZ, 64UZ, 65UZ}) {
                if (start >= n) {
                        continue;
                }
                for (auto const stride : {0UZ, 1UZ, 2UZ, 3UZ, 8UZ, 64UZ, 65UZ, 129UZ}) {
                        auto const most = stride == 0UZ ? std::ranges::min(3UZ, n) : ((n - 1UZ - start) / stride) + 1UZ;
                        for (auto const size : {1UZ, most}) {
                                result.emplace_back(start, size, stride);
                        }
                }
        }
        return result;
}

[[nodiscard]] inline auto slice_indices(std::slice const& s)
        -> std::vector<std::size_t>
{
        auto result = std::vector<std::size_t>();
        for (auto const j : std::views::iota(0UZ, s.size())) {
                result.push_back(s.start() + (j * s.stride()));
        }
        return result;
}

[[nodiscard]] inline auto mask_indices(model const& m)
        -> std::vector<std::size_t>
{
        auto result = std::vector<std::size_t>();
        for (auto const i : std::views::iota(0UZ, m.size())) {
                if (m[i]) {
                        result.push_back(i);
                }
        }
        return result;
}

template<class F>
auto for_each_slice(std::size_t n, F f)
        -> void
{
        for (auto const& s : slices(n)) {
                BOOST_TEST_CONTEXT("slice(" << s.start() << ", " << s.size() << ", " << s.stride() << ")")
                {
                        f(s);
                }
        }
}

// [valarray.sub]/2-3 and [template.slice.array]: a slice's elements gathered, and written through its proxy.
template<class V>
auto check_slice()
        -> void
{
        for_each_subscripted<V>([](model const& a) -> void {
                for_each_slice(a.size(), [&](std::slice const& s) -> void {
                        check_reads<V>(a, s, s);
                        auto const bs = xstd::bit_slice(s);
                        check_reads<V>(a, bs, s);
                        auto const column = s.stride() == 0UZ ? reference::undefined : reference::defined;
                        check_writes<V>(a, slice_indices(s), [&](V& x) -> auto { return x[bs]; }, [&](reference_array& r) -> auto { return r[s]; }, column);
                });
        });
}

// The positions a gslice selects, computed as [class.gslice] says, to tell which ones a width can hold.
[[nodiscard]] inline auto gslice_indices(std::gslice const& g)
        -> std::vector<std::size_t>
{
        auto const lengths = g.size();
        auto const strides = g.stride();
        auto result        = std::vector<std::size_t>();
        BOOST_REQUIRE_EQUAL(strides.size(), lengths.size());
        if (lengths.size() == 0UZ) {
                return result;
        }
        auto counter = std::vector<std::size_t>(lengths.size());
        auto count   = 1UZ;
        for (auto const length : lengths) { // NOLINT(clang-analyzer-core.uninitialized.Assign): the analyzer cannot follow libstdc++'s valarray copy.
                count *= length;
        }
        for ([[maybe_unused]] auto const t : std::views::iota(0UZ, count)) {
                auto pos = g.start();
                for (auto const d : std::views::iota(0UZ, lengths.size())) {
                        pos += counter[d] * strides[d];
                }
                result.push_back(pos);
                for (auto d = lengths.size() - 1UZ; d < lengths.size(); --d) {
                        if (++counter[d] < lengths[d]) {
                                break;
                        }
                        counter[d] = 0UZ;
                }
        }
        return result;
}

[[nodiscard]] inline auto gslice_of(std::size_t start, std::vector<std::size_t> const& lengths, std::vector<std::size_t> const& strides)
        -> std::gslice
{
        return {start, std::valarray<std::size_t>(lengths.data(), lengths.size()), std::valarray<std::size_t>(strides.data(), strides.size())};
}

// Ascending ones, which take the mask path, and ones that repeat or turn back, which take the index path; none at all.
[[nodiscard]] inline auto gslices(std::size_t n)
        -> std::vector<std::gslice>
{
        auto const candidates = std::vector<std::gslice>{
                std::gslice(),
                gslice_of(0UZ, {0UZ}, {1UZ}),
                gslice_of(0UZ, {2UZ, 0UZ}, {1UZ, 1UZ}),
                gslice_of(1UZ, {5UZ}, {3UZ}),
                gslice_of(0UZ, {3UZ, 4UZ}, {9UZ, 2UZ}),
                gslice_of(5UZ, {2UZ, 3UZ, 4UZ}, {64UZ, 17UZ, 3UZ}),
                gslice_of(2UZ, {4UZ, 8UZ}, {32UZ, 1UZ}),
                gslice_of(0UZ, {2UZ, 3UZ}, {1UZ, 2UZ}),
                gslice_of(3UZ, {2UZ, 2UZ}, {0UZ, 1UZ}),
                gslice_of(0UZ, {3UZ, 3UZ}, {1UZ, 1UZ}),
                gslice_of(n / 2UZ, {n - (n / 2UZ)}, {1UZ}),
                gslice_of(0UZ, {n}, {1UZ}),
                gslice_of(n - 1UZ, {n}, {0UZ}),
                gslice_of(0UZ, {(n + 7UZ) / 8UZ, 8UZ}, {8UZ, 1UZ}),
        };
        auto result = std::vector<std::gslice>();
        for (auto const& g : candidates) {
                auto const indices = gslice_indices(g);
                if (std::ranges::all_of(indices, [&](std::size_t i) -> bool { return i < n; }) and indices.size() <= n) {
                        result.push_back(g);
                }
        }
        return result;
}

// [valarray.sub]/4-5 and [template.gslice.array]: a gslice's elements gathered, and written through its proxy.
template<class V>
auto check_gslice()
        -> void
{
        for_each_subscripted<V>([](model const& a) -> void {
                for (auto const& g : gslices(a.size())) {
                        BOOST_TEST_CONTEXT("gslice from " << g.start() << " of " << gslice_indices(g).size())
                        {
                                check_reads<V>(a, g, g);
                                auto const bg = xstd::bit_gslice(g);
                                check_reads<V>(a, bg, g);
                                check_writes<V>(a, gslice_indices(g), [&](V& x) -> auto { return x[bg]; }, [&](reference_array& r) -> auto { return r[g]; });
                        }
                }
        });
}

// [valarray.sub]/6-7 and [template.mask.array]: a mask's elements gathered, and written through its proxy.
template<class V, class Mask>
auto check_mask_with()
        -> void
{
        for_each_subscripted<V>([](model const& a) -> void {
                auto const ps = patterns(a.size());
                // Clear, full, odd positions, a sparse scramble and the last alone, some blocks selecting nothing.
                for (auto const& m : {ps[0], ps[1], ps[3], ps[5], ps[6]}) {
                        auto const mask      = make<Mask>(m);
                        auto const reference = reference_of(m);
                        auto const count     = static_cast<std::size_t>(std::ranges::count(m, true));
                        auto const elements  = reference_of(a);
                        auto const expected  = model_of(elements[reference]);
                        auto const v         = make<V>(a);
                        static_assert(std::same_as<decltype(v[mask]), gathered_t<V>>);
                        check_equal(v[mask], expected);
                        auto x = make<V>(a);
                        if (not is_fixed<V> or count == fixed_size<V>) {
                                check_equal(V(x[mask]), expected);
                                x = x[mask];
                                check_equal(x, expected);
                        }
                        check_equal(gathered_t<V>(make<V>(a)[mask]), expected);
                        check_writes<V>(a, mask_indices(m), [&](V& y) -> auto { return y[mask]; }, [&](reference_array& r) -> auto { return r[reference]; });
                }
        });
}

template<class V>
auto check_mask()
        -> void
{
        check_mask_with<V, V>();
        check_mask_with<V, operand_t<V>>();
}

// Index arrays ascending, descending, permuted and partial; and, for reading alone, repeated.
[[nodiscard]] inline auto index_arrays(std::size_t n, bool repeats)
        -> std::vector<std::valarray<std::size_t>>
{
        auto result = std::vector<std::valarray<std::size_t>>{std::valarray<std::size_t>()};
        if (n == 0UZ) {
                return result;
        }
        auto ascending  = std::valarray<std::size_t>(n);
        auto descending = std::valarray<std::size_t>(n);
        auto shuffled   = std::valarray<std::size_t>(n);
        for (auto const i : std::views::iota(0UZ, n)) {
                ascending[i]  = i;
                descending[i] = n - 1UZ - i;
                shuffled[i]   = (i * 7UZ) % n;
        }
        result.push_back(ascending);
        result.push_back(descending);
        result.push_back(std::valarray<std::size_t>{n - 1UZ});
        result.emplace_back(descending[std::slice(0UZ, (n + 1UZ) / 2UZ, 2UZ)]);
        if (n % 7UZ != 0UZ) {
                result.push_back(shuffled);
        }
        if (repeats) {
                result.push_back(std::valarray<std::size_t>{0UZ, n - 1UZ, 0UZ, n / 2UZ, n - 1UZ});
        }
        return result;
}

// [valarray.sub]/8-9 and [template.indirect.array]: the indexed elements gathered, and written through its proxy.
template<class V>
auto check_indirect()
        -> void
{
        for_each_subscripted<V>([](model const& a) -> void {
                for (auto const& indices : index_arrays(a.size(), true)) {
                        BOOST_TEST_CONTEXT("indices " << indices.size())
                        {
                                if (indices.size() <= gather_capacity<V>) {
                                        check_reads<V>(a, indices, indices);
                                }
                        }
                }
                for (auto const& indices : index_arrays(a.size(), false)) {
                        BOOST_TEST_CONTEXT("indices " << indices.size())
                        {
                                check_writes<V>(a, std::vector<std::size_t>(std::begin(indices), std::end(indices)), [&](V& x) -> auto { return x[indices]; }, [&](reference_array& r) -> auto { return r[indices]; });
                        }
                }
        });
}

// [valarray.cons]: n bools read from a pointer, where n is the type's own at a fixed size.
template<class V>
auto check_pointer()
        -> void
{
        for_each_subscripted<V>([](model const& a) -> void {
                auto const p = std::make_unique<bool[]>(a.size()); // NOLINT(modernize-avoid-c-arrays): what the pointer reads.
                for (auto const i : std::views::iota(0UZ, a.size())) {
                        p[i] = a[i];
                }
                check_equal(V(p.get(), a.size()), a);
        });
}

} // namespace test::valarray

#endif // TEST_VALARRAY_HPP
