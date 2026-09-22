//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set_view.hpp>  // bit_set_view
#include <xstd/bits/bit_span.hpp>      // bit_span
#include <xstd/bits/bit_subspan.hpp>   // IWYU pragma: keep; bit_subspan, what subspan hands back
#include <xstd/bits/borrowed_bits.hpp> // borrow_bits, borrowed_bits
#include <boost/test/unit_test.hpp>    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                   // ranges::equal, ranges::reverse
#include <array>                       // array
#include <concepts>                    // same_as
#include <cstddef>                     // size_t
#include <cstdint>                     // uint8_t, uint16_t, uint32_t, uint64_t
#include <limits>                      // numeric_limits
#include <ranges>                      // distance, to
#include <span>                        // dynamic_extent, span
#include <type_traits>                 // is_constructible_v
#include <vector>                      // vector

BOOST_AUTO_TEST_SUITE(BorrowedBits)

namespace {

// The positions the words hold, read one bit at a time: bit n of word i is position i * digits + n.
template<class R>
[[nodiscard]] auto positions_of(R const& words)
        -> std::vector<std::size_t>
{
        using block_type = std::ranges::range_value_t<R>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);
        auto positions = std::vector<std::size_t>();
        auto i = 0UZ;
        for (auto const word : words) {
                for (auto n = 0UZ; n < digits; ++n) {
                        if (((word >> n) & 1U) != 0U) {
                                positions.push_back((i * digits) + n);
                        }
                }
                ++i;
        }
        return positions;
}

// Both readings of the same borrowed words agree with the words themselves, forwards and backwards.
template<class Bits, class R>
[[nodiscard]] auto readings_agree_with(Bits& bits, R const& words)
        -> bool
{
        auto const expected = positions_of(words);
        auto const set = xstd::bit_set_view(bits);
        auto const seq = xstd::bit_span(bits);

        auto forward = std::vector<std::size_t>(set.begin(), set.end());
        auto backward = std::vector<std::size_t>(set.rbegin(), set.rend());
        std::ranges::reverse(backward);

        auto from_sequence = std::vector<std::size_t>();
        for (auto n = 0UZ; n < seq.size(); ++n) {
                if (seq[n]) {
                        from_sequence.push_back(n);
                }
        }
        return forward == expected and backward == expected and from_sequence == expected and set.size() == expected.size() and seq.count() == expected.size();
}

template<class W>
concept borrowable = requires (W& w) { xstd::borrow_bits(w); };

template<class V>
concept can_insert = requires (V const& v) { v.insert(0UZ); };

template<class S>
concept can_assign_element = requires (S const& s) { s[0] = true; };

} // namespace

