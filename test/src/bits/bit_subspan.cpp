//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit/bit_convert.hpp>            // bit_convert
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_span.hpp>                   // bit_span
#include <xstd/bits/bit_subspan.hpp>                // bit_subspan
#include <xstd/bits/bit_type_traits/bit_rebind.hpp> // bit_rebind
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                // copy, equal, fill
#include <array>                                    // array
#include <concepts>                                 // equality_comparable, same_as
#include <cstddef>                                  // ptrdiff_t, size_t
#include <cstdint>                                  // uint8_t
#include <functional>                               // hash
#include <iterator>                                 // size
#include <ranges>                                   // empty, iota, random_access_range, size
#include <span>                                     // dynamic_extent
#include <tuple>                                    // tuple
#include <type_traits>                              // false_type, integral_constant, is_default_constructible_v, is_member_function_pointer_v, true_type
#include <utility>                                  // declval
#include <vector>                                   // vector

BOOST_AUTO_TEST_SUITE(BitSubspan)

namespace {

using Storage = xstd::bits::detail::bit_block_container<std::array<std::uint8_t, 3>, 20>;
using Blocks  = std::array<std::uint8_t, 3>;
using Owner   = xstd::basic_bit_array<std::uint8_t, 20>;
using Span    = xstd::bit_span<Blocks, 20>;
using Sub     = xstd::bit_subspan<Blocks, std::dynamic_extent, 20>;
using CSpan   = xstd::bit_span<Blocks const, 20>;
using CSub    = xstd::bit_subspan<Blocks const, std::dynamic_extent, 20>;

// Dependent, so an absent member is a substitution failure rather than a hard error.
template<class X>
constexpr bool has_subspan = requires (X x) { x.subspan(0UZ); x.first(0UZ); x.last(0UZ); };

template<class X>
constexpr bool can_fill = requires (X x) { x.fill(true); };

template<class X>
constexpr bool has_bulk_ops = requires (X x) { x &= x; x |= x; x ^= x; };

template<class X>
constexpr bool has_shifts = requires (X x) { x <<= 1UZ; x >>= 1UZ; };

template<class W, class O>
constexpr bool combinable = requires (W w, O const& o) { w &= o; };

// Twenty bits of any viewed storage: a static width has them, a run-time one is resized to them, as the sieve does.
template<class T>
auto twenty()
{
        auto x = T();
        if constexpr (requires { x.resize(20UZ); }) {
                x.resize(20UZ);
        }
        return x;
}

using ViewedTypes = std::tuple<Owner, xstd::basic_bit_vector<std::uint8_t>>;

} // namespace

// A window is the referring adaptor windowed, an alias since nothing deduces it, storing what std::span stores.
BOOST_AUTO_TEST_CASE(TheWindowIsTheAdaptorWindowed)
{
        static_assert(std::same_as<Sub, xstd::bit_subspan<Blocks, std::dynamic_extent, 20>>);
        static_assert(sizeof(Span) == sizeof(void*));
        static_assert(sizeof(Sub) == 3 * sizeof(std::size_t));

        static_assert(std::same_as<decltype(std::declval<Span const&>().subspan(1UZ)), Sub>);
        static_assert(std::same_as<decltype(std::declval<Span const&>().first(1UZ)), Sub>);
        static_assert(std::same_as<decltype(std::declval<Span const&>().last(1UZ)), Sub>);
        static_assert(std::same_as<decltype(std::declval<Sub const&>().subspan(1UZ)), Sub>);

        // Windows are the view's alone, as std::array and std::vector have no subviews.
        static_assert(has_subspan<Span>);
        static_assert(has_subspan<Sub>);
        static_assert(not has_subspan<Owner>);

        // Like span it neither compares nor hashes.
        static_assert(not std::equality_comparable<Sub>);
        static_assert(not std::is_default_constructible_v<std::hash<Sub>>);
        static_assert(can_fill<Sub>);
        static_assert(has_bulk_ops<Sub>);
        static_assert(not has_shifts<Sub>);
        static_assert(can_fill<Span>);
        static_assert(has_bulk_ops<Span>);
        static_assert(has_shifts<Span>);

        // Over a const storage nothing writes: the window's bulk operators ask Bits, not the const-stripped bits_type.
        static_assert(not can_fill<CSub>);
        static_assert(not has_bulk_ops<CSub>);
        static_assert(not can_fill<CSpan>);
        static_assert(not has_bulk_ops<CSpan>);
        static_assert(std::ranges::random_access_range<CSub>); // reading is untouched
        static_assert(has_subspan<CSub>);
}

