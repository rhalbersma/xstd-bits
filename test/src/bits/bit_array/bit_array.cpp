//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                    // graded_extents
#include <test/sequence/dense.hpp>                 // yields_every_position
#include <test/sequence/rotation.hpp>              // permutation_sweep, permutes_ten_bits
#include <test/value_reference.hpp>                // value_reference
#include <xstd/bits/bit_algorithm/bit_reverse.hpp> // bit_reverse
#include <xstd/bits/bit_algorithm/bit_rotate.hpp>  // bit_rotate
#include <xstd/bits/bit_array.hpp>                 // basic_bit_array, bit_array
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                               // count, none_of
#include <concepts>                                // regular, same_as, totally_ordered
#include <cstddef>                                 // ptrdiff_t, size_t
#include <cstdint>                                 // uint32_t, uint64_t, uint8_t
#include <functional>                              // identity
#include <iterator>                                // contiguous_iterator, random_access_iterator, size
#include <ranges>                                  // begin, contiguous_range, empty, iota, random_access_range, size
#include <tuple>                                   // tuple_cat, tuple_size_v
#include <type_traits>                             // bool_constant, integral_constant, is_trivially_default_constructible_v
#include <utility>                                 // declval, index_sequence, make_index_sequence

BOOST_AUTO_TEST_SUITE(BitArray)

// Every Block model within one block, the narrow ones across boundaries, and the widest across one too.
using Types = decltype(std::tuple_cat(
        std::declval<test::graded_extents<xstd::basic_bit_array>>(),
        std::declval<test::wide_extents<xstd::basic_bit_array>>()
));

