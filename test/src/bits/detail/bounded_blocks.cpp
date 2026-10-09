//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifdef _MSC_VER

// Growth past a capacity of nought always throws, and MSVC calls what follows it inside BOOST_CHECK_THROW unreachable.
#pragma warning(disable : 4702)

#endif

#include <xstd/bits/bit_concepts/resizable_bit_blocks.hpp>   // resizable_bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_capacity.hpp> // bit_blocks_capacity_v
#include <xstd/bits/detail/bounded_blocks.hpp>               // bounded_blocks, bounded_blocks_for, no_blocks
#include <boost/test/unit_test.hpp>                          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <array>                                             // array
#include <concepts>                                          // same_as
#include <cstdint>                                           // uint8_t, uint64_t
#include <new>                                               // bad_alloc
#include <ranges>                                            // contiguous_range, empty
#include <type_traits>                                       // is_empty_v, is_trivially_copyable_v, is_trivially_default_constructible_v
#include <utility>                                           // declval

BOOST_AUTO_TEST_SUITE(Detail)
BOOST_AUTO_TEST_SUITE(BoundedBlocks)

using Blocks = xstd::bits::detail::no_blocks<std::uint8_t>;

namespace {

// Not constexpr, so that the conversions run rather than fold into the initializer they feed.
[[nodiscard]] auto pointer_of(Blocks& b) noexcept
        -> std::uint8_t*
{
        return b;
}

[[nodiscard]] auto pointer_of(Blocks const& b) noexcept
        -> std::uint8_t const*
{
        return b;
}

} // namespace

BOOST_AUTO_TEST_CASE(ACapacityOfNoughtHoldsNoBlocks)
{
        static_assert(std::same_as<xstd::bits::detail::bounded_blocks_for<std::uint8_t, 0>, Blocks>);
        static_assert(std::same_as<xstd::bits::detail::bounded_blocks_for<std::uint8_t, 9>, xstd::bits::detail::bounded_blocks<std::uint8_t, 2>>);
        static_assert(std::same_as<xstd::bits::detail::bounded_blocks_for<std::uint64_t, 64>, xstd::bits::detail::bounded_blocks<std::uint64_t, 1>>);
        static_assert(xstd::resizable_bit_blocks<Blocks> and xstd::bit_blocks_capacity_v<Blocks> == 0UZ);
        BOOST_CHECK_EQUAL(Blocks::capacity(), 0UZ);
        BOOST_CHECK_EQUAL(Blocks::max_size(), 0UZ);
}

BOOST_AUTO_TEST_CASE(NoBlocksIsAnEmptyTrivialType)
{
        static_assert(std::is_empty_v<Blocks> and std::is_trivially_copyable_v<Blocks> and std::is_trivially_default_constructible_v<Blocks>);
        auto const a = Blocks();
        auto const b = a;
        BOOST_CHECK(a == b);
}

BOOST_AUTO_TEST_CASE(NoBlocksIsAnEmptyContiguousRange)
{
        static_assert(std::ranges::contiguous_range<Blocks> and std::ranges::contiguous_range<Blocks const>);
        static_assert(std::same_as<decltype(std::declval<Blocks&>()[0UZ]), std::uint8_t&>);
        static_assert(std::same_as<decltype(std::declval<Blocks const&>()[0UZ]), std::uint8_t const&>);
        auto a        = Blocks();
        auto const& c = a;
        BOOST_CHECK(a.begin() == a.end() and a.data() == nullptr);
        BOOST_CHECK(c.begin() == c.end() and c.data() == nullptr);
        BOOST_CHECK(pointer_of(a) == nullptr and pointer_of(c) == nullptr);
        BOOST_CHECK(std::ranges::empty(a));
        BOOST_CHECK_EQUAL(Blocks::size(), 0UZ);
}

BOOST_AUTO_TEST_CASE(GrowthPastNoughtThrowsBadAlloc)
{
        auto const blocks = std::array<std::uint8_t, 1>{0xFFU};
        BOOST_CHECK_THROW(Blocks::resize(1UZ, blocks[0]), std::bad_alloc);
        BOOST_CHECK_THROW(Blocks::push_back(blocks[0]), std::bad_alloc);
        BOOST_CHECK_THROW(Blocks::insert(nullptr, blocks.begin(), blocks.end()), std::bad_alloc);
}

BOOST_AUTO_TEST_CASE(ChangesThatStayAtNoughtChangeNothing)
{
        auto const a      = Blocks();
        auto const blocks = std::array<std::uint8_t, 1>{0xFFU};
        Blocks::resize(0UZ, blocks[0]);
        Blocks::insert(a.end(), blocks.begin(), blocks.begin());
        Blocks::clear();
        BOOST_CHECK(a == Blocks());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
