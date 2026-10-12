//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/rotation.hpp>         // as_owner, rotated_ints, rotation_patterns, with_pattern
#include <xstd/bits/algorithm/bit_rotate.hpp> // bit_rotate
#include <xstd/bits/bit_array.hpp>            // basic_bit_array, bit_array
#include <xstd/bits/bit_vector.hpp>           // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                          // equal, rotate
#include <array>                              // array
#include <cstddef>                            // ptrdiff_t, size_t
#include <cstdint>                            // uint64_t, uint8_t
#include <iterator>                           // iter_reference_t
#include <numeric>                            // gcd
#include <ranges>                             // begin, end, iota, iterator_t, next, size
#include <type_traits>                        // is_reference_v
#include <utility>                            // index_sequence, make_index_sequence
#include <vector>                             // vector

BOOST_AUTO_TEST_SUITE(Rotation)

namespace {

// libstdc++ before 16 holds the bit a rotation by one displaces as a proxy to it: GCC PR libstdc++/121913.
#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE < 16
constexpr auto ranges_rotate_holds_a_proxy = true;
#else
constexpr auto ranges_rotate_holds_a_proxy = false;
#endif

// A bool& is held as the bool it refers to, so only a sequence handing out a proxy reference is exposed.
template<class S>
constexpr auto ranges_rotate_loses_a_bit = ranges_rotate_holds_a_proxy and not std::is_reference_v<std::iter_reference_t<std::ranges::iterator_t<S>>>;

// The flaw is in a closing rotation by one, which the algorithm's Euclidean descent reaches only at a gcd of one.
template<class S>
[[nodiscard]] constexpr auto can_lose_a_bit(std::size_t width, std::size_t turn) noexcept
        -> bool
{
        return ranges_rotate_loses_a_bit<S> and width >= 3UZ and std::gcd(width, turn) == 1UZ;
}

template<class S>
[[nodiscard]] auto by_range(S s, std::size_t turn)
        -> S
{
        std::ranges::rotate(s, std::ranges::next(std::ranges::begin(s), static_cast<std::ptrdiff_t>(turn)));
        return s;
}

template<class S>
[[nodiscard]] auto by_iterators(S s, std::size_t turn)
        -> S
{
        auto const first = std::ranges::begin(s);
        std::ranges::rotate(first, std::ranges::next(first, static_cast<std::ptrdiff_t>(turn)), std::ranges::end(s));
        return s;
}

// The pre-ranges algorithm on purpose: it holds the displaced bit as the value type.
template<class S>
[[nodiscard]] auto by_std(S s, std::size_t turn)
        -> S
{
        std::rotate(s.begin(), std::ranges::next(s.begin(), static_cast<std::ptrdiff_t>(turn)), s.end()); // NOLINT(modernize-use-ranges)
        return s;
}

template<class S>
[[nodiscard]] auto by_algorithm(S s, std::size_t turn)
        -> S
{
        xstd::bit_rotate(s, std::ranges::next(std::ranges::begin(s), static_cast<std::ptrdiff_t>(turn)));
        return s;
}

// The positions where the bools and the ints part.
template<class S>
[[nodiscard]] auto mismatches(S const& got, std::vector<int> const& want)
        -> std::size_t
{
        auto count = 0UZ;
        for (auto const i : std::views::iota(0UZ, want.size())) {
                count += static_cast<std::size_t>(static_cast<bool>(got[i]) != (want[i] != 0));
        }
        return count;
}

// Every pattern of the blank's width at every turn in [0, size()], each check's disagreements summed.
template<class S, class Check>
[[nodiscard]] auto sweep(S const& blank, Check check)
        -> int
{
        auto const width   = std::ranges::size(blank);
        auto disagreements = 0;
        for (auto const& pattern : test::sequence::rotation_patterns(width)) {
                auto const s = test::sequence::with_pattern(blank, pattern, test::sequence::as_owner());
                for (auto const turn : std::views::iota(0UZ, width + 1UZ)) {
                        disagreements += check(s, pattern, turn);
                }
        }
        return disagreements;
}

// Each check over vector<bool> and a bit_vector, at every width to seventeen and those around 64 and 128.
template<class Check>
[[nodiscard]] auto growable_sweeps(Check check)
        -> int
{
        auto disagreements = 0;
        for (auto const width : {0UZ, 1UZ, 2UZ, 3UZ, 4UZ, 5UZ, 6UZ, 7UZ, 8UZ, 9UZ, 10UZ, 11UZ, 12UZ, 13UZ, 14UZ, 15UZ, 16UZ, 17UZ, 63UZ, 64UZ, 65UZ, 127UZ, 128UZ, 129UZ}) {
                disagreements += sweep(std::vector<bool>(width), check);
                disagreements += sweep(xstd::basic_bit_vector<std::uint64_t>(width), check);
        }
        return disagreements;
}

template<class Block, std::size_t... N, class Check>
[[nodiscard]] auto fixed_sweeps(Check check, std::index_sequence<N...>)
        -> int
{
        return (0 + ... + sweep(xstd::basic_bit_array<Block, N>(), check));
}

// Each check over every width to seventeen in narrow blocks, and the widths around 64 and 128 in wide ones.
template<class Check>
[[nodiscard]] auto all_sweeps(Check check)
        -> int
{
        auto const narrow = fixed_sweeps<std::uint8_t>(check, std::make_index_sequence<18>());
        auto const wide   = fixed_sweeps<std::uint64_t>(check, std::index_sequence<63, 64, 65, 127, 128, 129>());
        return growable_sweeps(check) + narrow + wide;
}

// Bit 0 of three turned by one, through both ranges spellings: bit 2 takes it, or where the flaw is, bit 1's zero.
template<class S>
[[nodiscard]] auto answers_the_minimal_case(S const& blank)
        -> bool
{
        auto const pattern = std::vector<bool>{true, false, false};
        auto const want    = ranges_rotate_loses_a_bit<S> ? std::vector<bool>{false, false, false} : std::vector<bool>{false, false, true};
        auto const s       = test::sequence::with_pattern(blank, pattern, test::sequence::as_owner());
        return std::ranges::equal(by_range(s, 1UZ), want) and std::ranges::equal(by_iterators(s, 1UZ), want);
}

} // namespace