// The clauses one at a time, so a failure names which one; the umbrella asserts the composite.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsTotallyOrdered, T, Types)
{
        static_assert(std::totally_ordered<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsARandomAccessRange, T, Types)
{
        static_assert(std::ranges::random_access_range<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsIteratorIsRandomAccess, T, Types)
{
        using I = T::iterator;
        static_assert(std::random_access_iterator<I>);
}

// Random access is where it stops: the blocks are contiguous, the bits are not addressable.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItIsNotAContiguousRange, T, Types)
{
        static_assert(not std::ranges::contiguous_range<T>);
        static_assert(not std::contiguous_iterator<typename T::iterator>);
}

// operator& on the proxy answers an iterator, not a pointer, so the identity holds in iterator arithmetic.
BOOST_AUTO_TEST_CASE_TEMPLATE(AddressOfASubscriptIsTheIteratorToIt, T, Types)
{
        static_assert(std::same_as<decltype(&std::declval<T&>()[0UZ]), typename T::iterator>);

        auto a = T();
        for (auto const n : std::views::iota(0UZ, a.size())) {
                auto const step = static_cast<std::ptrdiff_t>(n);
                BOOST_CHECK(&a[n] == &a[0UZ] + step);
                BOOST_CHECK(&a[n] == std::ranges::begin(a) + step);
                BOOST_CHECK(static_cast<bool>(*(&a[n])) == static_cast<bool>(a[n]));
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsConstReferenceIsAValue, T, Types)
{
        static_assert(test::value_reference<typename T::const_reference>);
}

// The width is the type's: T::size names it as a value, a.size() answers size_type, and std::size reads it as before.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItsSizesAreConstantsOfItsType, T, Types)
{
        constexpr auto N = std::tuple_size_v<T>;
        static_assert(std::same_as<decltype(T::size), std::integral_constant<std::size_t, N> const>);
        static_assert(std::same_as<decltype(T::empty), std::bool_constant<N == 0UZ> const>);
        static_assert(std::same_as<decltype(T::max_size), std::integral_constant<std::size_t, N> const>);

        static_assert(std::same_as<decltype(std::declval<T const&>().size()), typename T::size_type>);
        static_assert(std::same_as<decltype(std::declval<T const&>().empty()), bool>);
        static_assert(std::same_as<decltype(std::declval<T const&>().max_size()), typename T::size_type>);
        static_assert(noexcept(std::declval<T const&>().size()));
        static_assert(noexcept(std::declval<T const&>().empty()));
        static_assert(noexcept(std::declval<T const&>().max_size()));

        static_assert(T::size == N);
        static_assert(T::empty == (N == 0UZ));
        static_assert(T::max_size == N);
        auto const a = T();
        BOOST_CHECK_EQUAL(std::ranges::size(a), N);
        BOOST_CHECK_EQUAL(std::size(a), N);
        BOOST_CHECK_EQUAL(a.size(), N);
        BOOST_CHECK_EQUAL(std::ranges::empty(a), N == 0UZ);
}

// Every position, densely, agreeing with the subscript -- and not a contiguous range, which no proxy sequence can be.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItYieldsEveryPosition, T, Types)
{
        auto c = T();
        test::sequence::yields_every_position(c);

        for (auto const n : std::views::iota(0UZ, c.size())) {
                c[n] = (n % 3UZ == 0UZ);
        }
        test::sequence::yields_every_position(c);
}

namespace {

// One sweep per width, each width its own instantiation of the storage's permutations.
template<class Block, std::size_t... N>
[[nodiscard]] auto permutation_sweeps(std::index_sequence<N...>)
        -> int
{
        return (0 + ... + test::sequence::permutation_sweep(xstd::basic_bit_array<Block, N>()));
}

} // namespace

// bit_rotate and bit_reverse over it throw nothing.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItsRotationAndReversalThrowNothing, T, Types)
{
        static_assert(noexcept(xstd::ranges::bit_rotate(std::declval<T&>(), std::declval<T&>().begin())));
        static_assert(noexcept(xstd::ranges::bit_reverse(std::declval<T&>())));
        BOOST_CHECK_EQUAL(test::sequence::permutation_sweep(T()), 0);
}

// Every width to seventeen, in up to three narrow blocks and in one wider block.
BOOST_AUTO_TEST_CASE(ItRotatesAndReversesAsTheAlgorithmsDoAtEveryNarrowWidth)
{
        static_assert(test::sequence::permutes_ten_bits(xstd::basic_bit_array<std::uint8_t, 10>()));
        BOOST_CHECK_EQUAL(permutation_sweeps<std::uint8_t>(std::make_index_sequence<18>()), 0);
        BOOST_CHECK_EQUAL(permutation_sweeps<std::uint32_t>(std::make_index_sequence<18>()), 0);
}

// Whole and partial blocks past one: a turn by whole blocks, by a part of one, and around the padding.
BOOST_AUTO_TEST_CASE(ItRotatesAndReversesAsTheAlgorithmsDoAcrossManyBlocks)
{
        BOOST_CHECK_EQUAL(test::sequence::permutation_sweep(xstd::basic_bit_array<std::uint8_t, 64>()), 0);
        BOOST_CHECK_EQUAL(test::sequence::permutation_sweep(xstd::basic_bit_array<std::uint8_t, 70>()), 0);
        BOOST_CHECK_EQUAL(test::sequence::permutation_sweep(xstd::basic_bit_array<std::uint64_t, 128>()), 0);
        BOOST_CHECK_EQUAL(test::sequence::permutation_sweep(xstd::basic_bit_array<std::uint64_t, 130>()), 0);
}

// As std::array<bool, N>: a; default-initializes, a{} is all false, and a width that fills its blocks writes nothing.
BOOST_AUTO_TEST_CASE(DefaultInitializationIsArraysAndValueInitializationIsFalse)
{
        static_assert(std::is_trivially_default_constructible_v<xstd::bit_array<64>>);
        static_assert(std::is_trivially_default_constructible_v<xstd::basic_bit_array<std::uint8_t, 24>>);
        static_assert(not std::is_trivially_default_constructible_v<xstd::bit_array<100>>);
        static_assert([] {
                xstd::bit_array<64> a{};
                return std::ranges::none_of(a, std::identity());
        }());
        xstd::bit_array<64> const a{};
        BOOST_CHECK(std::ranges::none_of(a, std::identity()));
}

// Every element written one at a time is a value, at a padded width too: the bits past N stay clear for count().
BOOST_AUTO_TEST_CASE(WritingEveryElementGivesTheArrayItsValue)
{
        auto const fill_odd = []<class A>(A& a) -> std::size_t {
                for (auto const n : std::views::iota(0UZ, a.size())) {
                        a[n] = (n % 2UZ == 1UZ);
                }
                return static_cast<std::size_t>(std::ranges::count(a, true));
        };
        xstd::bit_array<64> whole;
        xstd::bit_array<100> padded;
        BOOST_CHECK_EQUAL(fill_odd(whole), 32UZ);
        BOOST_CHECK_EQUAL(fill_odd(padded), 50UZ);
}

BOOST_AUTO_TEST_SUITE_END()
