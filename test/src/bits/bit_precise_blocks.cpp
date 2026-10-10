//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit.hpp>               // bit_convert
#include <xstd/bits/bit_array.hpp>         // basic_bit_array
#include <xstd/bits/bit_fixed_set.hpp>     // basic_bit_fixed_set
#include <xstd/bits/bit_set.hpp>           // basic_bit_set
#include <xstd/bits/bit_vector.hpp>        // basic_bit_vector
#include <xstd/ints/cstdint/bit_int.hpp>   // bit_uint
#include <boost/test/unit_test.hpp>        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstddef>                         // size_t
#include <cstdint>                         // uint8_t
#include <functional>                      // hash
#include <ranges>                          // iota
#include <tuple>                           // tuple
#include <type_traits>                     // integral_constant

BOOST_AUTO_TEST_SUITE(BitPreciseBlocks)

#ifdef __BITINT_MAXWIDTH__

namespace {

template<std::size_t W>
using width = std::integral_constant<std::size_t, W>;

// Widths no power of two of a byte or more: below a byte, between two, and one short of a wide word.
using odd_widths = std::tuple<width<2>, width<4>, width<17>, width<23>, width<24>, width<127>>;

// The same positions in an odd block and in bytes, so the bytes say what the odd block should answer.
template<std::size_t N>
[[nodiscard]] constexpr auto in_pattern(std::size_t i) noexcept
        -> bool
{
        return i % 3UZ == 0UZ or i + 1UZ == N;
}

template<class Block, std::size_t N>
auto check_array()
        -> void
{
        using odd  = xstd::basic_bit_array<Block, N>;
        using byte = xstd::basic_bit_array<std::uint8_t, N>;
        static_assert(sizeof(odd) == sizeof(Block));

        auto a = odd();
        auto b = byte();
        for (auto const i : std::views::iota(0UZ, N)) {
                a[i] = in_pattern<N>(i);
                b[i] = in_pattern<N>(i);
        }
        BOOST_CHECK_EQUAL(a.count(), b.count());
        BOOST_CHECK(xstd::bit_convert<byte>(a) == b);
        BOOST_CHECK(xstd::bit_convert<odd>(b) == a);
        BOOST_CHECK_EQUAL(std::hash<odd>()(a), std::hash<byte>()(b));

        auto c = a;
        auto d = b;
        c[0] = not c[0];
        d[0] = not d[0];
        BOOST_CHECK((a <=> c) == (b <=> d));

        a.reverse();
        b.reverse();
        BOOST_CHECK(xstd::bit_convert<byte>(a) == b);

        a.rotate(N / 2UZ);
        b.rotate(N / 2UZ);
        BOOST_CHECK(xstd::bit_convert<byte>(a) == b);
}

template<class Block, std::size_t N>
auto check_set()
        -> void
{
        using odd  = xstd::basic_bit_fixed_set<std::size_t, Block, N>;
        using byte = xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, N>;
        static_assert(sizeof(odd) == sizeof(Block));

        auto a = odd();
        auto b = byte();
        for (auto const i : std::views::iota(0UZ, N)) {
                if (in_pattern<N>(i)) {
                        a.insert(i);
                        b.insert(i);
                }
        }
        BOOST_CHECK_EQUAL(a.size(), b.size());
        BOOST_CHECK(xstd::bit_convert<byte>(a) == b);
        BOOST_CHECK(xstd::bit_convert<odd>(b) == a);
        BOOST_CHECK_EQUAL(std::hash<odd>()(a), std::hash<byte>()(b));
        BOOST_CHECK(*a.rbegin() == *b.rbegin());

        auto c = a;
        auto d = b;
        c.erase(0UZ);
        d.erase(0UZ);
        BOOST_CHECK((a <=> c) == (b <=> d));
}

template<std::size_t W, std::size_t N>
concept array_admits = requires { typename xstd::basic_bit_array<xstd::bit_uint<W>, N>; };

template<std::size_t W, std::size_t N>
concept set_admits = requires { typename xstd::basic_bit_fixed_set<std::size_t, xstd::bit_uint<W>, N>; };

template<std::size_t W>
concept vector_admits = requires { typename xstd::basic_bit_vector<xstd::bit_uint<W>>; };

} // namespace

// One block of any width, at its narrowest, one short of full and full.
BOOST_AUTO_TEST_CASE_TEMPLATE(OneOddBlockHoldsAFixedWidth, W, odd_widths)
{
        using block = xstd::bit_uint<W::value>;
        check_array<block, 1UZ>();
        check_array<block, W::value - 1UZ>();
        check_array<block, W::value>();
        check_set<block, 1UZ>();
        check_set<block, W::value - 1UZ>();
        check_set<block, W::value>();
}

// No second odd block, and no width chosen at run time: either would put a position across two blocks.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnOddBlockStaysSingle, W, odd_widths)
{
        static_assert(array_admits<W::value, W::value> and set_admits<W::value, W::value>);
        static_assert(not array_admits<W::value, W::value + 1UZ> and not set_admits<W::value, W::value + 1UZ>);
        static_assert(not vector_admits<W::value>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

// A power of two of a byte or more tiles any width, bit-precise or not.
BOOST_AUTO_TEST_CASE(APowerOfTwoTiles)
{
        static_assert(array_admits<64UZ, 200UZ> and vector_admits<64UZ> and vector_admits<256UZ>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

#else

BOOST_AUTO_TEST_CASE(NeedsBitPreciseIntegers)
{
        BOOST_CHECK(true);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