// A window holds as many positions as it views, and no more: max_size, which span lacks, is its size.
BOOST_AUTO_TEST_CASE(AWindowHoldsNoMoreThanItViews)
{
        auto a       = Owner();
        auto const w = xstd::bit_span(a).subspan(2, 6);
        BOOST_CHECK_EQUAL(w.max_size(), 6UZ);
        BOOST_CHECK_EQUAL(w.max_size(), w.size());
}

// Writing through a window writes the storage, one position at a time or through the iterators an algorithm walks.
BOOST_AUTO_TEST_CASE(AWindowWritesThrough)
{
        auto a       = Owner();
        a[2]         = true;
        a[7]         = true;
        a[9]         = true;
        auto const w = xstd::bit_span(a).subspan(2, 6);

        w[1] = true;
        BOOST_CHECK(static_cast<bool>(a[3]));

        std::ranges::fill(w, false);
        BOOST_CHECK(std::ranges::equal(w, std::vector<bool>(6, false)));
        BOOST_CHECK(static_cast<bool>(a[9]));
}

namespace {

// The bools a range holds, for the checks below.
template<class R>
auto bools(R const& r)
        -> std::vector<bool>
{
        return std::vector<bool>(r.begin(), r.end());
}

} // namespace

// fill on a window: a masked block at a time over our storage, one position at a time over a foreign one.
BOOST_AUTO_TEST_CASE_TEMPLATE(AWindowFillsItsPositionsAlone, T, ViewedTypes)
{
        for (auto const off : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 15UZ}) {
                for (auto const count : {0UZ, 1UZ, 3UZ, 8UZ, 9UZ, 20UZ - off}) {
                        if (off + count > 20UZ) {
                                continue;
                        }
                        auto bits    = twenty<T>();
                        auto const v = xstd::bit_span(bits);
                        auto model   = std::vector<bool>(20, false);
                        for (auto const i : {2UZ, 8UZ, 13UZ, 19UZ}) {
                                v[i]     = true;
                                model[i] = true;
                        }
                        v.subspan(off, count).fill(true);
                        std::ranges::fill(model.begin() + static_cast<std::ptrdiff_t>(off), model.begin() + static_cast<std::ptrdiff_t>(off + count), true);
                        BOOST_CHECK(std::ranges::equal(v, model));
                        v.subspan(off, count).fill(false);
                        std::ranges::fill(model.begin() + static_cast<std::ptrdiff_t>(off), model.begin() + static_cast<std::ptrdiff_t>(off + count), false);
                        BOOST_CHECK(std::ranges::equal(v, model));
                }
        }
}

