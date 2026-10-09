//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                               // all_block_types
#include <test/set/enums.hpp>                                 // perm, piece
#include <test/set/strong_index.hpp>                          // offset_mapping, strong_index
#include <xstd/bits/bit_concepts/bit_index_mapping.hpp>       // bit_index_mapping
#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/bit_flag_mapping.hpp>                     // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>                      // bit_find_mapping, bit_key_mapping, bit_range_mapping
#include <xstd/bits/detail/is_key.hpp>                        // is_key
#include <boost/test/unit_test.hpp>                           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                              // array
#include <bit>                                                // bit_cast
#include <cstddef>                                            // size_t
#include <cstdint>                                            // int8_t, uint8_t

// Declared and never defined: a concept reads their declarations, and an unnamed namespace would warn of the unused.
namespace nonmapping {

// A size with no positions behind it, so no mapping either.
struct size_only
{
        static constexpr std::size_t size = 8;
};

// A mapping with a size that cannot say which values are keys, so it closes no universe.
struct size_without_is_key
{
        static constexpr std::size_t size = 8;

        [[nodiscard]] static auto to_index(std::size_t key) noexcept -> std::size_t;
        [[nodiscard]] static auto from_index(std::size_t index) noexcept -> std::size_t;
};

} // namespace nonmapping

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(SizedBitIndexMapping)

namespace {

// A bitmask enumeration, each enumerator one bit.
enum class mode : std::uint8_t
{
        read  = 0x01,
        write = 0x02,
};

// Sorted and gapped keys, found by search.
constexpr auto ids = std::array{3, 17, 40};

// A key whose arithmetic is its underlying type's, in a range that starts below zero.
enum class storey : std::int8_t
{
        basement = -3,
        roof     = 4,
};

} // namespace

// The identity answers for every unsigned key and leaves the universe open.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheIdentityIsAnUnsizedMapping, Key, test::all_block_types)
{
        static_assert(not xstd::sized_bit_index_mapping<xstd::bit_key_mapping<Key>, Key>);

        BOOST_CHECK(true);
}

// A range, a list of keys, a listed enumeration and a bitmask enumeration each close the universe at a size.
BOOST_AUTO_TEST_CASE(TheRangeFindEnumAndFlagMappingsAreSized)
{
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_range_mapping<int, -50, 100UZ>, int>);
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_range_mapping<storey, storey::basement, 8UZ>, storey>);
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_find_mapping<int, ids>, int>);
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_key_mapping<test::set::piece>, test::set::piece>);
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_key_mapping<test::set::perm>, test::set::perm>);
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_flag_mapping<mode>, mode>);

        BOOST_CHECK(true);
}

// A user's mapping for a strong index models the concept as the shipped ones do, sized or not.
BOOST_AUTO_TEST_CASE(AUsersMappingModelsTheConcept)
{
        static_assert(not xstd::sized_bit_index_mapping<xstd::bit_key_mapping<test::set::strong_index>, test::set::strong_index>);
        static_assert(xstd::sized_bit_index_mapping<test::set::offset_mapping<10UZ, 5UZ>, test::set::strong_index>);

        BOOST_CHECK(true);
}

// A size alone is no mapping, and a mapping with a size but no is_key closes no universe.
BOOST_AUTO_TEST_CASE(ASizeWithoutIsKeyIsNotSized)
{
        static_assert(not xstd::sized_bit_index_mapping<nonmapping::size_only, std::size_t>);
        static_assert(xstd::bit_index_mapping<nonmapping::size_without_is_key, std::size_t>);
        static_assert(not xstd::sized_bit_index_mapping<nonmapping::size_without_is_key, std::size_t>);

        BOOST_CHECK(true);
}

// Asked of any mapping, whether a value is a key is the mapping's own answer, and every value where it gives none.
BOOST_AUTO_TEST_CASE(AMappingWithoutIsKeyHoldsEveryValue)
{
        using range = xstd::bit_range_mapping<int, -50, 100UZ>;
        static_assert(noexcept(xstd::bits::detail::is_key<range>(0)));
        BOOST_CHECK(xstd::bits::detail::is_key<range>(-50) and not xstd::bits::detail::is_key<range>(50));
        BOOST_CHECK(xstd::bits::detail::is_key<xstd::bit_flag_mapping<mode>>(mode::write));
        BOOST_CHECK(not xstd::bits::detail::is_key<xstd::bit_flag_mapping<mode>>(std::bit_cast<mode>(std::uint8_t{0x03})));
        BOOST_CHECK(xstd::bits::detail::is_key<nonmapping::size_without_is_key>(1000UZ));
        BOOST_CHECK(xstd::bits::detail::is_key<xstd::bit_key_mapping<test::set::strong_index>>(test::set::strong_index{.value = 1000UZ}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
