//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/block_types.hpp>          // graded_extents
#include <xstd/bits/bit_traits.hpp>      // bit_storage, bit_traits, block_readable, scan_*, static_bit_extent
#include <xstd/bits/block_sequence.hpp>  // block_array
#include <cstddef>                       // size_t
#include <set>                           // set

// Two adapters over identical storage, differing only in whether they hand their blocks over. [design.md#detection-by-absence]
namespace {

// The floor and nothing more: a width and an indexed read.
template<std::size_t N, class Block>
struct element_bits
{
        xstd::block_array<Block, N> bits{};
};

// The same bits, with block access as well.
template<std::size_t N, class Block>
struct block_bits
{
        xstd::block_array<Block, N> bits{};
};

}       // namespace

namespace xstd {

template<std::size_t N, class Block>
struct bit_traits<element_bits<N, Block>>
{
        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size(element_bits<N, Block> const&) noexcept -> std::size_t { return N; }
        [[nodiscard]] static constexpr auto at(element_bits<N, Block> const& c, std::size_t n) noexcept -> bool { return c.bits.test(n); }
};

template<std::size_t N, class Block>
struct bit_traits<block_bits<N, Block>>
{
        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size(block_bits<N, Block> const&) noexcept -> std::size_t { return N; }
        [[nodiscard]] static constexpr auto at(block_bits<N, Block> const& c, std::size_t n) noexcept -> bool { return c.bits.test(n); }

        [[nodiscard]] static constexpr auto num_blocks(block_bits<N, Block> const& c) noexcept -> std::size_t { return c.bits.num_blocks(); }
        [[nodiscard]] static constexpr auto block(block_bits<N, Block> const& c, std::size_t i) noexcept -> Block { return c.bits.block(i); }
};

}       // namespace xstd

namespace {

namespace bits = xstd::detail::bits;

template<class T>
using traits_of = xstd::bit_traits<T>;

template<class T>
inline constexpr auto extent_of = xstd::bit_traits<T>::extent;

// The model and the container, built from one description so they cannot drift.
template<class T>
[[nodiscard]] auto make(std::set<std::size_t> const& model) -> T
{
        auto c = T();
        for (auto const p : model) {
                c.bits.set(p);
        }
        return c;
}

// Every scan, at every argument its domain admits, against std::set answering the same question.
template<class T>
auto check_scans(std::set<std::size_t> const& model) -> void
{
        auto const c = make<T>(model);
        constexpr auto N = extent_of<T>;

        BOOST_CHECK_EQUAL(bits::scan_count<traits_of<T>>(c), model.size());
        BOOST_CHECK_EQUAL(bits::scan_last<traits_of<T>>(c), N);
        BOOST_CHECK_EQUAL(bits::scan_first<traits_of<T>>(c), model.empty() ? N : *model.begin());

        // One past the width too, every scan here being total. [design.md#total-versus-precondition]
        for (auto n = 0UZ; n <= N + 1UZ; ++n) {
                auto const above = model.upper_bound(n);
                BOOST_CHECK_EQUAL(bits::scan_next<traits_of<T>>(c, n), above == model.end() ? N : *above);

                auto const at_or_above = model.lower_bound(n);
                BOOST_CHECK_EQUAL(bits::scan_inclusive_next<traits_of<T>>(c, n), at_or_above == model.end() ? N : *at_or_above);

                auto const below = model.lower_bound(n < N ? n : N);
                BOOST_CHECK_EQUAL(bits::scan_prev<traits_of<T>>(c, n), below == model.begin() ? N : *std::prev(below));
        }
}

// Patterns rather than every subset: adjacent pairs put a set bit on both sides of every block boundary.
template<class T>
auto check_every_pattern() -> void
{
        constexpr auto N = extent_of<T>;

        check_scans<T>({});

        auto full = std::set<std::size_t>();
        for (auto i = 0UZ; i < N; ++i) {
                full.insert(i);
        }
        check_scans<T>(full);

        for (auto i = 0UZ; i < N; ++i) {
                check_scans<T>({ i });
                if (i + 1UZ < N) {
                        check_scans<T>({ i, i + 1UZ });
                }
        }
}

}       // namespace

BOOST_AUTO_TEST_SUITE(BitTraits)

using ElementTypes = test::graded_extents<element_bits>;
using BlockTypes   = test::graded_extents<block_bits>;

// The floor is a width and an indexed read; nothing above it is required to satisfy bit_storage.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheFloorIsAWidthAndAnIndexedRead, T, ElementTypes)
{
        static_assert(xstd::bit_storage<xstd::bit_traits<T>, T>);
        static_assert(xstd::static_bit_extent<xstd::bit_traits<T>, T>);
}

// The tier split pinned down: one adapter answers block_readable, one does not. [design.md#detection-by-absence]
BOOST_AUTO_TEST_CASE_TEMPLATE(BlockAccessIsWhatSeparatesTheTiers, T, ElementTypes)
{
        static_assert(not xstd::block_readable<xstd::bit_traits<T>, T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BlockAccessIsDetectedWhereOffered, T, BlockTypes)
{
        static_assert(xstd::block_readable<xstd::bit_traits<T>, T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ElementWiseScansAgreeWithStdSet, T, ElementTypes)
{
        check_every_pattern<T>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BlockWiseScansAgreeWithStdSet, T, BlockTypes)
{
        check_every_pattern<T>();
}

// No ordering case: the door carries none, the two readings disagreeing. [design.md#two-readings-disagree]

BOOST_AUTO_TEST_SUITE_END()