namespace {

// A pattern over n positions from a seed, with a period that never aligns with a block.
auto pattern(std::size_t n, std::size_t seed)
        -> std::vector<bool>
{
        auto v = std::vector<bool>(n);
        for (auto const i : std::views::iota(0UZ, n)) {
                v[i] = ((i + seed) % 3 == 0) or ((i * seed) % 5 == 1);
        }
        return v;
}

// The four operators by number: on two bools for the model, on a window and a source for the sequence.
auto model_op(int op, bool a, bool b)
        -> bool
{
        switch (op) {
                case 0:
                        return a and b;
                case 1:
                        return a or b;
                default:
                        return a != b;
        }
}

auto window_op(int op, auto const& w, auto const& o)
        -> void
{
        switch (op) {
                case 0:
                        w &= o;
                        break;
                case 1:
                        w |= o;
                        break;
                default:
                        w ^= o;
                        break;
        }
}

// One combination: a window of the destination at off against a window of the source at other, count wide.
auto check_combination(int op, std::size_t off, std::size_t other, std::size_t count)
        -> void
{
        auto source       = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(40, 2));
        auto dest         = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(40, 3));
        auto model        = pattern(40, 3);
        auto const w      = xstd::bit_span(dest).subspan(off, count);
        auto const theirs = bools(xstd::bit_span(source).subspan(other, count));
        window_op(op, w, xstd::bit_span(source).subspan(other, count));
        for (auto const i : std::views::iota(0UZ, count)) {
                model[off + i] = model_op(op, model[off + i], theirs[i]);
        }
        BOOST_CHECK(std::ranges::equal(dest, model));
}

} // namespace

// The three bulk operators against a window at any other alignment, block by block and masked to the window.
BOOST_AUTO_TEST_CASE(AWindowCombinesWithAnotherAtAnyAlignment)
{
        for (auto const op : {0, 1, 2}) {
                for (auto const off : {0UZ, 3UZ, 8UZ, 13UZ}) {
                        for (auto const other : {0UZ, 1UZ, 5UZ, 8UZ, 17UZ}) {
                                for (auto const count : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 20UZ}) {
                                        check_combination(op, off, other, count);
                                }
                        }
                }
        }

        // A window against itself, block by block in place; asking clang if another block type is a source crashes it.
        auto self          = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(20, 1));
        auto const outside = bools(xstd::bit_span(self).first(3));
        auto const w       = xstd::bit_span(self).subspan(3, 12);
        w ^= w;
        BOOST_CHECK(std::ranges::equal(w, std::vector<bool>(12, false)));
        BOOST_CHECK(std::ranges::equal(xstd::bit_span(self).first(3), outside));
        static_assert(combinable<decltype(w), decltype(w)>);
        static_assert(combinable<decltype(w), decltype(xstd::bit_span(self))>);
}

// An owner combines into a window as its whole view does, block by block at the window's alignment.
BOOST_AUTO_TEST_CASE(AWindowCombinesWithAnOwnerAsWithItsView)
{
        using bytes = xstd::basic_bit_vector<std::uint8_t>;
        for (auto const op : {0, 1, 2}) {
                auto const vector = bytes(std::from_range, pattern(12, 2));
                auto array        = xstd::basic_bit_array<std::uint8_t, 12>();
                std::ranges::copy(pattern(12, 4), array.begin());
                auto by_owner = bytes(std::from_range, pattern(20, 3));
                auto by_view  = by_owner;
                window_op(op, xstd::bit_span(by_owner).subspan(5, 12), vector);
                window_op(op, xstd::bit_span(by_view).subspan(5, 12), xstd::bit_span(vector));
                window_op(op, xstd::bit_span(by_owner).subspan(2, 12), array);
                window_op(op, xstd::bit_span(by_view).subspan(2, 12), xstd::bit_span(array));
                BOOST_CHECK(by_owner == by_view);
        }
        static_assert(combinable<xstd::bit_subspan<std::vector<std::uint8_t>>, bytes>);
}

// Another block type crosses by an explicit rebind, after which the combine blits as from any owner of the window's.
BOOST_AUTO_TEST_CASE(AnotherBlockTypeCombinesOnceRebound)
{
        using bytes = xstd::basic_bit_vector<std::uint8_t>;
        using wide  = xstd::basic_bit_vector<std::uint64_t>;
        for (auto const op : {0, 1, 2}) {
                auto const source = wide(std::from_range, pattern(12, 2));
                auto by_rebind    = bytes(std::from_range, pattern(20, 3));
                auto by_bools     = by_rebind;
                window_op(op, xstd::bit_span(by_rebind).subspan(5, 12), xstd::bit_convert<xstd::bit_rebind<std::uint8_t, wide>>(source));
                window_op(op, xstd::bit_span(by_bools).subspan(5, 12), bytes(std::from_range, source));
                BOOST_CHECK(by_rebind == by_bools);
        }
}

