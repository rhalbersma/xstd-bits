//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>  // bit_array
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <tuple>                    // tuple_element_t, tuple_size_v
#include <type_traits>              // is_same_v

BOOST_AUTO_TEST_SUITE(BitArrayTupleElement)

// A proxy, so a binding over the array writes through to it; by value it would bind to the copy.
BOOST_AUTO_TEST_CASE(AStructuredBindingWritesThroughToTheArray)
{
        auto a          = xstd::bit_array<3>({true, false, true});
        auto& [x, y, z] = a;
        y               = true;
        BOOST_CHECK(a[1] == true);
        BOOST_CHECK(x == true);
        BOOST_CHECK(z == true);
}

// [array.tuple]'s size and element, as the adaptor answers them, for the array and the const array.
BOOST_AUTO_TEST_CASE(ItsTupleSizeIsItsWidthAndItsElementsAreItsReferences)
{
        using T = xstd::bit_array<3>;
        static_assert(std::tuple_size_v<T> == 3UZ and std::tuple_size_v<T const> == 3UZ);
        static_assert(std::is_same_v<std::tuple_element_t<0, T>, T::reference>);
        static_assert(std::is_same_v<std::tuple_element_t<2, T const>, T::const_reference>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
