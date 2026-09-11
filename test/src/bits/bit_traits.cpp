//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                       // graded_extents
#include <xstd/bits/bit_traits.hpp>                   // all, any, bit_storage, bit_traits, block_readable, contiguous_bit_sequence, count, none, scan_*, static_bit_extent, word_at
#include <xstd/bits/detail/contiguous_bit_array.hpp>  // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/ext/boost/dynamic_bitset.hpp>     // IWYU pragma: keep; bit_traits<boost::dynamic_bitset>
#include <xstd/bits/ext/std/bitset.hpp>               // IWYU pragma: keep; bit_traits<std::bitset>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>    // dynamic_bitset
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <bitset>                                     // bitset
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint8_t, uint64_t
#include <set>                                        // set

// Two adapters over identical storage, differing only in whether they hand their blocks over. [design.md#detection-by-absence]
namespace {

// The required entries and nothing more: a width and an indexed read.
template<class Block, std::size_t N>
struct element_bits
{
        xstd::detail::bits::contiguous_bit_array<Block, N> bits{};
};

// The same bits, with block access as well.
template<class Block, std::size_t N>
struct block_bits
{
        xstd::detail::bits::contiguous_bit_array<Block, N> bits{};
};

}       // namespace

namespace xstd {

template<class Block, std::size_t N>
struct bit_traits<element_bits<Block, N>>
{
        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size(element_bits<Block, N> const&) noexcept -> std::size_t { return N; }
        [[nodiscard]] static constexpr auto at(element_bits<Block, N> const& c, std::size_t n) noexcept -> bool { return c.bits.test(n); }
};

template<class Block, std::size_t N>
struct bit_traits<block_bits<Block, N>>
{
        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size(block_bits<Block, N> const&) noexcept -> std::size_t { return N; }
        [[nodiscard]] static constexpr auto at(block_bits<Block, N> const& c, std::size_t n) noexcept -> bool { return c.bits.test(n); }

        [[nodiscard]] static constexpr auto num_blocks(block_bits<Block, N> const& c) noexcept -> std::size_t { return c.bits.num_blocks(); }
        [[nodiscard]] static constexpr auto block(block_bits<Block, N> const& c, std::size_t i) noexcept -> Block { return c.bits.block(i); }
};

}       // namespace xstd

namespace {

// A type nobody has adapted, for the case below.
struct unknown_to_the_library {};

namespace bits = xstd::detail::bits;

template<class T>
using traits_of = xstd::bit_traits<T>;

template<class T>
inline constexpr auto extent_of = xstd::bit_traits<T>::extent;

// The model and the container, built from one description so they cannot drift.
template<class T>
[[nodiscard]] auto make(std::set<std::size_t> const& model)
        -> T
{
        auto c = T();
        for (auto const p : model) {
                c.bits.set(p);
        }
        return c;
}

// The four the sequence reading asks, synthesized here: neither adapter declares an entry, so this is the
// fallback arm on both tiers, and at N == 0 the arm before either. Its own function, four BOOST_CHECK_EQUALs
// being enough to put check_scans over the cognitive-complexity threshold. [design.md#one-function-per-tier]
template<class T>
auto check_aggregates(std::set<std::size_t> const& model)
        -> void
{
        auto const c = make<T>(model);
        constexpr auto N = extent_of<T>;

        BOOST_CHECK_EQUAL(bits::count<traits_of<T>>(c), model.size());
        BOOST_CHECK_EQUAL(bits::any  <traits_of<T>>(c), not model.empty());
        BOOST_CHECK_EQUAL(bits::none <traits_of<T>>(c), model.empty());
        BOOST_CHECK_EQUAL(bits::all  <traits_of<T>>(c), model.size() == N);
}

// Every scan, at every argument its domain admits, against std::set answering the same question.
template<class T>
auto check_scans(std::set<std::size_t> const& model)
        -> void
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
auto check_every_pattern()
        -> void
{
        constexpr auto N = extent_of<T>;

        check_scans<T>({});
        check_aggregates<T>({});

        auto full = std::set<std::size_t>();
        for (auto i = 0UZ; i < N; ++i) {
                full.insert(i);
        }
        check_scans<T>(full);
        check_aggregates<T>(full);

        for (auto i = 0UZ; i < N; ++i) {
                check_scans<T>({ i });
                check_aggregates<T>({ i });
                if (i + 1UZ < N) {
                        check_scans<T>({ i, i + 1UZ });
                        check_aggregates<T>({ i, i + 1UZ });
                }
        }
}

}       // namespace

BOOST_AUTO_TEST_SUITE(BitTraits)

using ElementTypes = test::graded_extents<element_bits>;
using BlockTypes   = test::graded_extents<block_bits>;

// The required entries are a width and an indexed read; nothing else is needed to satisfy bit_storage.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheRequiredEntriesAreAWidthAndAnIndexedRead, T, ElementTypes)
{
        static_assert(xstd::bit_storage<xstd::bit_traits<T>, T>);
        static_assert(xstd::static_bit_extent<xstd::bit_traits<T>, T>);
}

