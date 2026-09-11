//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <xstd/bits/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/bits/bit_array.hpp>                // bit_array
#include <xstd/bits/bit_span.hpp>                 // bit_span
#include <xstd/bits/bit_subspan.hpp>              // bit_subspan
#include <xstd/bits/bit_vector.hpp>               // bit_vector
#include <xstd/bits/detail/block_array.hpp>       // block_array
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_traits over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bit_traits over std::bitset
#include <xstd/bits/ownership.hpp>                // ownership
#include <algorithm>                              // equal, fill
#include <bitset>                                 // bitset
#include <concepts>                               // equality_comparable, same_as
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t
#include <functional>                             // hash
#include <iterator>                               // distance
#include <ranges>                                 // borrowed_range, random_access_range, reverse, view
#include <span>                                   // dynamic_extent
#include <stdexcept>                              // out_of_range
#include <tuple>                                  // tuple
#include <type_traits>                            // is_default_constructible_v
#include <utility>                                // declval
#include <vector>                                 // vector

BOOST_AUTO_TEST_SUITE(BitSubspan)

namespace {

using Blocks = xstd::block_array<std::uint8_t, 20>;
using Owner  = xstd::basic_bit_array<20, std::uint8_t>;
using Span   = xstd::bit_span<Blocks>;
using Sub    = xstd::bit_subspan<Blocks>;

// Dependent, so an absent member is a substitution failure rather than a hard error.
template<class X> constexpr bool has_subspan  = requires (X x) { x.subspan(0UZ); x.first(0UZ); x.last(0UZ); };
template<class X> constexpr bool can_fill     = requires (X x) { x.fill(true); };
template<class X> constexpr bool has_bulk_ops = requires (X x) { x &= x; x |= x; x ^= x; x -= x; };
template<class X> constexpr bool has_shifts   = requires (X x) { x <<= 1UZ; x >>= 1UZ; };
template<class W, class O> constexpr bool combinable = requires (W w, O const& o) { w &= o; };

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

using ViewedTypes = std::tuple<Owner, xstd::basic_bit_vector<std::uint8_t>, std::bitset<20>, boost::dynamic_bitset<>>;

}  // namespace

// A window is the referring adaptor windowed, an alias since nothing deduces it; it stores what std::span stores, three words beside the whole view's one. [design.md#windows]
BOOST_AUTO_TEST_CASE(TheWindowIsTheAdaptorWindowed)
{
        static_assert(std::same_as<Sub, xstd::sequence_adaptor<Blocks, xstd::ownership::refers, true>>);
        static_assert(sizeof(Span) == sizeof(void*));
        static_assert(sizeof(Sub)  == 3 * sizeof(std::size_t));

        static_assert(std::same_as<decltype(std::declval<Span const&>().subspan(1UZ)), Sub>);
        static_assert(std::same_as<decltype(std::declval<Span const&>().first(1UZ)),   Sub>);
        static_assert(std::same_as<decltype(std::declval<Span const&>().last(1UZ)),    Sub>);
        static_assert(std::same_as<decltype(std::declval<Sub  const&>().subspan(1UZ)), Sub>);

        // Windows are the view's alone, as std::array and std::vector have no subviews. [design.md#windows]
        static_assert(    has_subspan<Span>);
        static_assert(    has_subspan<Sub>);
        static_assert(not has_subspan<Owner>);

        // A view in std::ranges' sense and borrowed like span; like span it neither compares nor hashes. It fills and takes the four bulk operators a word at a time, and has no shifts. [design.md#windows]
        static_assert(std::ranges::view<Sub>);
        static_assert(std::ranges::borrowed_range<Sub>);
        static_assert(std::ranges::random_access_range<Sub>);
        static_assert(not std::equality_comparable<Sub>);
        static_assert(not std::is_default_constructible_v<std::hash<Sub>>);
        static_assert(    can_fill<Sub>);
        static_assert(    has_bulk_ops<Sub>);
        static_assert(not has_shifts<Sub>);
        static_assert(    can_fill<Span>);
        static_assert(    has_bulk_ops<Span>);
        static_assert(    has_shifts<Span>);
}

// A window sees its positions and nothing beyond them, reading them from zero. [design.md#windows]
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
        BOOST_CHECK(std::ranges::equal(w, std::vector<bool>{ true, false, false, true, false, true }));
        BOOST_CHECK(std::ranges::equal(std::views::reverse(w), std::vector<bool>{ true, false, true, false, false, true }));
        BOOST_CHECK(w[0] and w[3] and w[5]);
        BOOST_CHECK(not w[1] and not w[2] and not w[4]);
        BOOST_CHECK(w.front() and w.back());
        BOOST_CHECK(static_cast<bool>(w.at(3)));
        BOOST_CHECK_THROW(static_cast<void>(w.at(6)), std::out_of_range);
        BOOST_CHECK_EQUAL(std::distance(w.begin(), w.end()), 6);
        BOOST_CHECK_EQUAL(std::distance(w.cbegin(), w.cend()), 6);
}

