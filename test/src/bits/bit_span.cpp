//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/ordering.hpp>               // ordering_agrees_with_vector_bool
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_fixed_set.hpp>              // bit_fixed_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_span.hpp>                   // bit_span
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/ownership.hpp>           // storage
#include <xstd/bits/detail/sequence_adaptor.hpp>    // sequence_adaptor
#include <boost/test/unit_test.hpp>                 // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <array>                                    // array
#include <concepts>                                 // constructible_from, convertible_to, derived_from, equality_comparable, same_as, totally_ordered
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t
#include <ranges>                                   // iota
#include <utility>                                  // declval
#include <vector>                                   // vector

BOOST_AUTO_TEST_SUITE(BitSpan)

namespace {

using Storage = xstd::bits::detail::bit_block_container<std::array<std::size_t, 1>, 8>;
using Blocks  = std::array<std::size_t, 1>;

template<class T>
using view_of = decltype(xstd::bit_span(std::declval<T&>()));

// Named rather than a lambda, so the conversion happens at a call boundary the way a caller would meet it.
constexpr auto takes_a_span(xstd::bit_span<Blocks, 8> v) noexcept
        -> bool
{
        return v[3];
}

} // namespace

// The view is the referring adaptor under another name, and over an owner it refers into the storage the owner wraps.
BOOST_AUTO_TEST_CASE(TheViewIsTheReferringAdaptor)
{
        static_assert(std::derived_from<xstd::bit_span<Blocks, 8>, xstd::bits::detail::sequence_adaptor<Storage, xstd::bits::detail::storage::borrowed, xstd::bits::detail::window::all, xstd::bit_span<Blocks, 8>>>);
        static_assert(std::same_as<view_of<Storage>, xstd::bit_span<Blocks, 8>>);
        static_assert(std::same_as<view_of<Storage const>, xstd::bit_span<Blocks const, 8>>);
        static_assert(std::same_as<view_of<xstd::bit_array<8>>, xstd::bit_span<Blocks, 8>>);
        static_assert(std::same_as<view_of<xstd::bit_array<8> const>, xstd::bit_span<Blocks const, 8>>);
}

// A sequence owner is committed to the sequence reading and a set owner to the set one; only the first is spanned.
BOOST_AUTO_TEST_CASE(TheReadingsDoNotMix)
{
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::declval<xstd::bit_fixed_set<8>&>())), xstd::bit_set_view<Blocks, 8>>);
        static_assert(std::constructible_from<xstd::bit_span<Blocks, 8>, xstd::bit_array<8>&>);
        static_assert(not std::constructible_from<xstd::bit_span<Blocks, 8>, xstd::bit_fixed_set<8>&>);
}

// Viewing an owner is implicit and viewing raw storage is not, which is where span draws the line.
BOOST_AUTO_TEST_CASE(ViewingAnOwnerIsImplicit)
{
        static_assert(std::convertible_to<xstd::bit_array<8>&, xstd::bit_span<Blocks, 8>>);
        static_assert(std::convertible_to<xstd::bit_array<8> const&, xstd::bit_span<Blocks const, 8>>);
        static_assert(not std::convertible_to<xstd::bit_array<8> const&, xstd::bit_span<Blocks, 8>>);

        static_assert(not std::convertible_to<xstd::bit_array<8>, xstd::bit_span<Blocks, 8>>);
        static_assert(not std::convertible_to<xstd::bit_array<8>&&, xstd::bit_span<Blocks, 8>>);

        static_assert(std::constructible_from<xstd::bit_span<Blocks, 8>, Storage&>);
        static_assert(not std::convertible_to<Storage&, xstd::bit_span<Blocks, 8>>);

        auto a = xstd::bit_array<8>();
        a[3]   = true;
        BOOST_CHECK(takes_a_span(a));
}

// Like span it neither compares nor orders, where the owner it views does both.
BOOST_AUTO_TEST_CASE(TheViewNeitherComparesNorOrders)
{
        static_assert(not std::equality_comparable<view_of<Storage>>);
        static_assert(not std::totally_ordered<view_of<Storage>>);
}

// A view is mutable through: writing a position through the view writes the bit.
BOOST_AUTO_TEST_CASE(WritingThroughTheViewWritesTheBits)
{
        auto packed     = std::uint8_t{};
        auto const view = xstd::bit_span(packed);

        view[3] = true;
        BOOST_CHECK_EQUAL(packed, std::uint8_t{0b1000});

        view[3] = false;
        BOOST_CHECK_EQUAL(packed, std::uint8_t{0});
}

// The same reading over the type this library packs, so bit_array's operator[] and the view agree position by position.
BOOST_AUTO_TEST_CASE(APackedArrayAgreesWithItsOwnView)
{
        auto packed = xstd::basic_bit_array<unsigned char, 8>{};
        packed[1]   = true;
        packed[6]   = true;

        auto const view = xstd::bit_span(packed);
        for (auto const k : std::views::iota(0UZ, 8UZ)) {
                BOOST_CHECK_EQUAL(static_cast<bool>(view[k]), static_cast<bool>(packed[k]));
        }

        // Both directions, through both proxies.
        packed[1] = false;
        BOOST_CHECK(not static_cast<bool>(packed[1]));
        BOOST_CHECK(not static_cast<bool>(view[1]));

        view[6] = false;
        BOOST_CHECK(not static_cast<bool>(packed[6]));

        view[0] = true;
        BOOST_CHECK(static_cast<bool>(packed[0]));
}

// The sequence reading against std::vector<bool>, through the iterators the view hands out, for every viewed type.
BOOST_AUTO_TEST_CASE(EveryViewedTypeReadsLikeAVectorBool)
{
        test::sequence::ordering_agrees_with_vector_bool<std::uint8_t>();
        test::sequence::ordering_agrees_with_vector_bool<std::array<std::uint8_t, 1>>();
        test::sequence::ordering_agrees_with_vector_bool<std::vector<std::uint64_t>>();
        test::sequence::ordering_agrees_with_vector_bool<xstd::bit_array<8>>();
}

BOOST_AUTO_TEST_SUITE_END()
