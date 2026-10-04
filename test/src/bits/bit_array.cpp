//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>     // graded_extents
#include <test/sequence/dense.hpp>  // yields_every_position
#include <test/value_reference.hpp> // value_reference
#include <xstd/bits/bit_array.hpp>  // bit_array
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                 // constructible_from, convertible_to, regular, same_as, totally_ordered
#include <cstddef>                  // ptrdiff_t, size_t
#include <functional>               // hash
#include <iterator>                 // contiguous_iterator, random_access_iterator, size
#include <ranges>                   // begin, contiguous_range, empty, iota, random_access_range, size
#include <tuple>                    // tuple_cat, tuple_size_v
#include <type_traits>              // bool_constant, integral_constant, remove_cvref_t
#include <utility>                  // declval

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

        // A reference of unknown origin still names a constant, which a member function's answer is not.
        auto const through_reference = [](T const& a) -> void {
                static_assert(std::remove_cvref_t<decltype(a)>::size == N);
                static_assert(std::remove_cvref_t<decltype(a)>::empty == (N == 0UZ));
                static_assert(std::remove_cvref_t<decltype(a)>::max_size == N);
        };
        auto const a = T();
        through_reference(a);
        BOOST_CHECK_EQUAL(std::ranges::size(a), N);
        BOOST_CHECK_EQUAL(std::size(a), N);
        BOOST_CHECK_EQUAL(a.size(), N);
        BOOST_CHECK_EQUAL(std::ranges::empty(a), N == 0UZ);
}

// Every owner hashes, this one although std::array<bool, N> does not: equal values equal, at every extent.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItHashesAsAnOwner, T, Types)
{
        auto const h = std::hash<T>();
        BOOST_CHECK_EQUAL(h(T()), h(T()));
        if constexpr (T().size() > 0UZ) {
                auto x = T();
                x[0] = true;
                BOOST_CHECK(h(x) != h(T()));
        }
}

// A proxy, so a binding over the array writes through to it; by value it would bind to the copy.
BOOST_AUTO_TEST_CASE(AStructuredBindingWritesThroughToTheArray)
{
        auto a = xstd::bit_array<3>({true, false, true});
        auto& [x, y, z] = a;
        y = true;
        BOOST_CHECK(a[1] == true);
        BOOST_CHECK(x == true);
        BOOST_CHECK(z == true);
}

// A built-in array converts explicitly, position by position, and only one of the array's own width.
BOOST_AUTO_TEST_CASE(ABuiltInArrayOfItsWidthConverts)
{
        constexpr bool c[3] = {true, false, true}; // NOLINT(modernize-avoid-c-arrays): the built-in array is what is converted.
        bool const m[3] = {false, true, true};     // NOLINT(modernize-avoid-c-arrays): the built-in array is what is converted.
        auto const a = xstd::bit_array<3>(c);
        auto const b = xstd::bit_array<3>(m);
        BOOST_CHECK(a == xstd::bit_array<3>({true, false, true}));
        BOOST_CHECK(b == xstd::bit_array<3>({false, true, true}));
        static_assert(xstd::bit_array<3>(c) == xstd::to_bit_array(c));
        static_assert(not std::constructible_from<xstd::bit_array<3>, bool const(&)[2]>); // NOLINT(modernize-avoid-c-arrays)
        static_assert(not std::convertible_to<bool const(&)[3], xstd::bit_array<3>>);     // NOLINT(modernize-avoid-c-arrays)
        static_assert(not std::constructible_from<xstd::bit_array<0>, bool const(&)[1]>); // NOLINT(modernize-avoid-c-arrays)
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

BOOST_AUTO_TEST_SUITE_END()
