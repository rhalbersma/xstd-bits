//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>            // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/minimal_words.hpp>             // minimal_words
#include <xstd/bits/bit_array.hpp>            // bit_array
#include <xstd/bits/bit_sequence_adaptor.hpp> // bit_sequence_adaptor
#include <xstd/bits/bit_set.hpp>              // basic_bit_set, bit_set
#include <xstd/bits/bit_vector.hpp>           // basic_bit_vector, bit_vector
#include <xstd/bits/dynamic_bitset.hpp>       // basic_dynamic_bitset
#include <xstd/bits/from_bit_storage.hpp>     // from_bit_storage, from_bit_storage_t
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                          // equal
#include <concepts>                           // same_as
#include <cstddef>                            // size_t
#include <cstdint>                            // uint8_t
#include <memory>                             // allocator
#include <type_traits>                        // is_constructible_v, is_nothrow_constructible_v
#include <utility>                            // move
#include <vector>                             // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <xstd/bits/bit_bounded_set.hpp>    // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp> // basic_bit_bounded_vector
#include <xstd/bits/bounded_bitset.hpp>     // basic_bounded_bitset
#include <inplace_vector>                   // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(Adopting)

// The buffer the owner hands back is the one it was given: the blocks moved in and out, and were never copied.
BOOST_AUTO_TEST_CASE(TheBlocksMoveInWithoutACopy)
{
        auto v = std::vector<std::uint8_t>{0x05, 0x80};
        auto const* const data = v.data();

        auto s = xstd::basic_bit_set(xstd::from_bit_storage, std::move(v));
        static_assert(std::same_as<decltype(s), xstd::basic_bit_set<std::uint8_t>>);
        BOOST_CHECK(std::ranges::equal(s, std::vector<std::size_t>{0, 2, 15}));
        auto const blocks = std::move(s).extract();
        BOOST_CHECK(blocks.data() == data);
}

// Every bit of the blocks is a position, at each reading, and each reading's owner deduces from them.
BOOST_AUTO_TEST_CASE(EveryBitOfTheBlocksIsAPosition)
{
        auto const v = xstd::basic_bit_vector(xstd::from_bit_storage, std::vector<std::uint8_t>{0x05, 0x80});
        static_assert(std::same_as<decltype(v), xstd::basic_bit_vector<std::uint8_t> const>);
        BOOST_CHECK_EQUAL(v.size(), 16UZ);
        BOOST_CHECK(v[0] and not v[1] and v[2] and v[15]);

        auto const b = xstd::basic_dynamic_bitset(xstd::from_bit_storage, std::vector<std::uint8_t>{0x05, 0x80});
        static_assert(std::same_as<decltype(b), xstd::basic_dynamic_bitset<std::uint8_t> const>);
        BOOST_CHECK_EQUAL(b.size(), 16UZ);
        BOOST_CHECK_EQUAL(b.count(), 3UZ);
}

// flat_set's allocator-extended form: the allocator is deduced with the blocks, and given to the storage.
BOOST_AUTO_TEST_CASE(TheAllocatorExtendedFormDeducesAsThePlainOne)
{
        auto const alloc = std::allocator<std::uint8_t>();
        auto const s = xstd::basic_bit_set(xstd::from_bit_storage, std::vector<std::uint8_t>{0x01}, alloc);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_set<std::uint8_t> const>);
        BOOST_CHECK(s.contains(0));

        auto const v = xstd::basic_bit_vector(xstd::from_bit_storage, std::vector<std::uint8_t>{0x01}, alloc);
        static_assert(std::same_as<decltype(v), xstd::basic_bit_vector<std::uint8_t> const>);
        BOOST_CHECK(v.size() == 8UZ and v[0]);

        auto const b = xstd::basic_dynamic_bitset(xstd::from_bit_storage, std::vector<std::uint8_t>{0x01}, alloc);
        static_assert(std::same_as<decltype(b), xstd::basic_dynamic_bitset<std::uint8_t> const>);
        BOOST_CHECK(b.size() == 8UZ and b.test(0));
}

// extract and adoption round-trip whole blocks, so a width short of them comes back padded, and resize restores it.
BOOST_AUTO_TEST_CASE(ARoundTripKeepsTheBitsAndPadsTheWidth)
{
        auto v = xstd::bit_vector(70);
        v[0] = true;
        v[69] = true;
        auto const original = v;

        auto w = xstd::bit_vector(xstd::from_bit_storage, std::move(v).extract());
        BOOST_CHECK_EQUAL(w.size(), 128UZ);
        BOOST_CHECK(w[0] and w[69] and not w[70]);
        w.resize(70);
        BOOST_CHECK(w == original);
}

// The move is the whole of the plain form, and a static width takes no block container, its tag door being bit_cast.
BOOST_AUTO_TEST_CASE(OnlyARunTimeWidthAdopts)
{
        static_assert(std::is_nothrow_constructible_v<xstd::bit_vector, xstd::from_bit_storage_t, std::vector<std::size_t>>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_set, xstd::from_bit_storage_t, std::vector<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, xstd::from_bit_storage_t, std::vector<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_vector, xstd::from_bit_storage_t, std::size_t>);
        BOOST_CHECK(true);
}

// A storage written outside the library is adopted the same way, its adaptor named since no guide spells it.
BOOST_AUTO_TEST_CASE(AnyResizableStorageIsAdopted)
{
        auto words = test::minimal_words<std::uint8_t>();
        words.push_back(0x80);
        auto const v = xstd::bit_sequence_adaptor<test::minimal_words<std::uint8_t>>(xstd::from_bit_storage, std::move(words));
        BOOST_CHECK(v.size() == 8UZ and v[7] and not v[0]);
}

// Adoption is constant-evaluable wherever the storage's move is.
BOOST_AUTO_TEST_CASE(AdoptionIsConstexpr)
{
        static_assert([] -> bool {
                auto const v = xstd::basic_bit_vector(xstd::from_bit_storage, std::vector<std::uint8_t>{0x01});
                return v.size() == 8UZ and v[0];
        }());
        BOOST_CHECK(true);
}

#ifdef TEST_HAS_INPLACE_VECTOR

// Inline blocks deduce the capacity they hold in whole, through each reading's owner.
BOOST_AUTO_TEST_CASE(InlineBlocksDeduceTheirAlignedCapacity)
{
        auto const v = xstd::basic_bit_bounded_vector(xstd::from_bit_storage, std::inplace_vector<std::uint8_t, 2>{0x81});
        static_assert(std::same_as<decltype(v), xstd::basic_bit_bounded_vector<std::uint8_t, 16> const>);
        BOOST_CHECK(v.size() == 8UZ and v[0] and v[7]);

        auto const s = xstd::basic_bit_bounded_set(xstd::from_bit_storage, std::inplace_vector<std::uint8_t, 2>{0x81});
        static_assert(std::same_as<decltype(s), xstd::basic_bit_bounded_set<std::uint8_t, 16> const>);
        BOOST_CHECK(s.contains(0) and s.contains(7));

        auto const b = xstd::basic_bounded_bitset(xstd::from_bit_storage, std::inplace_vector<std::uint8_t, 2>{0x81});
        static_assert(std::same_as<decltype(b), xstd::basic_bounded_bitset<std::uint8_t, 16> const>);
        BOOST_CHECK_EQUAL(b.count(), 2UZ);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