// Writing through a window writes the storage, one position at a time or through the iterators an algorithm walks. [design.md#windows]
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

}       // namespace

// fill on a window: a masked word at a time over our storage, one position at a time over a foreign one, and never a position outside. [design.md#windows]
BOOST_AUTO_TEST_CASE_TEMPLATE(AWindowFillsItsPositionsAlone, T, ViewedTypes)
{
        for (auto const off : { 0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 15UZ }) {
                for (auto const count : { 0UZ, 1UZ, 3UZ, 8UZ, 9UZ, 20UZ - off }) {
                        if (off + count > 20UZ) {
                                continue;
                        }
                        auto bits = twenty<T>();
                        auto const v = xstd::bit_span(bits);
                        auto model = std::vector<bool>(20, false);
                        for (auto const i : { 2UZ, 8UZ, 13UZ, 19UZ }) {
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
        case 0:  return a and b;
        case 1:  return a or b;
        case 2:  return a != b;
        default: return a and not b;
        }
}

auto window_op(int op, auto const& w, auto const& o)
        -> void
{
        switch (op) {
        case 0:  w &= o; break;
        case 1:  w |= o; break;
        case 2:  w ^= o; break;
        default: w -= o; break;
        }
}

// One combination: a window of the destination at off against a window of the source at other, count wide, against the bool model.
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

}       // namespace

// The four bulk operators on a window of ours against a window at any other alignment: word by word, masked to the window; a source of another block type, a std::bitset's say, is not a source. [design.md#windows]
BOOST_AUTO_TEST_CASE(AWindowCombinesWithAnotherAtAnyAlignment)
{
        for (auto const op : { 0, 1, 2, 3 }) {
                for (auto const off : { 0UZ, 3UZ, 8UZ, 13UZ }) {
                        for (auto const other : { 0UZ, 1UZ, 5UZ, 8UZ, 17UZ }) {
                                for (auto const count : { 0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 20UZ }) {
                                        check_combination(op, off, other, count);
                                }
                        }
                }
        }

        // A window against itself, word by word in place. A source of another block type, a std::bitset's say, is not a source; asking clang whether it is crashes the compiler, so no assertion says so here. [design.md#clang-crashes-on-a-foreign-bulk-source]
        auto self = xstd::basic_bit_vector<std::uint8_t>(std::from_range, pattern(20, 1));
        auto const outside = bools(xstd::bit_span(self).first(3));
        auto const w = xstd::bit_span(self).subspan(3, 12);
        w ^= w;
        BOOST_CHECK(std::ranges::equal(w, std::vector<bool>(12, false)));
        BOOST_CHECK(std::ranges::equal(xstd::bit_span(self).first(3), outside));
        static_assert(combinable<decltype(w), decltype(w)>);
        static_assert(combinable<decltype(w), decltype(xstd::bit_span(self))>);
}

// Windows compose: a window of a window offsets once more, first and last are the two ends, and dynamic_extent reaches the end. [design.md#windows]
BOOST_AUTO_TEST_CASE(WindowsCompose)
{
        auto a = Owner();
        for (auto const i : { 2UZ, 5UZ, 17UZ, 19UZ }) {
                a[i] = true;
        }
        auto const v = xstd::bit_span(a);

        BOOST_CHECK(std::ranges::equal(v.subspan(2).first(4), std::vector<bool>{ true, false, false, true }));
        BOOST_CHECK(std::ranges::equal(v.last(3),             std::vector<bool>{ true, false, true }));
        BOOST_CHECK(std::ranges::equal(v.subspan(1, 5).subspan(1, 2), std::vector<bool>{ true, false }));
        BOOST_CHECK(std::ranges::equal(v.subspan(15).last(3), v.last(3)));
        BOOST_CHECK_EQUAL(v.subspan(4, std::dynamic_extent).size(), 16UZ);
        BOOST_CHECK_EQUAL(v.subspan(4).size(), 16UZ);

        // The degenerate windows: at the end, of no positions, and over a zero width.
        BOOST_CHECK(v.subspan(20).empty());
        BOOST_CHECK(v.first(0).empty());
        BOOST_CHECK(v.last(0).empty());
        BOOST_CHECK(v.subspan(20).begin() == v.subspan(20).end());
        auto z = xstd::basic_bit_array<0, std::uint8_t>();
        BOOST_CHECK(xstd::bit_span(z).subspan(0).empty());
}

// Every viewed storage windows the same way, ours and the two foreign ones alike, through the trait. [design.md#windows]
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
        BOOST_CHECK(std::ranges::equal(v.first(8).last(4), std::vector<bool>{ false, true, true, false }));
}

BOOST_AUTO_TEST_SUITE_END()
