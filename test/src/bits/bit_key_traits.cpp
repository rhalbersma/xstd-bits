//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/strong_index.hpp>        // offset_traits, strong_index
#include <xstd/bits/bit_key_traits.hpp>     // bit_key_traits
#include <xstd/bits/detail/set_adaptor.hpp> // admits_width
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                         // same_as
#include <cstddef>                          // size_t
#include <limits>                           // numeric_limits
#include <ranges>                           // iota

BOOST_AUTO_TEST_SUITE(BitKeyTraits)

namespace {

// A requires-expression on a concrete type is ill-formed rather than false ([expr.prim.req]/5).
template<class KeyTraits>
constexpr bool has_size = requires { KeyTraits::size; };

using identity = xstd::bit_key_traits<std::size_t>;

} // namespace

BOOST_AUTO_TEST_CASE(AStdSizeTKeyIsItsOwnPosition)
{
        static_assert(identity::to_index(0UZ) == 0UZ);
        static_assert(identity::from_index(0UZ) == 0UZ);
        static_assert(identity::to_index(std::numeric_limits<std::size_t>::max()) == std::numeric_limits<std::size_t>::max());
        static_assert(std::same_as<decltype(identity::from_index(0UZ)), std::size_t>);
        static_assert(noexcept(identity::to_index(0UZ)));
        static_assert(noexcept(identity::from_index(0UZ)));

        for (auto const i : std::views::iota(0UZ, 200UZ)) {
                BOOST_CHECK_EQUAL(identity::to_index(i), i);
                BOOST_CHECK_EQUAL(identity::from_index(i), i);
        }
}

// No size: a std::size_t key leaves the universe open, so an owner of any width or capacity takes it.
BOOST_AUTO_TEST_CASE(AStdSizeTKeyLeavesTheUniverseOpen)
{
        static_assert(not has_size<identity>);
        static_assert(xstd::bits::detail::set::admits_width<identity, 0UZ>);
        static_assert(xstd::bits::detail::set::admits_width<identity, 100UZ>);

        BOOST_CHECK(true);
}

// A strong index type specializes the default, and round-trips through its position preserving order.
BOOST_AUTO_TEST_CASE(AStrongIndexSpecializesTheDefault)
{
        using traits = xstd::bit_key_traits<test::set::strong_index>;
        static_assert(not has_size<traits>);

        for (auto const i : std::views::iota(0UZ, 200UZ)) {
                BOOST_CHECK(traits::from_index(i) == test::set::strong_index{.value = i});
                BOOST_CHECK_EQUAL(traits::to_index(traits::from_index(i)), i);
                BOOST_CHECK((traits::from_index(i) < traits::from_index(i + 1UZ)));
        }
}

// A traits type of its own may close the universe with a size, and map a key to a position other than its value.
BOOST_AUTO_TEST_CASE(ATraitsTypeOfItsOwnMayCloseTheUniverse)
{
        using traits = test::set::offset_traits<10UZ, 5UZ>;
        static_assert(has_size<traits>);
        static_assert(traits::size == 5UZ);

        // The owners' static_assert on the width: the size admits its own width and no other.
        static_assert(xstd::bits::detail::set::admits_width<traits, 5UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<traits, 4UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<traits, 6UZ>);

        BOOST_CHECK_EQUAL(traits::to_index({.value = 10UZ}), 0UZ);
        BOOST_CHECK(traits::from_index(4UZ) == test::set::strong_index{.value = 14UZ});
}

BOOST_AUTO_TEST_SUITE_END()