// The primary is declared and never defined, so a type nobody adapted is a constraint not satisfied, never a silent dynamic width. [design.md#opt-in]
BOOST_AUTO_TEST_CASE(SayingNothingMeansNotAdaptable)
{
        static_assert(not xstd::bit_storage<xstd::bit_traits<unknown_to_the_library>, unknown_to_the_library>);
        static_assert(not xstd::static_bit_extent<xstd::bit_traits<unknown_to_the_library>, unknown_to_the_library>);
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

// No ordering case: the trait carries none, the two readings disagreeing. [design.md#two-readings-disagree]

// The word at any position: aligned, straddling two blocks, and in the last block with nothing above it. [design.md#the-blit]
BOOST_AUTO_TEST_CASE(TheWordAtAPositionReadsAcrossBlocks)
{
        using T = xstd::detail::bits::contiguous_bit_array<std::uint8_t, 20>;
        using traits = xstd::bit_traits<T>;
        auto c = T();
        for (auto const i : { 0UZ, 3UZ, 7UZ, 8UZ, 12UZ, 15UZ, 19UZ }) {
                c.set(i);
        }
        // Blocks: 0b1000'1001, 0b1001'0001, 0b0000'1000.
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 0UZ),  0b1000'1001);
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 8UZ),  0b1001'0001);
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 3UZ),  0b0011'0001);
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 7UZ),  0b0010'0011);
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 12UZ), 0b1000'1001);
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 16UZ), 0b0000'1000);
        BOOST_CHECK_EQUAL(xstd::detail::bits::word_at<traits>(c, 17UZ), 0b0000'0100);
}

BOOST_AUTO_TEST_SUITE_END()

// The common vocabulary the three bit containers answer in their own names. [design.md#the-common-vocabulary]
namespace {

using ours_static  = xstd::detail::bits::contiguous_bit_array<std::uint64_t, 64>;
using ours_dynamic = xstd::detail::bits::contiguous_bit_vector<std::uint64_t>;
using theirs       = std::bitset<64>;
using boosts       = boost::dynamic_bitset<>;

// Each probe is a template: a requires-expression over a concrete type is evaluated eagerly and hard-errors
// rather than answering false, so "does not have" can only be asked through a parameter.
template<class C> concept has_subscript = requires (C const& c, std::size_t n) { c[n];                };
template<class C> concept has_complement= requires (C const& c)                { ~c;                 };
template<class C> concept has_set_value = requires (C& b, std::size_t n, bool v) { b.set(n, v);       };
template<class C> concept has_difference= requires (C& b, C const& c)          { b -= c;             };
template<class C> concept has_subset_of = requires (C const& c)                { c.is_subset_of(c);  };
template<class C> concept has_to_string = requires (C const& c)                { c.to_string();      };

// A storage carrying no vocabulary of its own, reached through its trait alone.
struct word { std::uint64_t bits = 0; };

}       // namespace

template<>
struct xstd::bit_traits<word>
{
        using bits_type = word;
        static constexpr std::size_t extent = 64;
        [[nodiscard]] static constexpr auto size(bits_type const&)                  noexcept -> std::size_t { return extent;                        }
        [[nodiscard]] static constexpr auto at  (bits_type const& c, std::size_t n) noexcept -> bool        { return ((c.bits >> n) & 1ULL) != 0ULL; }
};

BOOST_AUTO_TEST_SUITE(TheCommonVocabulary)

// All three model it, at both widths of ours.
static_assert(xstd::contiguous_bit_sequence<ours_static>);
static_assert(xstd::contiguous_bit_sequence<ours_dynamic>);
static_assert(xstd::contiguous_bit_sequence<theirs>);
static_assert(xstd::contiguous_bit_sequence<boosts>);

// It is the intersection and not the union: every one of these is absent from at least one of the three, so
// asking for it would drop a model. This is what pins the concept to the three rather than to whichever was
// read last.
static_assert(not has_subscript<ours_static>);   // ours reads through test, never a subscript [design.md#test-not-subscript]
static_assert(not has_complement<ours_static>);  // nor does it complement in place
static_assert(not has_set_value<ours_static>);   // nor take the two-argument set
static_assert(not has_difference<theirs>);       // std::bitset has no difference
static_assert(not has_subset_of<theirs>);        // nor boost's set vocabulary
static_assert(not has_to_string<boosts>);        // to_string is std::bitset's alone

// The member door and the trait door are different doors: a storage with no vocabulary of its own is adapted
// and is not one of these, which is why nothing is constrained on this concept. [design.md#the-common-vocabulary]
static_assert(    xstd::bit_storage<xstd::bit_traits<word>, word>);
static_assert(not xstd::contiguous_bit_sequence<word>);

// Exercised and not only asserted: the trait declares the three required entries and nothing else, so every
// question below is answered by a synthesized scan over at(). That is the whole of what an incomplete basis
// costs, and what the trait pays. [design.md#the-primitive-basis]
BOOST_AUTO_TEST_CASE(ATraitOnlyStorageAnswersEveryScan)
{
        using traits = xstd::bit_traits<word>;
        auto const c = word{(1ULL << 3) | (1ULL << 40)};

        BOOST_CHECK_EQUAL(traits::size(c), 64UZ);
        BOOST_CHECK(traits::at(c, 3UZ));
        BOOST_CHECK(not traits::at(c, 4UZ));

        // None of these is an entry on the trait, so each is the generic walk. [design.md#detection-by-absence]
        BOOST_CHECK_EQUAL(xstd::detail::bits::find_first<traits>(c),        3UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::find_next<traits>(c,  3UZ),  40UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::find_prev<traits>(c, 40UZ),   3UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::count<traits>(c),             2UZ);
        BOOST_CHECK(xstd::detail::bits::any<traits>(c));
        BOOST_CHECK(not xstd::detail::bits::all<traits>(c));
        BOOST_CHECK(not xstd::detail::bits::none<traits>(c));
}


BOOST_AUTO_TEST_SUITE_END()