namespace {

template<class X, std::size_t Count>
constexpr bool has_first = requires (X x) { x.template first<Count>(); };

template<class X, std::size_t Offset, std::size_t Count>
constexpr bool has_subspan_of = requires (X x) { x.template subspan<Offset, Count>(); };

} // namespace

// A static window stores no count: its width is in its type, and writes as a dynamic one does.
BOOST_AUTO_TEST_CASE(AStaticWindowCarriesItsWidthInItsType)
{
        auto a       = Owner();
        auto const v = xstd::bit_span(a);

        static_assert(std::same_as<decltype(v.first<4>()), xstd::bit_subspan<Blocks, 4, 20>>);
        static_assert(std::same_as<decltype(v.subspan<15>()), xstd::bit_subspan<Blocks, 5, 20>>);
        static_assert(sizeof(v.first<4>()) + sizeof(std::size_t) == sizeof(v.first(4)));

        // A masked block at a time, as a dynamic window fills.
        v.subspan<8, 8>().fill(true);
        BOOST_CHECK_EQUAL(a.count(), 8UZ);
        BOOST_CHECK(a[8] and a[15] and not a[7] and not a[16]);
}

// A static window's width is a constant of its type, as an array's is; a dynamic one's stays a function.
BOOST_AUTO_TEST_CASE(AStaticWindowsSizesAreConstantsOfItsType)
{
        using W = xstd::bit_subspan<Blocks, 4, 20>;
        static_assert(std::same_as<decltype(W::size), std::integral_constant<std::size_t, 4> const>);
        static_assert(std::same_as<decltype(W::empty), std::false_type const>);
        static_assert(std::same_as<decltype(xstd::bit_subspan<Blocks, 0, 20>::empty), std::true_type const>);
        // NOLINTBEGIN(readability-static-accessed-through-instance): the call through an object is what is checked.
        static_assert(std::same_as<decltype(std::declval<W const&>().size()), W::size_type>);
        static_assert(std::same_as<decltype(std::declval<W const&>().empty()), bool>);
        static_assert(noexcept(std::declval<W const&>().size()) and noexcept(std::declval<W const&>().empty()));
        static_assert(std::is_member_function_pointer_v<decltype(&Sub::size)>);

        static_assert(W::size == 4UZ);
        static_assert(not W::empty);
        auto a       = Owner();
        auto const w = xstd::bit_span(a).first<4>();
        BOOST_CHECK_EQUAL(std::ranges::size(w), 4UZ);
        BOOST_CHECK_EQUAL(std::size(w), 4UZ);
        BOOST_CHECK(not std::ranges::empty(w));
        // NOLINTEND(readability-static-accessed-through-instance)
}

// Where the type already says a count cannot fit, the member is not there to call, rather than ill-formed inside.
BOOST_AUTO_TEST_CASE(AStaticWindowIsConstrainedByItsExtent)
{
        static_assert(has_first<Span, 20UZ> and not has_first<Span, 21UZ>);
        static_assert(has_subspan_of<Span, 20UZ, 0UZ> and not has_subspan_of<Span, 21UZ, std::dynamic_extent>);
        static_assert(has_subspan_of<Span, 10UZ, 10UZ> and not has_subspan_of<Span, 10UZ, 11UZ>);
        static_assert(has_first<xstd::bit_subspan<Blocks, 4, 20>, 4UZ> and not has_first<xstd::bit_subspan<Blocks, 4, 20>, 5UZ>);
        static_assert(has_first<Sub, 100UZ>);
}

BOOST_AUTO_TEST_SUITE_END()
