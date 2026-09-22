//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                    // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/bitset_adaptor.hpp>               // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_array.hpp>  // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/grid.hpp>                         // adaptor, bits_t
#include <xstd/bits/ownership.hpp>                    // storage, window
#include <xstd/bits/sequence_adaptor.hpp>             // sequence_adaptor
#include <xstd/bits/set_adaptor.hpp>                  // set_adaptor
#include <xstd/bits/tags.hpp>                         // array_container_tag, bitset_reading_tag, inplace_vector_container_tag, sequence_reading_tag, set_reading_tag, vector_container_tag
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                                   // same_as
#include <cstddef>                                    // size_t
#include <memory>                                     // allocator
#include <memory_resource>                            // polymorphic_allocator
#include <span>                                       // dynamic_extent

#ifdef TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // contiguous_bit_inplace_vector
#endif

namespace {

using block_type = std::size_t;
using allocator_type = std::allocator<block_type>;
inline constexpr auto width = 100UZ;

// Whether a container tag names a storage at this extent at all, the primary being declared and never defined.
template<class C, std::size_t N>
concept has_storage = requires { typename xstd::bits_t<C, block_type, N>; };

} // namespace

BOOST_AUTO_TEST_SUITE(Grid)

BOOST_AUTO_TEST_CASE(TheContainerTagPicksTheStorage)
{
        static_assert(std::same_as<xstd::bits_t<xstd::array_container_tag, block_type, width>, xstd::detail::bits::contiguous_bit_array<block_type, width>>);
        static_assert(std::same_as<xstd::bits_t<xstd::vector_container_tag, block_type>, xstd::detail::bits::contiguous_bit_vector<block_type, allocator_type>>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(std::same_as<xstd::bits_t<xstd::inplace_vector_container_tag, block_type, width>, xstd::detail::bits::contiguous_bit_inplace_vector<block_type, width>>);
#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheAllocatorIsTheFourthArgumentAndTheDefaultIsStdAllocator)
{
        using other_allocator = std::pmr::polymorphic_allocator<block_type>;
        static_assert(not std::same_as<other_allocator, allocator_type>);
        static_assert(std::same_as<xstd::bits_t<xstd::vector_container_tag, block_type, std::dynamic_extent, other_allocator>, xstd::detail::bits::contiguous_bit_vector<block_type, other_allocator>>);
        static_assert(std::same_as<xstd::bits_t<xstd::vector_container_tag, block_type>, xstd::bits_t<xstd::vector_container_tag, block_type, std::dynamic_extent, allocator_type>>);

        // A void allocator is the container's own default, which is std::allocator here and need not be elsewhere.
        static_assert(std::same_as<xstd::bits_t<xstd::vector_container_tag, block_type, std::dynamic_extent, void>, xstd::detail::bits::contiguous_bit_vector<block_type, allocator_type>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AStaticExtentAskedOfAnAllocatingContainerIsANonMatch)
{
        // The extent is pinned in the specialization, so the argument is refused rather than silently dropped.
        static_assert(has_storage<xstd::vector_container_tag, std::dynamic_extent>);
        static_assert(not has_storage<xstd::vector_container_tag, width>);
        static_assert(has_storage<xstd::array_container_tag, width>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheReadingTagNamesOneCellOfTheGeneralTemplate)
{
        using bits = xstd::detail::bits::contiguous_bit_array<block_type, width>;
        static_assert(std::same_as<xstd::bitset_adaptor<bits, void>, xstd::adaptor<xstd::bitset_reading_tag, bits, xstd::storage::owned, xstd::window::all, void>>);
        static_assert(std::same_as<xstd::sequence_adaptor<bits, xstd::storage::owned, xstd::window::all, void>, xstd::adaptor<xstd::sequence_reading_tag, bits, xstd::storage::owned, xstd::window::all, void>>);
        static_assert(std::same_as<xstd::set_adaptor<bits, xstd::storage::owned, void>, xstd::adaptor<xstd::set_reading_tag, bits, xstd::storage::owned, xstd::window::all, void>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