// The minimal case: a vector<bool>, a bit_array and a bit_vector lose the bit alike, and a bool& sequence never does.
BOOST_AUTO_TEST_CASE(OneBitOfThreeTurnedByOneIsLostWhereTheLibraryHoldsAProxy)
{
        static_assert(ranges_rotate_loses_a_bit<std::vector<bool>> == ranges_rotate_holds_a_proxy);
        static_assert(ranges_rotate_loses_a_bit<xstd::bit_array<3>> == ranges_rotate_holds_a_proxy);
        static_assert(ranges_rotate_loses_a_bit<xstd::bit_vector> == ranges_rotate_holds_a_proxy);
        static_assert(not ranges_rotate_loses_a_bit<std::array<bool, 3>>);
        BOOST_CHECK(answers_the_minimal_case(std::vector<bool>(3)));
        BOOST_CHECK(answers_the_minimal_case(xstd::bit_array<3>()));
        BOOST_CHECK(answers_the_minimal_case(xstd::bit_vector(3)));
        BOOST_CHECK(answers_the_minimal_case(std::array<bool, 3>()));
}

// Exact where the library holds no proxy; where it does, a bit at most, and only at a turn coprime with the width.
BOOST_AUTO_TEST_CASE(RangesRotateLosesAtMostOneBitAndOnlyAtATurnCoprimeWithTheWidth)
{
        auto const too_many = []<class S>(S const& s, std::vector<bool> const& pattern, std::size_t turn) -> int {
                auto const ints = test::sequence::rotated_ints(pattern, turn);
                auto const most = can_lose_a_bit<S>(pattern.size(), turn) ? 1UZ : 0UZ;
                return static_cast<int>(mismatches(by_range(s, turn), ints) > most) + static_cast<int>(mismatches(by_iterators(s, turn), ints) > most);
        };
        BOOST_CHECK_EQUAL(all_sweeps(too_many), 0);
}

// Bit for bit, flaw and all: the algorithm takes the same path through any proxy, so ours answer as vector<bool> does.
BOOST_AUTO_TEST_CASE(RangesRotateLeavesOurSequencesAsItLeavesVectorBool)
{
        auto const unlike = []<class S>(S const& s, std::vector<bool> const& pattern, std::size_t turn) -> int {
                auto const by_range_alike     = std::ranges::equal(by_range(s, turn), by_range(pattern, turn));
                auto const by_iterators_alike = std::ranges::equal(by_iterators(s, turn), by_iterators(pattern, turn));
                return static_cast<int>(not by_range_alike) + static_cast<int>(not by_iterators_alike);
        };
        BOOST_CHECK_EQUAL(all_sweeps(unlike), 0);
}

// std::rotate holds the displaced bit as the value type, so it is exact on every library.
BOOST_AUTO_TEST_CASE(StdRotateIsExactOnEveryLibrary)
{
        auto const inexact = []<class S>(S const& s, std::vector<bool> const& pattern, std::size_t turn) -> int {
                return static_cast<int>(mismatches(by_std(s, turn), test::sequence::rotated_ints(pattern, turn)) != 0UZ);
        };
        BOOST_CHECK_EQUAL(all_sweeps(inexact), 0);
}

// bit_rotate is a pass over the blocks, asking nothing of the library's proxies, so it is exact on every library.
BOOST_AUTO_TEST_CASE(BitRotateIsExactOnEveryLibrary)
{
        auto const inexact = []<class S>(S const& s, std::vector<bool> const& pattern, std::size_t turn) -> int {
                if constexpr (requires (S& r) { xstd::bit_rotate(r, std::ranges::begin(r)); }) {
                        return static_cast<int>(mismatches(by_algorithm(s, turn), test::sequence::rotated_ints(pattern, turn)) != 0UZ);
                } else {
                        return 0;
                }
        };
        BOOST_CHECK_EQUAL(all_sweeps(inexact), 0);
}

BOOST_AUTO_TEST_SUITE_END()
