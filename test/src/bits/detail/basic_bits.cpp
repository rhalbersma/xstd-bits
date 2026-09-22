//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>         // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/detail/basic_bits.hpp> // basic_bits
#include <xstd/bits/bit_array.hpp>         // basic_bit_array
#include <xstd/bits/bit_set.hpp>           // basic_bit_set
#include <xstd/bits/bit_static_set.hpp>    // basic_bit_static_set
#include <xstd/bits/bit_vector.hpp>        // basic_bit_vector
#include <xstd/bits/bitset.hpp>            // basic_bitset
#include <xstd/bits/dynamic_bitset.hpp>    // basic_dynamic_bitset
#include <xstd/bits/detail/grid.hpp>       // adaptor, bits_t
#include <xstd/bits/detail/ownership.hpp>  // owned_bits_t, storage, window
#include <xstd/bits/detail/tags.hpp>       // array_container_tag, bitset_reading_tag, inplace_vector_container_tag, sequence_reading_tag, set_reading_tag, vector_container_tag
#include <boost/test/unit_test.hpp>        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                        // derived_from, same_as
#include <cstddef>                         // size_t
#include <memory>                          // allocator
#include <span>                            // dynamic_extent

#ifdef TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/bit_inplace_set.hpp>    // basic_bit_inplace_set
#include <xstd/bits/bit_inplace_vector.hpp> // basic_bit_inplace_vector
#include <xstd/bits/inplace_bitset.hpp>     // basic_inplace_bitset
#endif

namespace {

using block_type = std::size_t;
using allocator_type = std::allocator<block_type>;
inline constexpr auto width = 100UZ;

// A cell reproduces a public name when the container tag picks its storage and the reading tag the adaptor it derives from.
template<class R, class C, class Row, std::size_t N = std::dynamic_extent, class Alloc = void>
concept reproduces =
        std::same_as<xstd::bits_t<C, block_type, N, Alloc>, xstd::owned_bits_t<Row>> and
        std::derived_from<Row, xstd::adaptor<R, xstd::owned_bits_t<Row>, xstd::storage::owned, xstd::window::all, Row>> and
        std::same_as<Row, xstd::basic_bits<R, C, block_type, N, Alloc>>;

} // namespace

BOOST_AUTO_TEST_SUITE(BasicBits)

BOOST_AUTO_TEST_CASE(TheSequenceRowIsThreeCells)
{
        static_assert(reproduces<xstd::sequence_reading_tag, xstd::array_container_tag, xstd::basic_bit_array<block_type, width>, width>);
        static_assert(reproduces<xstd::sequence_reading_tag, xstd::vector_container_tag, xstd::basic_bit_vector<block_type, allocator_type>, std::dynamic_extent, allocator_type>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(reproduces<xstd::sequence_reading_tag, xstd::inplace_vector_container_tag, xstd::basic_bit_inplace_vector<block_type, width>, width>);
#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheSetRowIsThreeCells)
{
        static_assert(reproduces<xstd::set_reading_tag, xstd::array_container_tag, xstd::basic_bit_static_set<block_type, width>, width>);
        static_assert(reproduces<xstd::set_reading_tag, xstd::vector_container_tag, xstd::basic_bit_set<block_type, allocator_type>, std::dynamic_extent, allocator_type>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(reproduces<xstd::set_reading_tag, xstd::inplace_vector_container_tag, xstd::basic_bit_inplace_set<block_type, width>, width>);
#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheBitsetRowIsThreeCells)
{
        static_assert(reproduces<xstd::bitset_reading_tag, xstd::array_container_tag, xstd::basic_bitset<block_type, width>, width>);
        static_assert(reproduces<xstd::bitset_reading_tag, xstd::vector_container_tag, xstd::basic_dynamic_bitset<block_type, allocator_type>, std::dynamic_extent, allocator_type>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(reproduces<xstd::bitset_reading_tag, xstd::inplace_vector_container_tag, xstd::basic_inplace_bitset<block_type, width>, width>);
#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(DistinctCellsAreDistinctTypes)
{
        // The public names are aliases, so two readings over one storage must still not collapse onto each other.
        static_assert(not std::same_as<xstd::basic_bit_array<block_type, width>, xstd::basic_bit_static_set<block_type, width>>);
        static_assert(not std::same_as<xstd::basic_bit_array<block_type, width>, xstd::basic_bitset<block_type, width>>);
        static_assert(not std::same_as<xstd::basic_bit_set<block_type>, xstd::basic_bit_vector<block_type>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(ACellIsUsableThroughEachReadingsOwnSurface)
{
        xstd::basic_bits<xstd::set_reading_tag, xstd::vector_container_tag, block_type> s;
        s.insert(42);
        BOOST_CHECK_EQUAL(s.size(), 1U);
        BOOST_CHECK(s.contains(42));

        xstd::basic_bits<xstd::sequence_reading_tag, xstd::array_container_tag, block_type, width> v;
        v[7] = true;
        BOOST_CHECK(v[7] == true);
        BOOST_CHECK_EQUAL(v.size(), width);

        xstd::basic_bits<xstd::bitset_reading_tag, xstd::array_container_tag, block_type, width> b;
        b.set(3);
        BOOST_CHECK(b.test(3));
        BOOST_CHECK_EQUAL(b.count(), 1U);
}

BOOST_AUTO_TEST_SUITE_END()