// One word is its own digits: a static width of one block, a pointer and nothing else.
BOOST_AUTO_TEST_CASE(OneWordIsItsOwnDigits)
{
        auto board = std::uint64_t{0b1010'0101};
        auto bits = xstd::borrow_bits(board);
        static_assert(std::same_as<decltype(bits), xstd::borrowed_bits<std::uint64_t, 1>>);
        static_assert(decltype(bits)::extent == 64UZ);
        static_assert(sizeof(bits) == sizeof(std::uint64_t*));
        static_assert(sizeof(xstd::bit_set_view(bits)) == sizeof(void*));
        BOOST_CHECK(readings_agree_with(bits, std::array{board}));

        // The views write through to the word, which is what borrowing is for.
        auto const squares = xstd::bit_set_view(bits);
        BOOST_CHECK(squares.insert(63UZ).second);
        BOOST_CHECK_EQUAL(squares.erase(0UZ), 1UZ);
        BOOST_CHECK_EQUAL(board, (std::uint64_t{1} << 63U) | std::uint64_t{0b1010'0100});
        BOOST_CHECK(readings_agree_with(bits, std::array{board}));
        BOOST_CHECK_EQUAL(squares.max_size(), 64UZ);
}

// A fixed number of words is a static width over them: one block, two, and more, each the storage's own path.
BOOST_AUTO_TEST_CASE(AnArrayOfWordsIsAStaticWidth)
{
        auto one = std::array<std::uint8_t, 1>{0x81};
        auto two = std::array<std::uint16_t, 2>{0x0001, 0x8000};
        auto four = std::array<std::uint32_t, 4>{0x0000'0003, 0x0000'0000, 0x8000'0000, 0x0001'0000};

        auto b1 = xstd::borrow_bits(one);
        auto b2 = xstd::borrow_bits(two);
        auto b4 = xstd::borrow_bits(four);
        static_assert(decltype(b1)::extent == 8UZ and decltype(b2)::extent == 32UZ and decltype(b4)::extent == 128UZ);
        BOOST_CHECK(readings_agree_with(b1, one));
        BOOST_CHECK(readings_agree_with(b2, two));
        BOOST_CHECK(readings_agree_with(b4, four));

        xstd::bit_span(b4).fill(true);
        BOOST_CHECK(std::ranges::equal(four, std::array<std::uint32_t, 4>{~0U, ~0U, ~0U, ~0U}));
        BOOST_CHECK(xstd::bit_span(b4).all());
}

// A growable container's words are a run-time width, the whole of its blocks, and never grow through the view.
BOOST_AUTO_TEST_CASE(AVectorOfWordsIsTheWidthOfItsBlocks)
{
        auto words = std::vector<std::uint32_t>{1U, 0x8000'0000U, 0U};
        auto bits = xstd::borrow_bits(words);
        static_assert(std::same_as<decltype(bits), xstd::borrowed_bits<std::uint32_t>>);
        static_assert(decltype(bits)::extent == std::dynamic_extent);
        static_assert(sizeof(bits) == sizeof(std::span<std::uint32_t>));
        BOOST_CHECK_EQUAL(bits.size(), 96UZ);
        BOOST_CHECK_EQUAL(bits.max_size(), 96UZ);
        BOOST_CHECK_EQUAL(bits.saturating_max_size(), 96UZ);
        BOOST_CHECK(readings_agree_with(bits, words));

        auto const seq = xstd::bit_span(bits);
        seq[40] = true;
        BOOST_CHECK_EQUAL(words[1], 0x8000'0000U | (1U << 8U));

        // A window reaches across the word boundary and writes only its own positions.
        auto const window = seq.subspan(30, 20);
        BOOST_CHECK(static_cast<bool>(window[10]));
        window.fill(true);
        BOOST_CHECK_EQUAL(words[0], 0xC000'0001U);
        BOOST_CHECK_EQUAL(words[1], 0x8003'FFFFU);
        BOOST_CHECK(readings_agree_with(bits, words));
        BOOST_CHECK_EQUAL(xstd::bit_set_view(bits).max_size(), 96UZ);
}

// No words is width zero, and every reading of it is empty.
BOOST_AUTO_TEST_CASE(NoWordsIsWidthZero)
{
        auto words = std::vector<std::uint64_t>();
        auto bits = xstd::borrow_bits(words);
        BOOST_CHECK_EQUAL(bits.size(), 0UZ);
        BOOST_CHECK(xstd::bit_set_view(bits).empty());
        BOOST_CHECK(xstd::bit_set_view(bits).begin() == xstd::bit_set_view(bits).end());
        BOOST_CHECK(xstd::bit_span(bits).empty());
        BOOST_CHECK(xstd::bit_span(bits).all());
        BOOST_CHECK(xstd::bit_span(bits).none());
}

// A view over the borrowed bits through a const reference reads them and cannot write.
BOOST_AUTO_TEST_CASE(AConstViewReadsAndCannotWrite)
{
        auto words = std::array<std::uint16_t, 2>{0x0100, 0x0000};
        auto bits = xstd::borrow_bits(words);
        auto const& readonly = bits;
        auto const set = xstd::bit_set_view(readonly);
        BOOST_CHECK(set.contains(8UZ));
        BOOST_CHECK_EQUAL(set.size(), 1UZ);
        static_assert(not can_insert<decltype(set)>);
        static_assert(not can_assign_element<decltype(xstd::bit_span(readonly))>);
        static_assert(can_insert<decltype(xstd::bit_set_view(bits))>);
        static_assert(can_assign_element<decltype(xstd::bit_span(bits))>);
}

// Only words that can be written are borrowed, and only as unsigned integers.
BOOST_AUTO_TEST_CASE(OnlyWritableWordsAreBorrowed)
{
        static_assert(borrowable<std::vector<std::uint32_t>>);
        static_assert(borrowable<std::uint8_t>);
        static_assert(not borrowable<std::vector<std::uint32_t> const>);
        static_assert(not borrowable<std::uint8_t const>);
        static_assert(not borrowable<std::vector<int>>);
        static_assert(not borrowable<int>);
        static_assert(not std::is_constructible_v<xstd::borrowed_bits<std::uint32_t, 1>>);
        static_assert(std::is_constructible_v<xstd::borrowed_bits<std::uint32_t>>);
        BOOST_CHECK_EQUAL(xstd::borrowed_bits<std::uint32_t>().size(), 0UZ);
}

BOOST_AUTO_TEST_SUITE_END()
