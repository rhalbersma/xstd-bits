//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                         // all_block_types
#include <test/set/strong_index.hpp>                    // strong_index
#include <xstd/bits/bit_concepts/bit_index_mapping.hpp> // bit_index_mapping
#include <xstd/bits/bit_flag_mapping.hpp>               // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>                // bit_find_mapping, bit_key_mapping, bit_range_mapping
#include <boost/test/unit_test.hpp>                     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                        // array
#include <cstddef>                                      // byte, size_t
#include <cstdint>                                      // uint8_t
#include <filesystem>                                   // perm_options, perms
#include <future>                                       // launch
#include <limits>                                       // float_round_style, round_indeterminate

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitIndexMapping)

namespace {

// Sorted and gapped keys, found by search.
constexpr auto ids = std::array{3, 17, 40};

} // namespace

// The identity answers for every unsigned key.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheIdentityIsAMapping, Key, test::all_block_types)
{
        static_assert(xstd::bit_index_mapping<xstd::bit_key_mapping<Key>, Key>);

        BOOST_CHECK(true);
}

// A range, a list of keys and a bitmask type's own flags each place their keys as the identity does.
BOOST_AUTO_TEST_CASE(TheRangeFindAndFlagMappingsAreMappings)
{
        static_assert(xstd::bit_index_mapping<xstd::bit_range_mapping<std::float_round_style, std::round_indeterminate, 5UZ>, std::float_round_style>);
        static_assert(xstd::bit_index_mapping<xstd::bit_find_mapping<int, ids>, int>);
        static_assert(xstd::bit_index_mapping<xstd::bit_flag_mapping<std::launch>, std::launch>);
        static_assert(xstd::bit_index_mapping<xstd::bit_flag_mapping<std::byte>, std::byte>);

        BOOST_CHECK(true);
}

// A user's mapping for a strong index models the concept as the shipped ones do.
BOOST_AUTO_TEST_CASE(AUsersMappingModelsTheConcept)
{
        static_assert(xstd::bit_index_mapping<xstd::bit_key_mapping<test::set::strong_index>, test::set::strong_index>);

        BOOST_CHECK(true);
}

// A mapping answers for its own key type only, however alike another is in width, signedness or header.
BOOST_AUTO_TEST_CASE(AMappingForAnotherKeyIsNone)
{
        static_assert(not xstd::bit_index_mapping<xstd::bit_key_mapping<std::size_t>, unsigned char>);
        static_assert(not xstd::bit_index_mapping<xstd::bit_range_mapping<int, -50, 100UZ>, long>);
        static_assert(not xstd::bit_index_mapping<xstd::bit_flag_mapping<std::byte>, std::uint8_t>);
        static_assert(not xstd::bit_index_mapping<xstd::bit_flag_mapping<std::filesystem::perms>, std::filesystem::perm_options>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
