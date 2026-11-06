//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                             // check_equal, gslice_of, make, model_of, patterns, reference_of
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp> // basic_bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_gslice.hpp>         // bit_gslice
#include <xstd/bits/bit_valarray/bit_valarray.hpp>       // basic_bit_valarray, bit_valarray
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                     // equal
#include <concepts>                                      // copyable
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <ranges>                                        // views::iota
#include <type_traits>                                   // is_convertible_v
#include <utility>                                       // as_const
#include <valarray>                                      // gslice, valarray
#include <vector>                                        // vector

BOOST_AUTO_TEST_SUITE(BitGslice)

namespace {

[[nodiscard]] auto equal(std::valarray<std::size_t> const& a, std::valarray<std::size_t> const& b)
        -> bool
{
        return std::ranges::equal(a, b);
}

} // namespace

// [gslice.cons] and [gslice.access]: what it is made of reads back, as from a std::gslice.
BOOST_AUTO_TEST_CASE(ItHoldsAStartLengthsAndStridesAsStdGsliceDoes)
{
        static_assert(std::copyable<xstd::bit_gslice>);
        auto const none = xstd::bit_gslice();
        BOOST_CHECK_EQUAL(none.start(), 0UZ);
        BOOST_CHECK_EQUAL(none.size().size(), 0UZ);
        BOOST_CHECK_EQUAL(none.stride().size(), 0UZ);
        auto const g = test::valarray::gslice_of(5UZ, {2UZ, 3UZ}, {64UZ, 17UZ});
        auto const b = xstd::bit_gslice(g);
        BOOST_CHECK_EQUAL(b.start(), g.start());
        BOOST_CHECK(equal(b.size(), g.size()));
        BOOST_CHECK(equal(b.stride(), g.stride()));
}

// One bit_gslice serves every owner and every block, ascending and not, as the std::gslice of the same numbers.
BOOST_AUTO_TEST_CASE(OneIndexServesEveryOwnerAndEveryBlock)
{
        auto const a = test::valarray::patterns(130UZ)[4];
        auto const r = test::valarray::reference_of(a);
        for (auto const& g : {test::valarray::gslice_of(5UZ, {2UZ, 3UZ, 4UZ}, {64UZ, 17UZ, 3UZ}), test::valarray::gslice_of(0UZ, {2UZ, 3UZ}, {1UZ, 2UZ})}) {
                auto const b    = xstd::bit_gslice(g);
                auto const wide = test::valarray::make<xstd::bit_valarray>(a);
                test::valarray::check_equal(wide[b], test::valarray::model_of(r[g]));
                auto const narrow = test::valarray::make<xstd::basic_bit_valarray<std::vector<std::uint8_t>>>(a);
                test::valarray::check_equal(narrow[b], test::valarray::model_of(r[g]));
                auto const fixed = test::valarray::make<xstd::basic_bit_fixed_valarray<std::uint8_t, 130>>(a);
                test::valarray::check_equal(fixed[b], test::valarray::model_of(r[g]));
        }
}

// [valarray.sub]: a std::valarray takes it as the std::gslice it converts to, which holds the same numbers.
BOOST_AUTO_TEST_CASE(ItSubscriptsAStdValarrayAsTheSameStdGslice)
{
        static_assert(std::is_convertible_v<xstd::bit_gslice, std::gslice>);
        static_assert(not std::is_convertible_v<std::gslice, xstd::bit_gslice>);
        auto ints  = std::valarray<int>(30UZ);
        auto bools = std::valarray<bool>(30UZ);
        for (auto const i : std::views::iota(0UZ, ints.size())) {
                ints[i]  = static_cast<int>(i * 7UZ % 11UZ);
                bools[i] = i % 3UZ == 1UZ;
        }
        for (auto const& g : {test::valarray::gslice_of(1UZ, {2UZ, 3UZ}, {10UZ, 3UZ}), test::valarray::gslice_of(4UZ, {3UZ, 2UZ}, {2UZ, 0UZ})}) {
                auto const b         = xstd::bit_gslice(g);
                auto const converted = std::gslice(b);
                BOOST_CHECK_EQUAL(converted.start(), g.start());
                BOOST_CHECK(equal(converted.size(), g.size()));
                BOOST_CHECK(equal(converted.stride(), g.stride()));
                BOOST_CHECK(std::ranges::equal(std::valarray<int>(std::as_const(ints)[b]), std::valarray<int>(std::as_const(ints)[g])));
                BOOST_CHECK(std::ranges::equal(std::valarray<bool>(std::as_const(bools)[b]), std::valarray<bool>(std::as_const(bools)[g])));
                auto by_bit_gslice = ints;
                by_bit_gslice[b]   = -1;
                auto by_gslice     = ints;
                by_gslice[g]       = -1;
                BOOST_CHECK(std::ranges::equal(by_bit_gslice, by_gslice));
        }
}

// A proxy holds its own copy of the index, which copies share, so it outlives the bit_gslice it was made from.
BOOST_AUTO_TEST_CASE(AProxyOutlivesTheBitGsliceItWasMadeFrom)
{
        auto const a  = test::valarray::patterns(130UZ)[4];
        auto v        = test::valarray::make<xstd::bit_valarray>(a);
        auto const g  = test::valarray::gslice_of(1UZ, {4UZ, 8UZ}, {32UZ, 1UZ});
        auto const p  = v[xstd::bit_gslice(g)];
        p             = true;
        auto expected = test::valarray::reference_of(a);
        expected[g]   = true;
        test::valarray::check_equal(v, test::valarray::model_of(expected));
}

BOOST_AUTO_TEST_SUITE_END()
