//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                         // all_block_types
#include <test/set/strong_index.hpp>                    // strong_index
#include <xstd/bits/bit_concepts/bit_index_mapping.hpp> // bit_index_mapping
#include <xstd/bits/bit_flag_mapping.hpp>               // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>                // bit_key_mapping, bit_range_mapping
#include <boost/test/unit_test.hpp>                     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstddef>                                      // size_t
#include <cstdint>                                      // int8_t, uint8_t

// Declared and never defined: a concept reads their declarations, and an unnamed namespace would warn of the unused.
namespace nonmapping {

// Positions out and no way back, so no mapping.
struct to_index_only
{
        [[nodiscard]] static auto to_index(std::size_t key) noexcept -> std::size_t;
};

// Positions back that are not the key, so no mapping for that key.
struct wrong_key
{
        [[nodiscard]] static auto to_index(std::size_t key) noexcept -> std::size_t;
        [[nodiscard]] static auto from_index(std::size_t index) noexcept -> int;
};

} // namespace nonmapping

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitIndexMapping)

namespace {

// A bitmask enumeration, each enumerator one bit.
enum class mode : std::uint8_t
{
        read  = 0x01,
        write = 0x02,
};

// A key whose arithmetic is its underlying type's, in a range that starts below zero.
enum class storey : std::int8_t
{
        basement = -3,
        roof     = 4,
};

} // namespace

// The identity answers for every unsigned key.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheIdentityIsAMapping, Key, test::all_block_types)
{
        static_assert(xstd::bit_index_mapping<xstd::bit_key_mapping<Key>, Key>);

        BOOST_CHECK(true);
}

// A user's mapping for a strong index models the concept as the shipped ones do.
BOOST_AUTO_TEST_CASE(AUsersMappingModelsTheConcept)
{
        static_assert(xstd::bit_index_mapping<xstd::bit_key_mapping<test::set::strong_index>, test::set::strong_index>);

        BOOST_CHECK(true);
}

// A mapping answers for one key type, from_index giving back exactly that key, and a type with no positions is none.
BOOST_AUTO_TEST_CASE(ANonMappingDoesNotModelTheConcept)
{
        static_assert(not xstd::bit_index_mapping<xstd::bit_key_mapping<std::size_t>, unsigned char>);
        static_assert(not xstd::bit_index_mapping<xstd::bit_range_mapping<int, -50, 100UZ>, long>);
        static_assert(not xstd::bit_index_mapping<xstd::bit_flag_mapping<mode>, storey>);
        static_assert(not xstd::bit_index_mapping<int, int>);
        static_assert(not xstd::bit_index_mapping<nonmapping::to_index_only, std::size_t>);
        static_assert(not xstd::bit_index_mapping<nonmapping::wrong_key, std::size_t>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
