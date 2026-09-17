//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/bytewise_bitset.hpp> // bitset_bytes, bitset_layout_verified, bytes_bitset, bytewise_bitset, fits_one_word
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bitset>                               // bitset
#include <cstddef>                              // byte, size_t

BOOST_AUTO_TEST_SUITE(BytewiseBitset)

// Every width one unsigned long long holds goes through to_ullong and the constructor taking one, which the
// standard provides, so at those widths there is nothing to verify and nothing that could be unsatisfied.
BOOST_AUTO_TEST_CASE(AWidthOneWordHoldsNeedsNoLayoutAtAll)
{
        static_assert(xstd::detail::bits::fits_one_word< 0UZ>);
        static_assert(xstd::detail::bits::fits_one_word<64UZ>);
        static_assert(not xstd::detail::bits::fits_one_word<65UZ>);

        static_assert(xstd::detail::bits::bytewise_bitset< 0UZ>);
        static_assert(xstd::detail::bits::bytewise_bitset< 1UZ>);
        static_assert(xstd::detail::bits::bytewise_bitset<64UZ>);
}

// White box, and the one place the wide path's claim is stated rather than relied on: every standard library this
// ladder builds against lays a std::bitset out as ascending words, least significant bit first, so the byte at
// index n / 8 holds position n at bit n % 8. A BOOST_CHECK on the value and not a static_assert on it, so that a
// rung where the answer were ever no reports one failing assertion here rather than failing to compile the target.
BOOST_AUTO_TEST_CASE(TheWideLayoutIsVerifiedOnThisStandardLibrary)
{
        BOOST_CHECK(xstd::detail::bits::bitset_layout_verified);
        static_assert(xstd::detail::bits::bytewise_bitset< 65UZ>);
        static_assert(xstd::detail::bits::bytewise_bitset<200UZ>);

        // A million positions costs what one does: the layout is the standard library's property, so the probes
        // behind the concept are fixed widths and not this one. Probing this one instead is quadratic in it.
        static_assert(xstd::detail::bits::bytewise_bitset<1UZ << 20UZ>);
}

// The byte view is the whole of the object and nothing besides: setting one position lights one bit of one byte
// and leaves every other byte clear. That second half is what refuses a reordered word and a big-endian target
// both, bit_cast handing back the object representation rather than the value.
BOOST_AUTO_TEST_CASE(OnePositionLightsOneBitOfOneByte)
{
        constexpr auto N = 200UZ;
        for (auto i = 0UZ; i < N; ++i) {
                auto bs = std::bitset<N>();
                bs.set(i);
                auto const bytes = xstd::detail::bits::bitset_bytes(bs);
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        BOOST_CHECK(bytes[j] == (j == i / 8UZ ? static_cast<std::byte>(1U << (i % 8UZ)) : std::byte{}));
                }
        }
}

// Both paths, and the same claim of each: the two directions are inverse. Sixty-four takes the standard door and
// two hundred the proved layout, so this is one assertion over two implementations of it.
BOOST_AUTO_TEST_CASE(TheTwoDirectionsAreEachOthersInverse)
{
        auto const round_trips = []<std::size_t N>(std::bitset<N> const& bs) -> bool {
                return xstd::detail::bits::bytes_bitset<N>(xstd::detail::bits::bitset_bytes(bs)) == bs;
        };
        auto narrow = std::bitset<64>();
        auto wide   = std::bitset<200>();
        for (auto i = 0UZ; i < 200UZ; i += 3UZ) {
                if (i < 64UZ) {
                        narrow.set(i);
                }
                wide.set(i);
        }
        BOOST_CHECK(round_trips(narrow));
        BOOST_CHECK(round_trips(wide));

        // The two the stride above reaches neither of, at both widths.
        BOOST_CHECK(round_trips(std::bitset<64>()));
        BOOST_CHECK(round_trips(std::bitset<64>().flip()));
        BOOST_CHECK(round_trips(std::bitset<200>()));
        BOOST_CHECK(round_trips(std::bitset<200>().flip()));

        // Zero width, which has no byte to exchange and round trips all the same.
        BOOST_CHECK(round_trips(std::bitset<0>()));
}

BOOST_AUTO_TEST_SUITE_END()
