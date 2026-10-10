//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                          // all_block_types
#include <xstd/bits/bit_array.hpp>                       // basic_bit_array
#include <xstd/bits/bit_bounded_set.hpp>                 // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>              // basic_bit_bounded_vector
#include <xstd/bits/bit_concepts/bit_convertible_to.hpp> // bit_convertible_to
#include <xstd/bits/bit_fixed_set.hpp>                   // basic_bit_fixed_set
#include <xstd/bits/bit_hasher.hpp>                      // bit_hasher
#include <xstd/bits/bit_set.hpp>                         // basic_bit_set
#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_span.hpp>                        // bit_span
#include <xstd/bits/bit_type_traits/bit_rebind.hpp>      // bit_rebind
#include <xstd/bits/bit_type_traits/bit_resize.hpp>      // bit_resize
#include <xstd/bits/bit_vector.hpp>                      // basic_bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>         // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp>      // basic_bit_small_vector
#include <xstd/bits/from_blocks.hpp>                     // from_blocks_t
#include <xstd/ints/cstdint/bit_int.hpp>                 // bit_uint
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_MESSAGE
#include <algorithm>                                     // min
#include <array>                                         // array
#include <concepts>                                      // constructible_from, copyable, default_initializable, equality_comparable, same_as, three_way_comparable
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <format>                                        // formattable
#include <functional>                                    // hash
#include <ranges>                                        // bidirectional_range, from_range_t, random_access_range
#include <string_view>                                   // string_view
#include <tuple>                                         // tuple, tuple_cat
#include <type_traits>                                   // conditional_t, remove_cv_t
#include <utility>                                       // declval
#include <vector>                                        // vector

BOOST_AUTO_TEST_SUITE(TypeMatrix)

namespace {

// Every block the library takes, and under Clang the bit-precise ones of a power-of-two width, which tile as any do.
#ifdef __BITINT_MAXWIDTH__
using matrix_block_types = decltype(std::tuple_cat(std::declval<test::all_block_types>(), std::declval<std::tuple<xstd::bit_uint<8>, xstd::bit_uint<64>, xstd::bit_uint<128>>>()));
#else
using matrix_block_types = test::all_block_types;
#endif

template<class T>
using value_of = std::remove_cv_t<T>;

template<class T>
concept sequence = std::same_as<typename value_of<T>::value_type, bool>;

// The other owner of the same reading and block, whose width is a run-time one: what a cross-type column asks about.
template<class Block, class T>
using other_owner_t = std::conditional_t<sequence<T>, xstd::basic_bit_vector<Block>, xstd::basic_bit_set<std::size_t, Block>>;

// The columns, each one capability a type has or lacks.
constexpr auto column_names = std::array<std::string_view, 19>{
        "default construction",
        "copy",
        "from_range construction",
        "from_blocks construction",
        "bit_convert into the other owner",
        "bit_convert from the other owner",
        "bit_rebind to std::uint8_t",
        "bit_resize to 8",
        "append_range of bools",
        "insert_range of keys",
        "&=, |=, ^= with its own type",
        "&= with the other owner",
        "equality_comparable",
        "three_way_comparable",
        "std::hash",
        "bit_hasher",
        "formattable",
        "random_access_range",
        "bidirectional_range",
};

template<class Block, class T>
[[nodiscard]] consteval auto capabilities() noexcept
        -> std::array<bool, std::size(column_names)>
{
        using U     = value_of<T>;
        using other = other_owner_t<Block, T>;
        return {
                std::default_initializable<T>,
                std::copyable<T>,
                std::constructible_from<T, std::from_range_t, std::vector<typename U::value_type> const&>,
                requires { typename U::block_container_type; requires std::constructible_from<T, xstd::from_blocks_t, typename U::block_container_type>; },
                xstd::bit_convertible_to<T const&, other>,
                xstd::bit_convertible_to<other const&, U>,
                requires { typename xstd::bit_rebind<std::uint8_t, U>; },
                requires { typename xstd::bit_resize<8, U>; },
                requires (T& t, std::vector<bool> const& r) { t.append_range(r); },
                requires (T& t, std::vector<typename U::value_type> const& r) { t.insert_range(r); },
                requires (T& t, U const& u) { t &= u; t |= u; t ^= u; },
                requires (T& t, other const& u) { t &= u; },
                std::equality_comparable<T>,
                std::three_way_comparable<T>,
                requires (T const& t) { std::hash<U>{}(t); },
                requires (T const& t) { xstd::bit_hasher<>{}(t); },
                std::formattable<U, char>,
                std::ranges::random_access_range<T>,
                std::ranges::bidirectional_range<T>,
        };
}

// One row, its columns spelled 1 for a capability the type has and . for one it lacks, checked column by column.
template<class Block, class T>
auto check_row(std::string_view name, std::string_view expected)
        -> void
{
        constexpr auto actual = capabilities<Block, T>();
        BOOST_CHECK_MESSAGE(std::size(expected) == std::size(actual), name << ": the row spells " << std::size(expected) << " columns");
        for (auto const i : std::views::iota(0UZ, std::ranges::min(std::size(expected), std::size(actual)))) {
                BOOST_CHECK_MESSAGE((expected[i] == '1') == actual[i], name << ": " << column_names[i] << (actual[i] ? " is there" : " is missing"));
        }
}

} // namespace

