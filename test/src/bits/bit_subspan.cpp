//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                   // bit_array
#include <xstd/bits/bit_span.hpp>                    // bit_span
#include <xstd/bits/bit_subspan.hpp>                 // bit_subspan
#include <xstd/bits/bit_vector.hpp>                  // bit_vector
#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/detail/sequence_adaptor.hpp>     // sequence_adaptor
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                 // equal, fill
#include <concepts>                                  // equality_comparable, same_as
#include <cstddef>                                   // size_t
#include <cstdint>                                   // uint8_t
#include <functional>                                // hash
#include <iterator>                                  // distance
#include <ranges>                                    // borrowed_range, random_access_range, reverse, view
#include <span>                                      // dynamic_extent
#include <stdexcept>                                 // out_of_range
#include <tuple>                                     // tuple
#include <type_traits>                               // is_constructible_v, is_convertible_v, is_default_constructible_v
#include <utility>                                   // declval
#include <vector>                                    // vector

BOOST_AUTO_TEST_SUITE(BitSubspan)

namespace {

using Blocks = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 20>;
using Owner = xstd::basic_bit_array<std::uint8_t, 20>;
using Span = xstd::bit_span<Blocks>;
using Sub = xstd::bit_subspan<Blocks>;
using CSpan = xstd::bit_span<Blocks const>;
using CSub = xstd::bit_subspan<Blocks const>;

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
        static_assert(std::same_as<Sub, xstd::bit_subspan<Blocks>>);
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

        // A view in std::ranges' sense and borrowed like span, and like span it neither compares nor hashes.
        static_assert(std::ranges::view<Sub>);
        static_assert(std::ranges::borrowed_range<Sub>);
        static_assert(std::ranges::random_access_range<Sub>);
        static_assert(not std::equality_comparable<Sub>);
        static_assert(not std::is_default_constructible_v<std::hash<Sub>>);
        static_assert(can_fill<Sub>);
        static_assert(has_bulk_ops<Sub>);
        static_assert(not has_shifts<Sub>);
        static_assert(can_fill<Span>);
        static_assert(has_bulk_ops<Span>);
        static_assert(not has_shifts<Span>);

        // Over a const storage nothing writes: the window's bulk operators ask Bits, not the const-stripped bits_type.
        static_assert(not can_fill<CSub>);
        static_assert(not has_bulk_ops<CSub>);
        static_assert(not can_fill<CSpan>);
        static_assert(not has_bulk_ops<CSpan>);
        static_assert(std::ranges::random_access_range<CSub>); // reading is untouched
        static_assert(has_subspan<CSub>);
}

// A window sees its positions and nothing beyond them, reading them from zero.
BOOST_AUTO_TEST_CASE(AWindowSeesItsPositionsAlone)
{
        auto a = Owner();
        a[2] = true;
        a[5] = true;
        a[7] = true;
        auto const v = xstd::bit_span(a);
        auto const w = v.subspan(2, 6);

        BOOST_CHECK_EQUAL(w.size(), 6UZ);
        BOOST_CHECK(not w.empty());
        BOOST_CHECK_EQUAL(w.max_size(), 6UZ);
        BOOST_CHECK(std::ranges::equal(w, std::vector<bool>{true, false, false, true, false, true}));
        BOOST_CHECK(std::ranges::equal(std::views::reverse(w), std::vector<bool>{true, false, true, false, false, true}));
        BOOST_CHECK(w[0] and w[3] and w[5]);
        BOOST_CHECK(not w[1] and not w[2] and not w[4]);
        BOOST_CHECK(w.front() and w.back());
        BOOST_CHECK(static_cast<bool>(w.at(3)));
        BOOST_CHECK_THROW(static_cast<void>(w.at(6)), std::out_of_range);
        BOOST_CHECK_EQUAL(std::distance(w.begin(), w.end()), 6);
        BOOST_CHECK_EQUAL(std::distance(w.cbegin(), w.cend()), 6);
}

