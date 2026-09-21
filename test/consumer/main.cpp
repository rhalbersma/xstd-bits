//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The gate on the interface line.

#include <concepts>      // derived_from
#include <xstd/bits.hpp> // bit_array, bit_inplace_set, bit_inplace_vector, bit_set, bit_set_view, bit_span,
                         // bit_static_set, bit_subspan, bit_vector, bitset, bitset_adaptor, dynamic_bitset, inplace_bitset, ownership, sequence_adaptor, set_adaptor
#include <cstddef>       // size_t
#include <cstdint>       // uint8_t
#include <utility>       // declval
#include <version>       // IWYU pragma: keep; __cpp_lib_inplace_vector

namespace consumer {

// The adaptors named without their storage: set and sequence containers are them, bitset ones derive from them.
template<class>
constexpr bool is_set_adaptor = false;
template<class B, xstd::ownership O>
constexpr bool is_set_adaptor<xstd::set_adaptor<B, O>> = true;

template<class>
constexpr bool is_sequence_adaptor = false;
template<class B, xstd::ownership O, bool W>
constexpr bool is_sequence_adaptor<xstd::sequence_adaptor<B, O, W>> = true;

template<class T>
constexpr bool is_bitset_adaptor = requires { typename T::bits_type; } and std::derived_from<T, xstd::bitset_adaptor<typename T::bits_type, T>>;

// A view's Bits is the storage a container wraps, so a consumer reaches the view names by deduction.
using set_view_of_bitset = decltype(xstd::bit_set_view(std::declval<xstd::bitset<64>&>()));
using span_of_bitset = decltype(xstd::bit_span(std::declval<xstd::bitset<64>&>()));
using subspan_of_bitset = decltype(std::declval<span_of_bitset&>().subspan(8, 8));

// The set reading: three widths, one adaptor.
static_assert(is_set_adaptor<xstd::bit_static_set<100>>);
static_assert(is_set_adaptor<xstd::basic_bit_static_set<std::uint8_t, 24>>);
static_assert(is_set_adaptor<xstd::bit_set>);
static_assert(is_set_adaptor<set_view_of_bitset>);

// The sequence reading, the window included.
static_assert(is_sequence_adaptor<xstd::bit_array<64>>);
static_assert(is_sequence_adaptor<xstd::basic_bit_array<std::uint8_t, 24>>);
static_assert(is_sequence_adaptor<xstd::bit_vector>);
static_assert(is_sequence_adaptor<span_of_bitset>);
static_assert(is_sequence_adaptor<subspan_of_bitset>);

// The bitset reading, which owns by construction.
static_assert(is_bitset_adaptor<xstd::bitset<64>>);
static_assert(is_bitset_adaptor<xstd::basic_bitset<std::uint8_t, 24>>);
static_assert(is_bitset_adaptor<xstd::dynamic_bitset>);

// ownership is interface because you cannot name an adaptor without it.
static_assert(xstd::owns(xstd::ownership::owns));
static_assert(not xstd::owns(xstd::ownership::refers));
static_assert(is_set_adaptor<set_view_of_bitset>);

#ifdef __cpp_lib_inplace_vector

// The inplace column, present only where its storage is.
static_assert(is_set_adaptor<xstd::bit_inplace_set<100>>);
static_assert(is_sequence_adaptor<xstd::bit_inplace_vector<100>>);
static_assert(is_bitset_adaptor<xstd::inplace_bitset<100>>);

#endif

} // namespace consumer

auto main()
        -> int
{
        auto failures = 0;
        auto const check = [&failures](bool ok) noexcept { failures += ok ? 0 : 1; };

        // The set reading over storage the container owns.
        auto set = xstd::bit_static_set<100>();
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

        auto inplace = xstd::bit_inplace_set<100>();
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