// Every container and view against every capability, at every block: what the type matrix audit settled, frozen.
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryTypeAnswersItsRow, Block, matrix_block_types)
{
        constexpr auto N = 40UZ;
        using array      = xstd::basic_bit_array<Block, N>;
        using vector     = xstd::basic_bit_vector<Block>;
        using fixed_set  = xstd::basic_bit_fixed_set<std::size_t, Block, N>;
        using set        = xstd::basic_bit_set<std::size_t, Block>;

        // Owners: a sequence takes bools and a set keys, and each combines and compares with its own type alone.
        check_row<Block, array>("bit_array", "11.11111..1.1111111");
        check_row<Block, xstd::basic_bit_bounded_vector<Block, N>>("bit_bounded_vector", "111111111.1.1111111");
        check_row<Block, xstd::basic_bit_small_vector<Block, N>>("bit_small_vector", "111111111.1.1111111");
        check_row<Block, vector>("bit_vector", "1111111.1.111111111");
        check_row<Block, fixed_set>("bit_fixed_set", "11111111.11.11111.1");
        check_row<Block, xstd::basic_bit_bounded_set<std::size_t, Block, N>>("bit_bounded_set", "11111111.11.11111.1");
        check_row<Block, xstd::basic_bit_small_set<std::size_t, Block, N>>("bit_small_set", "11111111.11.11111.1");
        check_row<Block, set>("bit_set", "1111111..11111111.1");

        // Sequence views: read out by bit_convert and combined with any source of their block, compared never, as span.
        using span = decltype(xstd::bit_span(std::declval<array&>()));
        check_row<Block, span>("bit_span over bit_array", ".1..1.....11....111");
        check_row<Block, decltype(xstd::bit_span(std::declval<array const&>()))>("bit_span over const bit_array", ".1..1...........111");
        check_row<Block, decltype(xstd::bit_span(std::declval<vector&>()))>("bit_span over bit_vector", ".1..1.....11....111");
        check_row<Block, decltype(std::declval<span&>().subspan(0UZ, 8UZ))>("bit_subspan", ".1..1.....11....111");

        // Set views: values as string_view is, combined with any set over the same storage.
        check_row<Block, decltype(xstd::bit_set_view(std::declval<fixed_set&>()))>("bit_set_view over bit_fixed_set", ".1..1....11.11111.1");
        check_row<Block, decltype(xstd::bit_set_view(std::declval<fixed_set const&>()))>("bit_set_view over const bit_fixed_set", ".1..1.......11111.1");
        check_row<Block, decltype(xstd::bit_set_view(std::declval<set&>()))>("bit_set_view over bit_set", ".1..1....11111111.1");
}

BOOST_AUTO_TEST_SUITE_END()
