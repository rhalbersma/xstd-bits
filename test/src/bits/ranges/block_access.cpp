//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_CHECK_EQUAL, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/bitset/factory.hpp>                // make_bitset
#include <xstd/bits/bit_array.hpp>                // bit_array
#include <xstd/bits/bit_static_set.hpp>           // bit_static_set
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // the one that stays element-wise
#include <xstd/bits/ext/std/bitset.hpp>           // block_access over std::bitset, where the words are reachable
#include <xstd/bits/ranges/block_access.hpp>      // block_range
#include <xstd/bits/ranges/sequence_view.hpp>     // sequence_view
#include <xstd/bits/ranges/set_view.hpp>          // set_view
#include <algorithm>                              // lexicographical_compare, lexicographical_compare_three_way
#include <bitset>                                 // bitset
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t
#include <set>                                    // set
#include <vector>                                 // vector

BOOST_AUTO_TEST_SUITE(Ranges)
BOOST_AUTO_TEST_SUITE(BlockAccess)

namespace {

// Every ordered pair of subsets of an N-bit universe; the universe is small because the sweep is quadratic in 2^N.
template<std::size_t N, class Block>
auto sweep() -> void
{
        auto set_disagreements    = 0;
        auto sequence_disagreements  = 0;
        auto opaque_disagreements = 0;

        for (auto i = 0UZ; i < (1UZ << N); ++i) {
                for (auto j = 0UZ; j < (1UZ << N); ++j) {
                        auto sx = xstd::bit_static_set<N, Block>();
                        auto sy = xstd::bit_static_set<N, Block>();
                        auto ax = xstd::bit_array<N, Block>();
                        auto ay = xstd::bit_array<N, Block>();
                        auto kx = std::set<std::size_t>();
                        auto ky = std::set<std::size_t>();
                        auto vx = std::vector<bool>(N);
                        auto vy = std::vector<bool>(N);

                        for (auto k = 0UZ; k < N; ++k) {
                                if (i >> k & 1UZ) { sx.insert(k); ax[k] = true; kx.insert(k); vx[k] = true; }
                                if (j >> k & 1UZ) { sy.insert(k); ay[k] = true; ky.insert(k); vy[k] = true; }
                        }

                        set_disagreements   += ((sx <=> sy) < 0) != std::ranges::lexicographical_compare(kx, ky);
                        sequence_disagreements += ((ax <=> ay) < 0) != std::ranges::lexicographical_compare(vx, vy);

                        // The invariant proper: these two stream blocks and never iterate, and must still mean what iterating would.
                        opaque_disagreements += (sx <=> sy) != std::lexicographical_compare_three_way(sx.begin(), sx.end(), sy.begin(), sy.end());
                        opaque_disagreements += (ax <=> ay) != std::lexicographical_compare_three_way(ax.begin(), ax.end(), ay.begin(), ay.end());
                }
        }

        // Checked once rather than per pair: a million passing assertions say no more than one, and drown the log if any fails.
        BOOST_CHECK_EQUAL(set_disagreements, 0);
        BOOST_CHECK_EQUAL(sequence_disagreements, 0);
        BOOST_CHECK_EQUAL(opaque_disagreements, 0);
}

// Dependent, so a standard library without the member is a substitution failure rather than a hard error.
template<class B>
constexpr bool has_getword = requires (B const& c) { c._Getword(0UZ); };

// The same invariant through the two views, over both routes: streaming and iterating must mean the same thing.
template<class Bits>
auto views_agree_with_iteration(std::size_t universe) -> void
{
        auto disagreements = 0;
        auto const bound = 1UZ << universe;

        for (auto i = 0UZ; i < bound; ++i) {
                for (auto j = 0UZ; j < bound; ++j) {
                        auto x = test::bitset::make_bitset<Bits>(universe);
                        auto y = test::bitset::make_bitset<Bits>(universe);
                        for (auto k = 0UZ; k < universe; ++k) {
                                if (i >> k & 1UZ) { x.set(k); }
                                if (j >> k & 1UZ) { y.set(k); }
                        }

                        auto const sx = xstd::set_view(x);
                        auto const sy = xstd::set_view(y);
                        auto const ax = xstd::sequence_view(x);
                        auto const ay = xstd::sequence_view(y);

                        disagreements += (sx <=> sy) != std::lexicographical_compare_three_way(sx.begin(), sx.end(), sy.begin(), sy.end());
                        disagreements += (ax <=> ay) != std::lexicographical_compare_three_way(ax.begin(), ax.end(), ay.begin(), ay.end());
                }
        }

        BOOST_CHECK_EQUAL(disagreements, 0);
}

} // namespace

// Which types answer where their blocks are; the views are here because a view is as block-accessible as what it views.
BOOST_AUTO_TEST_CASE(OursSayWhereTheirBlocksAre)
{
        using Block = std::uint8_t;

        static_assert(xstd::ranges::block_range<xstd::bit_static_set<9, Block>>);
        static_assert(xstd::ranges::block_range<xstd::bit_array<9, Block>>);
        static_assert(xstd::ranges::block_range<xstd::bitset<9, Block>>);

        static_assert(xstd::ranges::block_range<xstd::set_view<xstd::bitset<9, Block>>>);
        static_assert(xstd::ranges::block_range<xstd::sequence_view<xstd::bitset<9, Block>>>);
}

// dynamic_bitset has blocks and will not hand them over, so it keeps the element-wise path -- which must still be right.
BOOST_AUTO_TEST_CASE(ATypeThatWillNotHandThemOverKeepsTheElementWisePath)
{
        static_assert(not xstd::ranges::block_range<boost::dynamic_bitset<>>);
        static_assert(not xstd::ranges::block_range<xstd::set_view<boost::dynamic_bitset<>>>);
}

// std::bitset only where the standard library lets it, asserted as a disjunction because which side holds is not this library's business.
BOOST_AUTO_TEST_CASE(StdBitsetOnlyWhereItsWordsAreReachable)
{
        constexpr auto reachable = xstd::ranges::block_range<std::bitset<64>>;
        static_assert(reachable == has_getword<std::bitset<64>>);
}

// Three shapes: inside one block, spanning two with the second nearly empty, and spanning two with padding above the last position.
BOOST_AUTO_TEST_CASE(TheOrderingsAgreeInsideASingleBlock)
{
        sweep<8, std::uint8_t>();
}

BOOST_AUTO_TEST_CASE(TheOrderingsAgreeWithOneBitInASecondBlock)
{
        sweep<9, std::uint8_t>();
}

BOOST_AUTO_TEST_CASE(TheOrderingsAgreeWithPaddingAboveTheLastPosition)
{
        sweep<10, std::uint8_t>();
}

// Through the views, for the streaming route and the iterating one alike.
BOOST_AUTO_TEST_CASE(TheViewsMeanWhatIteratingMeans)
{
        views_agree_with_iteration<xstd::bitset<8>>(4);
        views_agree_with_iteration<std::bitset<8>>(4);
        views_agree_with_iteration<boost::dynamic_bitset<>>(4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
