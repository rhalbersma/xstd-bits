//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>                              // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_span.hpp>                        // bit_span
#include <xstd/bits/bit_traits.hpp>                      // bit_storage, bit_traits, block_readable, contiguous_bit_sequence, static_bit_extent
#include <xstd/bits/bitset.hpp>                          // basic_bitset
#include <xstd/bits/bitset_adaptor.hpp>                  // bitset_adaptor, has_bitops
#include <xstd/bits/detail/contiguous_bit_array.hpp>     // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_block_container
#include <xstd/bits/ext/unsigned_integer.hpp>            // the built-in widths, asked for by name
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                                      // regular, same_as
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                         // tuple

BOOST_AUTO_TEST_SUITE(Ext)
BOOST_AUTO_TEST_SUITE(UnsignedInteger)

using Widths = std::tuple<
        unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long,
        std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t
>;

// Adapted at every built-in width, and by the trait alone: a word has no vocabulary of its own to forward to.
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryWidthIsAdapted, T, Widths)
{
        using traits = xstd::bit_traits<T>;
        static_assert(xstd::bit_storage<traits, T>);
        static_assert(xstd::static_bit_extent<traits, T>);
        static_assert(xstd::block_readable<traits, T>);
        static_assert(traits::extent == static_cast<std::size_t>(xstd::numeric_limits<T>::digits));
}

// The point of the exercise. std::bitset generalized the BITWISE OPERATORS to an arbitrary fixed width, and a
// built-in word is what it generalized them from: it has that half and not the half std::bitset added.
// [design.md#the-degenerate-bit-container]
namespace {

template<class C> concept operator_half = std::regular<C> and
        requires (C& b, C const& c, std::size_t n)
        {
                { b &= c  } -> std::same_as<C&>;
                { b |= c  } -> std::same_as<C&>;
                { b ^= c  } -> std::same_as<C&>;
                { b <<= n } -> std::same_as<C&>;
                { b >>= n } -> std::same_as<C&>;
        };

template<class C> concept member_half =
        requires (C& b, C const& c, std::size_t n)
        {
                c.size(); c.count(); c.test(n); c.all(); c.any(); c.none(); b.set(); b.reset(); b.flip();
        };

}       // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(ItIsTheOperatorHalfAndNotTheMemberHalf, T, Widths)
{
        static_assert(    operator_half<T>);
        static_assert(not member_half<T>);
        static_assert(not xstd::contiguous_bit_sequence<T>);
}

// Nothing else is dragged in: the specialization is constrained, so a signed or class type stays unadapted.
namespace {

struct plain {};
template<class T> concept adapted = requires { xstd::bit_traits<T>::extent; };

}       // namespace

BOOST_AUTO_TEST_CASE(NothingElseIsAdapted)
{
        static_assert(not adapted<plain>);
        static_assert(not adapted<int>);
        static_assert(not adapted<signed char>);
        static_assert(not adapted<bool>);
        static_assert(    adapted<std::uint32_t>);
}

// Both views read and write a word in place, which is what makes this worth shipping: bits you already have,
// viewed as a set or a sequence, without a copy. [design.md#the-degenerate-bit-container]
BOOST_AUTO_TEST_CASE(AViewWritesThroughToTheWord)
{
        auto w = std::uint64_t{};
        auto v = xstd::bit_set_view<std::uint64_t>(w);

        v.insert(3);
        v.insert(40);
        BOOST_CHECK_EQUAL(w, (1ULL << 3U) | (1ULL << 40U));
        BOOST_CHECK_EQUAL(v.size(), 2UZ);
        BOOST_CHECK_EQUAL(*v.begin(), 3UZ);
        BOOST_CHECK(v.contains(40));

        v.erase(3);
        BOOST_CHECK_EQUAL(w, 1ULL << 40U);

        auto const s = xstd::bit_span<std::uint64_t>(w);
        BOOST_CHECK_EQUAL(s.size(), 64UZ);
        BOOST_CHECK_EQUAL(s.count(), 1UZ);
        BOOST_CHECK(s[40] and not s[3]);
}

// A narrow word is a narrow universe, and the trait says so rather than the view guessing.
BOOST_AUTO_TEST_CASE(TheWidthIsTheWordsOwn)
{
        auto b = std::uint8_t{};
        auto v = xstd::bit_set_view<std::uint8_t>(b);
        BOOST_CHECK_EQUAL(v.max_size(), 8UZ);

        v.insert(7);
        BOOST_CHECK_EQUAL(b, 0b1000'0000);
        BOOST_CHECK_EQUAL(v.size(), 1UZ);
}

// The bitset reading refuses a raw word, and the refusal is load-bearing rather than incidental.
//
// has_bitops asks for the MEMBERS -- set(), reset(), flip(), count(), size(), all/any/none, and boost's three
// set predicates -- which a word has none of. Were it to ask only for the bitwise OPERATORS, the framing a raw
// word fits, a word would satisfy it and bitset_adaptor<Block> would compile with one operator meaning
// something else entirely: b -= c on an unsigned integer is arithmetic subtraction, not set difference. It
// wraps where a set difference saturates. [design.md#the-degenerate-bit-container]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitsetReadingRefusesARawWord, T, Widths)
{
        static_assert(not xstd::has_bitops<T>);
}

BOOST_AUTO_TEST_CASE(TheOperatorThatWouldHaveLied)
{
        // Set difference of {2} minus {2, 3} is empty. Arithmetic subtraction is not.
        auto word = std::uint64_t{0b0100};
        word -= std::uint64_t{0b1100};
        BOOST_CHECK(word != 0UZ);
        BOOST_CHECK_EQUAL(word, ~std::uint64_t{} - 7UZ);
}

// And the wrapping spelling is not it either: a word is not a RANGE of blocks, so it fails
// contiguous_block_container. What does work is the array of one, over which bitset_adaptor is a name the
// library already has -- and that one OWNS, which is the opposite of what adapting a raw word is for.
// [design.md#the-degenerate-bit-container]
BOOST_AUTO_TEST_CASE(TheWrappingSpellingOwnsAndIsAlreadyNamed)
{
        static_assert(not xstd::detail::bits::contiguous_block_container<std::uint64_t>);
        static_assert(    xstd::detail::bits::contiguous_block_container<std::array<std::uint64_t, 1>>);

        static_assert(std::same_as<
                xstd::bitset_adaptor<xstd::detail::bits::contiguous_bit_array<std::uint64_t, 64>>,
                xstd::basic_bitset<std::uint64_t, 64>
        >);
        static_assert(sizeof(xstd::basic_bitset<std::uint64_t, 64>) == sizeof(std::uint64_t));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
