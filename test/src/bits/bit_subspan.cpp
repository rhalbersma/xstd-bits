//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <xstd/bits/sequence_adaptor.hpp>       // sequence_adaptor
#include <xstd/bits/bit_array.hpp>                // bit_array
#include <xstd/bits/bit_span.hpp>                 // bit_span
#include <xstd/bits/bit_subspan.hpp>              // bit_subspan
#include <xstd/bits/bit_vector.hpp>               // bit_vector
#include <xstd/bits/block_sequence.hpp>           // block_array
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
template<class X> constexpr bool has_bulk_ops = requires (X x) { x &= x; x |= x; x ^= x; x -= x; x <<= 1UZ; x >>= 1UZ; };

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

        // A view in std::ranges' sense and borrowed like span; like span it neither compares nor hashes, and unlike the whole view it has no bulk operators yet. [design.md#windows]
        static_assert(std::ranges::view<Sub>);
        static_assert(std::ranges::borrowed_range<Sub>);
        static_assert(std::ranges::random_access_range<Sub>);
        static_assert(not std::equality_comparable<Sub>);
        static_assert(not std::is_default_constructible_v<std::hash<Sub>>);
        static_assert(not can_fill<Sub>);
        static_assert(not has_bulk_ops<Sub>);
        static_assert(    can_fill<Span>);
        static_assert(    has_bulk_ops<Span>);
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

// Every viewed storage windows the same way, ours and the two foreign ones alike, through the door. [design.md#windows]
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
