//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The gate on the interface line.

#include <concepts>      // convertible_to, copyable, default_initializable, equality_comparable, regular, same_as, totally_ordered
#include <xstd/bits.hpp> // bit_array, bit_bounded_set, bit_bounded_vector, bit_set, bit_set_view, bit_span,
                         // bit_fixed_set, bit_subspan, bit_vector, bitset, dynamic_bitset, bounded_bitset
#include <cstddef>       // size_t
#include <cstdint>       // uint8_t
#include <ranges>        // bidirectional_range, random_access_range, range, range_value_t, view
#include <string>        // string
#include <utility>       // declval
#include <version>       // IWYU pragma: keep; __cpp_lib_inplace_vector

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

// A bit string is the one reading that is no range: it is read whole, the way std::bitset is.
template<class T>
concept is_bitset_reading =
        std::copyable<T> and not std::ranges::range<T> and
        requires (T const& t) { { t.to_string() } -> std::convertible_to<std::string>; };

// A view's Bits is the storage a container wraps, so a consumer reaches the view names by deduction.
using set_view_of_bitset = decltype(xstd::bit_set_view(std::declval<xstd::bitset<64>&>()));
using span_of_bitset = decltype(xstd::bit_span(std::declval<xstd::bitset<64>&>()));
using subspan_of_bitset = decltype(std::declval<span_of_bitset&>().subspan(8, 8));

// What separates the two kinds: an owner is a value, a view is a handle, and std::span drops equality for the same reason.
static_assert(std::regular<xstd::bit_set> and std::totally_ordered<xstd::bit_set>);
static_assert(std::regular<xstd::bitset<64>> and std::totally_ordered<xstd::bitset<64>>);
static_assert(std::ranges::view<set_view_of_bitset> and not std::default_initializable<set_view_of_bitset>);
static_assert(std::ranges::view<span_of_bitset> and not std::equality_comparable<span_of_bitset>);

// The set reading, at three widths and as a view.
static_assert(is_set_reading<xstd::bit_fixed_set<100>>);
static_assert(is_set_reading<xstd::basic_bit_fixed_set<std::uint8_t, 24>>);
static_assert(is_set_reading<xstd::bit_set>);
static_assert(is_set_reading<set_view_of_bitset>);

// The sequence reading, the window included.
static_assert(is_sequence_reading<xstd::bit_array<64>>);
static_assert(is_sequence_reading<xstd::basic_bit_array<std::uint8_t, 24>>);
static_assert(is_sequence_reading<xstd::bit_vector>);
static_assert(is_sequence_reading<span_of_bitset>);
static_assert(is_sequence_reading<subspan_of_bitset>);

// The bitset reading, which owns by construction.
static_assert(is_bitset_reading<xstd::bitset<64>>);
static_assert(is_bitset_reading<xstd::basic_bitset<std::uint8_t, 24>>);
static_assert(is_bitset_reading<xstd::dynamic_bitset>);

static_assert(is_set_reading<set_view_of_bitset>);

#ifdef __cpp_lib_inplace_vector

// The bounded column, present only where its storage is.
static_assert(is_set_reading<xstd::bit_bounded_set<100>>);
static_assert(is_sequence_reading<xstd::bit_bounded_vector<100>>);
static_assert(is_bitset_reading<xstd::bounded_bitset<100>>);

#endif

} // namespace consumer

auto main()
        -> int
{
        auto failures = 0;
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

        // The sequence reading, and the bitset reading.
        auto array = xstd::bit_array<64>();
        array[7] = true;
        check(array.count() == 1);

        auto bits = xstd::bitset<64>();
        bits.set(5);
        check(bits.test(5) and bits.count() == 1);

        auto dynamic = xstd::dynamic_bitset(64);
        dynamic.set(5);
        check(dynamic.test(5) and dynamic.count() == 1);

        auto vector = xstd::bit_vector(64);
        vector[63] = true;
        check(vector.size() == 64 and vector.count() == 1);

#ifdef __cpp_lib_inplace_vector

        auto inplace = xstd::bit_bounded_set<100>();
        inplace.insert(99);
        check(inplace.contains(99));

#endif

        // The three view names end to end, over the one owner committed to neither reading.
        auto owner = xstd::bitset<64>();
        auto view = xstd::bit_set_view(owner);
        view.insert(9);
        view.insert(40);
        check(owner.test(9) and owner.test(40));
        check(view.size() == 2);

        auto span = xstd::bit_span(owner);
        check(span.count() == 2);
        check(span[9] and not span[10]);

        auto const window = span.subspan(8, 8);
        check(window.size() == 8);
        check(window.count() == 1);

        // A const owner reaches a read-only view, and the const is part of the type.
        auto const& frozen = owner;
        auto const reader = xstd::bit_set_view(frozen);
        check(reader.size() == 2);

        return failures;
}