// Writing through a window writes the storage, one position at a time or through the iterators an algorithm walks.
BOOST_AUTO_TEST_CASE(AWindowWritesThrough)
{
        auto a = Owner();
        a[2] = true;
        a[7] = true;
        a[9] = true;
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

// fill on a window: a masked word at a time over our storage, one position at a time over a foreign one.
BOOST_AUTO_TEST_CASE_TEMPLATE(AWindowFillsItsPositionsAlone, T, ViewedTypes)
{
        for (auto const off : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 15UZ}) {
                for (auto const count : {0UZ, 1UZ, 3UZ, 8UZ, 9UZ, 20UZ - off}) {
                        if (off + count > 20UZ) {
                                continue;
                        }
                        auto bits = twenty<T>();
                        auto const v = xstd::bit_span(bits);
                        auto model = std::vector<bool>(20, false);
                        for (auto const i : {2UZ, 8UZ, 13UZ, 19UZ}) {
                                v[i] = true;
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
        for (auto i = 0UZ; i < n; ++i) {
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
        auto source = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(40, 2));
        auto dest = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(40, 3));
        auto model = pattern(40, 3);
        auto const w = xstd::bit_span(dest).subspan(off, count);
        auto const theirs = bools(xstd::bit_span(source).subspan(other, count));
        window_op(op, w, xstd::bit_span(source).subspan(other, count));
        for (auto i = 0UZ; i < count; ++i) {
                model[off + i] = model_op(op, model[off + i], theirs[i]);
        }
        BOOST_CHECK(std::ranges::equal(dest, model));
}

} // namespace

// The three bulk operators against a window at any other alignment, word by word and masked to the window.
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

        // A window against itself, word by word in place; asking clang if another block type is a source crashes it.
        auto self = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(20, 1));
        auto const outside = bools(xstd::bit_span(self).first(3));
        auto const w = xstd::bit_span(self).subspan(3, 12);
        w ^= w;
        BOOST_CHECK(std::ranges::equal(w, std::vector<bool>(12, false)));
        BOOST_CHECK(std::ranges::equal(xstd::bit_span(self).first(3), outside));
        static_assert(combinable<decltype(w), decltype(w)>);
        static_assert(combinable<decltype(w), decltype(xstd::bit_span(self))>);
}

// Windows compose: a window of a window offsets once more, and dynamic_extent reaches the end.
BOOST_AUTO_TEST_CASE(WindowsCompose)
{
        auto a = Owner();
        for (auto const i : {2UZ, 5UZ, 17UZ, 19UZ}) {
                a[i] = true;
        }
        auto const v = xstd::bit_span(a);

        BOOST_CHECK(std::ranges::equal(v.subspan(2).first(4), std::vector<bool>{true, false, false, true}));
        BOOST_CHECK(std::ranges::equal(v.last(3), std::vector<bool>{true, false, true}));
        BOOST_CHECK(std::ranges::equal(v.subspan(1, 5).subspan(1, 2), std::vector<bool>{true, false}));
        BOOST_CHECK(std::ranges::equal(v.subspan(15).last(3), v.last(3)));
        BOOST_CHECK_EQUAL(v.subspan(4, std::dynamic_extent).size(), 16UZ);
        BOOST_CHECK_EQUAL(v.subspan(4).size(), 16UZ);

        // The degenerate windows: at the end, of no positions, and over a zero width.
        BOOST_CHECK(v.subspan(20).empty());
        BOOST_CHECK(v.first(0).empty());
        BOOST_CHECK(v.last(0).empty());
        BOOST_CHECK(v.subspan(20).begin() == v.subspan(20).end());
        auto z = xstd::basic_bit_array<std::uint8_t, 0>();
        BOOST_CHECK(xstd::bit_span(z).subspan(0).empty());
}

namespace {

template<class X, std::size_t Count>
constexpr bool has_first = requires (X x) { x.template first<Count>(); };
template<class X, std::size_t Offset, std::size_t Count>
constexpr bool has_subspan_of = requires (X x) { x.template subspan<Offset, Count>(); };

} // namespace

