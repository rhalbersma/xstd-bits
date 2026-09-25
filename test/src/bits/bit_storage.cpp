//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>       // bit_array
#include <xstd/bits/bit_storage.hpp>     // bit_storage
#include <xstd/bits/bit_set.hpp>         // bit_set
#include <xstd/bits/bit_set_adaptor.hpp> // bit_set_adaptor
#include <xstd/bits/bit_set_view.hpp>    // bit_set_view
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                         // array
#include <bitset>                        // bitset
#include <cstddef>                       // size_t
#include <cstdint>                       // uint8_t, uint16_t, uint32_t, uint64_t
#include <deque>                         // deque
#include <list>                          // list
#include <span>                          // span
#include <vector>                        // vector
#include <version>                       // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitStorage)

namespace {

template<class W>
concept names_an_owner = requires { typename xstd::bit_set_adaptor<W>; };

template<class W>
concept names_a_view = requires { typename xstd::bit_set_view<W>; };

} // namespace

// A word is bit storage, and so is a sized contiguous range of words: every storage the containers hold.
BOOST_AUTO_TEST_CASE(WordsAndContiguousRangesOfWordsAreBitStorage)
{
        static_assert(xstd::bit_storage<std::uint8_t> and xstd::bit_storage<std::uint64_t> and xstd::bit_storage<std::uint64_t const>);
        static_assert(xstd::bit_storage<std::array<std::uint16_t, 3>>);
        static_assert(xstd::bit_storage<std::vector<std::size_t>>);
        static_assert(xstd::bit_storage<std::span<std::uint32_t>> and xstd::bit_storage<std::span<std::uint32_t const, 2>>);
#ifdef __cpp_lib_inplace_vector
        static_assert(xstd::bit_storage<std::inplace_vector<std::uint16_t, 3>>);
#endif
        BOOST_CHECK(true);
}

// A packed container has bit storage and is not bit storage, and neither is anything not laid out as words.
BOOST_AUTO_TEST_CASE(EverythingElseIsNot)
{
        static_assert(not xstd::bit_storage<int> and not xstd::bit_storage<bool> and not xstd::bit_storage<double>);
        static_assert(not xstd::bit_storage<std::vector<int>> and not xstd::bit_storage<std::vector<bool>>);
        static_assert(not xstd::bit_storage<std::deque<std::uint32_t>> and not xstd::bit_storage<std::list<std::uint32_t>>);
        static_assert(not xstd::bit_storage<std::bitset<64>>);
        static_assert(not xstd::bit_storage<xstd::bit_set> and not xstd::bit_storage<xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

// The owners and views are named by bit storage and nothing else.
BOOST_AUTO_TEST_CASE(OwnersAndViewsAreNamedByBitStorage)
{
        static_assert(names_an_owner<std::uint64_t> and names_an_owner<std::vector<std::uint32_t>>);
        static_assert(names_a_view<std::uint64_t const> and names_a_view<std::span<std::uint32_t>>);
        static_assert(not names_an_owner<std::bitset<64>> and not names_an_owner<xstd::bit_set>);
        static_assert(not names_a_view<std::vector<bool>> and not names_a_view<int>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
