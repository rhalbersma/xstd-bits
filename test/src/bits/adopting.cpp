//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/minimal_blocks.hpp>               // minimal_blocks
#include <xstd/bits/bit_array.hpp>               // bit_array
#include <xstd/bits/bit_bounded_set.hpp>         // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>      // basic_bit_bounded_vector
#include <xstd/bits/bit_key_traits.hpp>          // bit_key_traits
#include <xstd/bits/bit_set.hpp>                 // basic_bit_set, bit_set
#include <xstd/bits/bit_vector.hpp>              // basic_bit_vector, bit_vector
#include <xstd/bits/detail/bit_container.hpp>    // bit_container
#include <xstd/bits/detail/bounded_blocks.hpp>   // bounded_blocks
#include <xstd/bits/detail/ownership.hpp>        // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp> // sequence_adaptor
#include <xstd/bits/ext/boost/bit_small_set.hpp> // basic_bit_small_set
#include <xstd/bits/from_bit_storage.hpp>        // from_bit_storage, from_bit_storage_t
#include <boost/test/unit_test.hpp>              // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                             // equal
#include <concepts>                              // same_as
#include <cstddef>                               // size_t
#include <cstdint>                               // uint8_t
#include <functional>                            // less
#include <memory>                                // allocator
#include <memory_resource>                       // new_delete_resource, polymorphic_allocator, unsynchronized_pool_resource
#include <tuple>                                 // tuple
#include <type_traits>                           // is_constructible_v, is_nothrow_constructible_v
#include <utility>                               // move
#include <vector>                                // vector

BOOST_AUTO_TEST_SUITE(Adopting)

// The buffer the owner hands back is the one it was given: the blocks moved in and out, and were never copied.
BOOST_AUTO_TEST_CASE(TheBlocksMoveInWithoutACopy)
{
        auto v = std::vector<std::uint8_t>{0x05, 0x80};
        auto const* const data = v.data();

        auto s = xstd::basic_bit_set(xstd::from_bit_storage, std::move(v));
        static_assert(std::same_as<decltype(s), xstd::basic_bit_set<std::size_t, std::uint8_t>>);
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
}

// flat_set's allocator-extended form: the allocator is deduced with the blocks, and given to the storage.
BOOST_AUTO_TEST_CASE(TheAllocatorExtendedFormDeducesAsThePlainOne)
{
        auto const alloc = std::allocator<std::uint8_t>();
        auto const s = xstd::basic_bit_set(xstd::from_bit_storage, std::vector<std::uint8_t>{0x01}, alloc);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_set<std::size_t, std::uint8_t> const>);
        BOOST_CHECK(s.contains(0));

        auto const v = xstd::basic_bit_vector(xstd::from_bit_storage, std::vector<std::uint8_t>{0x01}, alloc);
        static_assert(std::same_as<decltype(v), xstd::basic_bit_vector<std::uint8_t> const>);
        BOOST_CHECK(v.size() == 8UZ and v[0]);
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

// The move is the whole of the plain form, and a static width takes no block container, its tag door being bit_convert.
BOOST_AUTO_TEST_CASE(OnlyARunTimeWidthAdopts)
{
        static_assert(std::is_nothrow_constructible_v<xstd::bit_vector, xstd::from_bit_storage_t, std::vector<std::size_t>>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_set, xstd::from_bit_storage_t, std::vector<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, xstd::from_bit_storage_t, std::vector<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_vector, xstd::from_bit_storage_t, std::size_t>);
        BOOST_CHECK(true);
}

// A storage written outside the library is adopted the same way by the reading's adaptor over it.
BOOST_AUTO_TEST_CASE(AnyResizableStorageIsAdopted)
{
        using blocks_type = test::minimal_blocks<std::uint8_t>;
        auto blocks = blocks_type();
        blocks.push_back(0x80);
        auto const v = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::bit_container<blocks_type>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all>(xstd::from_bit_storage, std::move(blocks));
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

// Inline blocks deduce the capacity they hold in whole, through each bounded owner, whichever storage holds them.
BOOST_AUTO_TEST_CASE(InlineBlocksDeduceTheirAlignedCapacity)
{
        auto const v = xstd::basic_bit_bounded_vector(xstd::from_bit_storage, xstd::bits::detail::bounded_blocks<std::uint8_t, 2>{0x81});
        static_assert(std::same_as<decltype(v), xstd::basic_bit_bounded_vector<std::uint8_t, 16> const>);
        BOOST_CHECK(v.size() == 8UZ and v[0] and v[7]);

        auto const s = xstd::basic_bit_bounded_set(xstd::from_bit_storage, xstd::bits::detail::bounded_blocks<std::uint8_t, 2>{0x81});
        static_assert(std::same_as<decltype(s), xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 16> const>);
        BOOST_CHECK(s.contains(0) and s.contains(7));
}

// The heap owners over a polymorphic allocator, which two resources make unequal.
using polymorphic_owners = std::tuple<xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, std::pmr::polymorphic_allocator<std::uint8_t>>, xstd::basic_bit_small_set<std::size_t, std::uint8_t, 16, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, std::pmr::polymorphic_allocator<std::uint8_t>>>; // NOLINT(modernize-use-transparent-functors): the default comparator, spelled to reach the allocator

// Blocks adopted under an unequal allocator are copied into it; storage taken from new and delete would leak.
BOOST_AUTO_TEST_CASE_TEMPLATE(AdoptionUnderAnUnequalAllocatorCopiesTheBlocks, S, polymorphic_owners)
{
        auto adopter = std::pmr::unsynchronized_pool_resource();
        auto const keys = std::vector<std::size_t>{0, 9, 100, 700};
        auto source = S(keys.begin(), keys.end(), typename S::allocator_type(std::pmr::polymorphic_allocator<std::uint8_t>(std::pmr::new_delete_resource())));
        auto const alloc = typename S::allocator_type(std::pmr::polymorphic_allocator<std::uint8_t>(&adopter));

        auto const s = S(xstd::from_bit_storage, std::move(source).extract(), alloc);
        BOOST_CHECK(s.get_allocator() == alloc);
        BOOST_CHECK(std::ranges::equal(s, keys));
}

// Blocks adopted under the allocator that made them keep their storage: the buffer handed back is the one given.
BOOST_AUTO_TEST_CASE_TEMPLATE(AdoptionUnderAnEqualAllocatorKeepsTheStorage, S, polymorphic_owners)
{
        auto maker = std::pmr::unsynchronized_pool_resource();
        auto const keys = std::vector<std::size_t>{0, 9, 100, 700};
        auto const alloc = typename S::allocator_type(std::pmr::polymorphic_allocator<std::uint8_t>(&maker));
        auto blocks = S(keys.begin(), keys.end(), alloc).extract();
        auto const* const data = blocks.data();

        auto s = S(xstd::from_bit_storage, std::move(blocks), alloc);
        BOOST_CHECK(std::ranges::equal(s, keys));
        auto const handed_back = std::move(s).extract();
        BOOST_CHECK(handed_back.data() == data);
}

BOOST_AUTO_TEST_SUITE_END()