// [span.sub]'s compile-time three: the count in the type, no count stored, and the same positions as at run time.
BOOST_AUTO_TEST_CASE(AStaticWindowCarriesItsWidthInItsType)
{
        auto a = Owner();
        for (auto const i : {2UZ, 5UZ, 17UZ, 19UZ}) {
                a[i] = true;
        }
        auto const v = xstd::bit_span(a);

        auto const f = v.first<4>();
        auto const l = v.last<3>();
        auto const s = v.subspan<1, 5>();
        auto const tail = v.subspan<15>();
        static_assert(std::same_as<decltype(f), xstd::bit_subspan<Blocks, 4> const>);
        static_assert(std::same_as<decltype(tail), xstd::bit_subspan<Blocks, 5> const>);
        static_assert(decltype(s)::extent == 5UZ and Sub::extent == std::dynamic_extent);
        static_assert(sizeof(f) + sizeof(std::size_t) == sizeof(v.first(4)));
        BOOST_CHECK(std::ranges::equal(f, v.first(4)));
        BOOST_CHECK(std::ranges::equal(l, v.last(3)));
        BOOST_CHECK(std::ranges::equal(s, v.subspan(1, 5)));
        BOOST_CHECK(std::ranges::equal(tail, v.subspan(15)));
        BOOST_CHECK(std::ranges::equal(s.subspan<1, 2>(), v.subspan(2, 2)));
        BOOST_CHECK_EQUAL(f.size(), 4UZ);

        // A static window writes as a dynamic one does, a masked word at a time.
        v.subspan<8, 8>().fill(true);
        BOOST_CHECK_EQUAL(a.count(), 12UZ);

        // Over a run-time width the count is checked at run time, as std::span's is.
        auto d = xstd::basic_bit_vector<std::uint8_t>(12UZ);
        auto const dv = xstd::bit_span(d);
        dv.last<6>().fill(true);
        BOOST_CHECK_EQUAL(d.count(), 6UZ);
        BOOST_CHECK(d[11] and not d[5]);
}

// Where the type already says it cannot fit, it is ill-formed; to a dynamic extent implicitly, back explicitly.
BOOST_AUTO_TEST_CASE(AStaticWindowIsCheckedAndConvertedAsStdSpanIs)
{
        static_assert(has_first<Span, 20UZ> and not has_first<Span, 21UZ>);
        static_assert(has_subspan_of<Span, 20UZ, 0UZ> and not has_subspan_of<Span, 21UZ, std::dynamic_extent>);
        static_assert(has_subspan_of<Span, 10UZ, 10UZ> and not has_subspan_of<Span, 10UZ, 11UZ>);
        static_assert(has_first<xstd::bit_subspan<Blocks, 4>, 4UZ> and not has_first<xstd::bit_subspan<Blocks, 4>, 5UZ>);
        static_assert(has_first<Sub, 100UZ>);

        static_assert(std::is_convertible_v<xstd::bit_subspan<Blocks, 4>, Sub>);
        static_assert(not std::is_convertible_v<Sub, xstd::bit_subspan<Blocks, 4>>);
        static_assert(std::is_constructible_v<xstd::bit_subspan<Blocks, 4>, Sub>);
        static_assert(not std::is_constructible_v<xstd::bit_subspan<Blocks, 4>, xstd::bit_subspan<Blocks, 5>>);

        auto a = Owner();
        a[3] = true;
        auto const v = xstd::bit_span(a);
        Sub const dynamic = v.first<4>();
        auto const back = xstd::bit_subspan<Blocks, 4>(dynamic);
        BOOST_CHECK_EQUAL(dynamic.size(), 4UZ);
        BOOST_CHECK(back[3] and not back[0]);
}

// Every viewed storage windows the same way, ours and the two foreign ones alike, through the trait.
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryViewedStorageWindows, T, ViewedTypes)
{
        auto bits = twenty<T>();
        auto const v = xstd::bit_span(bits);
        v[5] = true;
        auto const w = v.subspan(4, 3);
        BOOST_CHECK_EQUAL(w.size(), 3UZ);
        BOOST_CHECK(not w[0] and w[1] and not w[2]);
        w[2] = true;
        BOOST_CHECK(static_cast<bool>(v[6]));
        BOOST_CHECK(std::ranges::equal(v.first(8).last(4), std::vector<bool>{false, true, true, false}));
}

BOOST_AUTO_TEST_SUITE_END()
