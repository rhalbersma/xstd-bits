//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_VALARRAY_HPP
#define TEST_VALARRAY_HPP

#include <xstd/bits/bit/bit_convert.hpp>            // bit_convert
#include <xstd/bits/bit_algorithm/bit_all_of.hpp>   // bit_all_of
#include <xstd/bits/bit_algorithm/bit_any_of.hpp>   // bit_any_of
#include <xstd/bits/bit_algorithm/bit_count.hpp>    // bit_count
#include <xstd/bits/bit_algorithm/bit_mismatch.hpp> // bit_mismatch
#include <xstd/bits/bit_algorithm/bit_none_of.hpp>  // bit_none_of
#include <xstd/bits/bit_algorithm/bit_reverse.hpp>  // bit_reverse
#include <xstd/bits/bit_algorithm/bit_rotate.hpp>   // bit_rotate
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_valarray.hpp>       // basic_bit_bounded_valarray
#include <xstd/bits/bit_fixed_valarray.hpp>         // basic_bit_fixed_valarray
#include <xstd/bits/bit_span.hpp>                   // bit_span
#include <xstd/bits/bit_valarray.hpp>               // basic_bit_valarray
#include <xstd/bits/bit_vector.hpp>                 // bit_vector
#include <xstd/bits/from_blocks.hpp>                // from_blocks, from_blocks_t
#include <xstd/ints/limits.hpp>                     // numeric_limits
#include <boost/test/unit_test.hpp>                 // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_TEST_CONTEXT
#include <algorithm>                                // count, equal, min, mismatch, reverse
#include <bitset>                                   // bitset
#include <climits>                                  // INT_MAX, INT_MIN
#include <concepts>                                 // constructible_from, convertible_to, copyable, equality_comparable, same_as
#include <cstddef>                                  // ptrdiff_t, size_t
#include <cstdint>                                  // uint64_t
#include <format>                                   // format
#include <functional>                               // hash
#include <iterator>                                 // contiguous_iterator
#include <ranges>                                   // begin, contiguous_range, end, iota, iterator_t, random_access_range, range_value_t, size, sized_range, ssize
#include <span>                                     // dynamic_extent
#include <type_traits>                              // is_default_constructible_v
#include <utility>                                  // declval, move
#include <vector>                                   // vector

// Every member and operator of the three valarrays, each held to the same operation on a std::vector<bool>.
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
inline constexpr auto capacity<xstd::basic_bit_bounded_valarray<Block, N>> = N;

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

} // namespace test::valarray

#endif // TEST_VALARRAY_HPP
