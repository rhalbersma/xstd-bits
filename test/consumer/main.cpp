//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The gate on the interface line.

#include <xstd/bits.hpp>           // bit_array, bit_bounded_set, bit_bounded_valarray, bit_bounded_vector, bit_convert, bit_count, bit_fixed_set, bit_fixed_valarray, bit_set, bit_set_view, bit_span, bit_subspan, bit_valarray, bit_vector
#include <xstd/bits/ext/boost.hpp> // bit_small_set, bit_small_vector
#include <array>                   // array
#include <concepts>                // copyable, default_initializable, equality_comparable, regular, same_as, totally_ordered
#include <cstddef>                 // size_t
#include <cstdint>                 // uint8_t, uint64_t
#include <ranges>                  // bidirectional_range, random_access_range, range_value_t, view
#include <utility>                 // declval

namespace consumer {

// Each reading named by what it means to a caller, in the standard's own vocabulary rather than by what it derives from.
// Copyable is the floor that owners and views share: a view constructs only from what it views, so it is not regular.
template<class T>
concept is_set_reading =
        std::copyable<T> and std::ranges::bidirectional_range<T> and not std::ranges::random_access_range<T> and
        requires { typename T::key_type; };

template<class T>
concept is_sequence_reading =
        std::copyable<T> and std::ranges::random_access_range<T> and
        std::same_as<std::ranges::range_value_t<T>, bool>;

// A valarray is a sequence whose comparisons answer element by element, so its == yields an array and not a bool.
template<class T>
concept is_valarray_reading =
        is_sequence_reading<T> and not std::equality_comparable<T> and
        requires (T const& v) { { v == v } -> std::same_as<T>; };

// A view's Bits is the storage it reads, so a consumer reaches the view names by deduction.
using blocks             = std::array<std::uint64_t, 1>;
using set_view_of_blocks = decltype(xstd::bit_set_view(std::declval<blocks&>()));
using span_of_blocks     = decltype(xstd::bit_span(std::declval<blocks&>()));
using subspan_of_blocks  = decltype(std::declval<span_of_blocks&>().subspan(8, 8));

// What separates the two kinds: an owner is a value, a view is a handle, and std::span drops equality for the same reason.
static_assert(std::regular<xstd::bit_set> and std::totally_ordered<xstd::bit_set>);
static_assert(std::regular<xstd::bit_array<64>> and std::totally_ordered<xstd::bit_array<64>>);
static_assert(std::ranges::view<set_view_of_blocks> and not std::default_initializable<set_view_of_blocks>);
static_assert(std::ranges::view<span_of_blocks> and not std::equality_comparable<span_of_blocks>);

// The set reading, at three widths and as a view.
static_assert(is_set_reading<xstd::bit_fixed_set<100>>);
static_assert(is_set_reading<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 24>>);
static_assert(is_set_reading<xstd::bit_set>);
static_assert(is_set_reading<set_view_of_blocks>);

// The sequence reading, the window included.
static_assert(is_sequence_reading<xstd::bit_array<64>>);
static_assert(is_sequence_reading<xstd::basic_bit_array<std::uint8_t, 24>>);
static_assert(is_sequence_reading<xstd::bit_vector>);
static_assert(is_sequence_reading<span_of_blocks>);
static_assert(is_sequence_reading<subspan_of_blocks>);

// The bounded column, over whichever inline storage the standard library leaves it.
static_assert(is_set_reading<xstd::bit_bounded_set<100>>);
static_assert(is_sequence_reading<xstd::bit_bounded_vector<100>>);

// The small column, opt-in through its own header: inline up to a static capacity and on the heap past it.
static_assert(is_set_reading<xstd::bit_small_set<100>>);
static_assert(is_sequence_reading<xstd::bit_small_vector<100>>);

// The valarray reading, over the fixed, bounded and dynamic storages.
static_assert(is_valarray_reading<xstd::bit_fixed_valarray<100>>);
static_assert(is_valarray_reading<xstd::bit_bounded_valarray<100>>);
static_assert(is_valarray_reading<xstd::bit_valarray>);

} // namespace consumer

auto main()
        -> int
{
        auto failures    = 0;
        auto const check = [&failures](bool ok) noexcept { failures += ok ? 0 : 1; };

        // The set reading over storage the container owns.
        auto set = xstd::bit_fixed_set<100>();
        set.insert(1);
        set.insert(2);
        set.insert(3);
        check(set.size() == 3);
        check(set.contains(2));
        check(*set.begin() == 1);

        auto grown = xstd::bit_set();
        grown.insert(64);
        check(grown.contains(64));

        // The sequence reading.
        auto array = xstd::bit_array<64>();
        array[7]   = true;
        check(xstd::bit_count(array) == 1);

        auto vector = xstd::bit_vector(64);
        vector[63]  = true;
        check(vector.size() == 64 and xstd::bit_count(vector) == 1);

        // A fixed width into a run-time one, the width carried along.
        auto const converted = xstd::bit_convert<xstd::bit_vector>(array);
        check(converted.size() == 64 and converted[7]);

        auto bounded = xstd::bit_bounded_set<100>();
        bounded.insert(99);
        check(bounded.contains(99));

        // The small column, past its inline capacity.
        auto small_set = xstd::bit_small_set<64>();
        small_set.insert(200);
        check(small_set.contains(200));

        auto small_vector = xstd::bit_small_vector<64>(100);
        small_vector[99]  = true;
        check(small_vector.size() == 100 and xstd::bit_count(small_vector) == 1);

        // The valarray reading: an operator answers for every element, and a reduction for the whole array.
        auto fixed_lanes = xstd::bit_fixed_valarray<100>();
        fixed_lanes[99]  = true;
        check(fixed_lanes.sum() and not fixed_lanes.min());
        check(xstd::bit_count(fixed_lanes == fixed_lanes) == 100);

        auto const bounded_lanes = xstd::bit_bounded_valarray<100>(true, 10);
        check(bounded_lanes.size() == 10 and bounded_lanes.min());

        auto lanes = xstd::bit_valarray(64);
        lanes[3]   = true;
        check(xstd::bit_count(lanes ^ true) == 63);

        // The three view names end to end, over blocks no container owns, read one way and then the other.
        auto owner = consumer::blocks();
        auto view  = xstd::bit_set_view(owner);
        view.insert(9);
        view.insert(40);
        check(owner[0] == ((std::uint64_t{1} << 9U) | (std::uint64_t{1} << 40U)));
        check(view.size() == 2);

        auto span = xstd::bit_span(owner);
        check(xstd::bit_count(span) == 2);
        check(span[9] and not span[10]);

        auto const window = span.subspan(8, 8);
        check(window.size() == 8);
        check(xstd::bit_count(window) == 1);

        // Const blocks reach a read-only view, and the const is part of the type.
        auto const& frozen = owner;
        auto const reader  = xstd::bit_set_view(frozen);
        check(reader.size() == 2);

        return failures;
}
