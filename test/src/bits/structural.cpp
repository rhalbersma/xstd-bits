//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>                   // for_each_type
#include <test/structural.hpp>                      // structural, value_parameter
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_blocks.hpp>                 // bit_align
#include <xstd/bits/bit_bounded_set.hpp>            // bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>              // bit_fixed_set
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/from_blocks.hpp>                // from_blocks
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <array>                                    // array
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t, uint64_t
#include <initializer_list>                         // initializer_list
#include <limits>                                   // numeric_limits
#include <tuple>                                    // tuple

BOOST_AUTO_TEST_SUITE(Structural)

namespace {

// One parameter of each owner's own type, spelled as a user writes it.
template<xstd::bit_align<xstd::bit_array<64>> V>
struct array_parameter
{};

template<xstd::bit_align<xstd::bit_fixed_set<64>> V>
struct set_parameter
{};

// Every owner bit_align fills to whole blocks, at one block and at two, and at a narrow block filled three times over.
using aligned_owners = std::tuple<
        xstd::bit_align<xstd::bit_array<64>>,
        xstd::bit_align<xstd::bit_array<128>>,
        xstd::bit_align<xstd::basic_bit_array<std::uint8_t, 24>>,
        xstd::bit_align<xstd::bit_fixed_set<64>>,
        xstd::bit_align<xstd::bit_fixed_set<128>>>;

// A width short of its last block by one bit and by all but one, at the machine word and at a byte.
using unaligned_owners = std::tuple<
        xstd::bit_array<3>,
        xstd::bit_array<65>,
        xstd::basic_bit_array<std::uint8_t, 9>,
        xstd::bit_fixed_set<3>,
        xstd::bit_fixed_set<127>>;

// Width zero holds no block at all, so it is aligned at every block and has no unused bit to keep clear.
using empty_owners = std::tuple<
        xstd::bit_array<0>,
        xstd::bit_align<xstd::bit_array<0>>,
        xstd::bit_fixed_set<0>>;

// A run-time width over storage that is not structural itself, and under Boost's static_vector not even literal.
using bounded_owners = std::tuple<
        xstd::bit_bounded_vector<64>,
        xstd::bit_bounded_set<64>>;

// Each reading's own door to a bit: a set inserts, and a sequence assigns through its proxy.
template<class T>
[[nodiscard]] constexpr auto with_bits(std::initializer_list<std::size_t> positions)
        -> T
{
        auto t = T();
        for (auto const pos : positions) {
                if constexpr (requires { t.insert(pos); }) {
                        t.insert(pos);
                } else {
                        t[pos] = true;
                }
        }
        return t;
}

// The block and the width each owner is instantiated with, which its blocks are built from.
template<class T>
struct shape_of;

template<class Block, std::size_t N>
struct shape_of<xstd::basic_bit_array<Block, N>>
{
        using block_type            = Block;
        static constexpr auto width = N;
};

template<class Block, std::size_t N>
struct shape_of<xstd::basic_bit_fixed_set<std::size_t, Block, N>>
{
        using block_type            = Block;
        static constexpr auto width = N;
};

// The same bits as blocks: the low bit of the first block and the high bit of the last one.
template<class T>
[[nodiscard]] constexpr auto from_words()
        -> T
{
        using block_type      = shape_of<T>::block_type;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);
        auto blocks           = std::array<block_type, shape_of<T>::width / digits>();
        blocks.front()        = static_cast<block_type>(1U);
        blocks.back() |= static_cast<block_type>(block_type{1U} << (digits - 1UZ));
        return T(xstd::from_blocks, blocks);
}

} // namespace

BOOST_AUTO_TEST_CASE(AnAlignedOwnerIsATemplateArgument)
{
        static_assert(std::same_as<array_parameter<xstd::bit_align<xstd::bit_array<64>>{true}>, array_parameter<xstd::bit_align<xstd::bit_array<64>>{true}>>);
        static_assert(std::same_as<set_parameter<xstd::bit_align<xstd::bit_fixed_set<64>>{0UZ}>, set_parameter<xstd::bit_align<xstd::bit_fixed_set<64>>{0UZ}>>);

        test::for_each_type<aligned_owners>([]<class T> -> void {
                // A constant T{} first: MSVC does not define a defaulted constructor for a requires-expression's sake.
                static_assert(T{} == T{});
                static_assert(test::structural<T>);
        });
}

BOOST_AUTO_TEST_CASE(EqualValuesNameTheSameSpecialization)
{
        test::for_each_type<aligned_owners>([]<class T> -> void {
                constexpr auto last     = shape_of<T>::width - 1UZ;
                constexpr auto by_bits  = with_bits<T>({0UZ, last});
                constexpr auto by_words = from_words<T>();
                static_assert(std::same_as<test::value_parameter<by_bits>, test::value_parameter<by_words>>);
                static_assert(std::same_as<test::value_parameter<T{}>, test::value_parameter<with_bits<T>({})>>);
        });
}

BOOST_AUTO_TEST_CASE(UnequalValuesNameDifferentSpecializations)
{
        test::for_each_type<aligned_owners>([]<class T> -> void {
                constexpr auto last = shape_of<T>::width - 1UZ;
                constexpr auto low  = with_bits<T>({0UZ});
                constexpr auto high = with_bits<T>({last});
                constexpr auto both = with_bits<T>({0UZ, last});
                static_assert(not std::same_as<test::value_parameter<T{}>, test::value_parameter<low>>);
                static_assert(not std::same_as<test::value_parameter<low>, test::value_parameter<high>>);
                static_assert(not std::same_as<test::value_parameter<high>, test::value_parameter<both>>);
        });
}

BOOST_AUTO_TEST_CASE(AnUnalignedWidthIsNotStructural)
{
        test::for_each_type<unaligned_owners>([]<class T> -> void {
                // A constant T{}, so that the answer is the type's and not the value's.
                static_assert(T{} == T{});
                static_assert(not test::structural<T>);
        });
}

BOOST_AUTO_TEST_CASE(WidthZeroIsStructural)
{
        test::for_each_type<empty_owners>([]<class T> -> void {
                // A constant T{} first: MSVC does not define a defaulted constructor for a requires-expression's sake.
                static_assert(T{} == T{});
                static_assert(test::structural<T>);
                static_assert(std::same_as<test::value_parameter<T{}>, test::value_parameter<with_bits<T>({})>>);
        });
}

BOOST_AUTO_TEST_CASE(ABoundedOwnerIsNotStructural)
{
        test::for_each_type<bounded_owners>([]<class T> -> void {
                static_assert(not test::structural<T>);
        });
}

// The vehicle alone: its blocks are public where they fill the width, and kept to it where a tail must stay clear.
BOOST_AUTO_TEST_CASE(TheVehicleIsStructuralExactlyWhereItsBlocksFillTheWidth)
{
        static_assert(test::structural<xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 2>, 128>>);
        static_assert(test::structural<xstd::bits::detail::bit_block_container<std::array<std::uint8_t, 3>, 24>>);
        static_assert(test::structural<xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 0>, 0>>);
        static_assert(not test::structural<xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 2>, 127>>);
        static_assert(not test::structural<xstd::bits::detail::bit_block_container<std::array<std::uint8_t, 1>, 0>>);
}

BOOST_AUTO_TEST_SUITE_END()
