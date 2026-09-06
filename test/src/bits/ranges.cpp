//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE
#include <xstd/bits/bit_proxy.hpp>      // bit_sequence_iterator, bit_sequence_reference, bit_set_iterator, bit_set_reference
#include <xstd/bits/ext/std/bitset.hpp> // the trait that makes std::bitset viewable
#include <xstd/bits/ranges.hpp>         // sequence_view, set_view
#include <bitset>                       // bitset
#include <concepts>                     // same_as
#include <cstddef>                      // size_t
#include <type_traits>                  // is_assignable_v, is_convertible_v
#include <utility>                      // declval

BOOST_AUTO_TEST_SUITE(Ranges)

namespace {

using Bits = std::bitset<64>;

using SetIt   = xstd::bit_set_iterator<Bits>;
using SetRef  = xstd::bit_set_reference<Bits>;
using ArrIt   = xstd::bit_sequence_iterator<Bits>;
using ArrRef  = xstd::bit_sequence_reference<Bits>;

// Dependent, so a type without the member is a substitution failure rather than a hard error.
template<class R>
constexpr bool has_address_of = requires(R r) { r.operator&(); };

}       // namespace

// The two views hand out the proxies of bit_proxy.hpp and nothing of their own. [design.md#the-iterator-is-the-primitive]
BOOST_AUTO_TEST_CASE(TheViewsIterateWithTheSharedProxies)
{
        static_assert(std::same_as<xstd::set_view<Bits>::iterator,       SetIt>);
        static_assert(std::same_as<xstd::set_view<Bits>::reference,      SetRef>);
        static_assert(std::same_as<xstd::sequence_view<Bits>::iterator,  ArrIt>);
        static_assert(std::same_as<xstd::sequence_view<Bits>::reference, ArrRef>);
}

// One shape asked twice: * gives a proxy, & gives an iterator back, and the value comes only by converting; the standard says nothing here, so only our own guarantees are asserted.
BOOST_AUTO_TEST_CASE(DereferencingYieldsAProxyRatherThanTheValue)
{
        static_assert(std::same_as<decltype(*std::declval<SetIt const&>()), SetRef>);
        static_assert(std::same_as<decltype(*std::declval<ArrIt const&>()), ArrRef>);

        static_assert(not std::same_as<decltype(*std::declval<SetIt const&>()), std::size_t>);
        static_assert(not std::same_as<decltype(*std::declval<ArrIt const&>()), bool>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AddressOfAProxyYieldsAnIterator)
{
        static_assert(std::same_as<decltype(&std::declval<SetRef const&>()), SetIt>);
        static_assert(std::same_as<decltype(&std::declval<ArrRef const&>()), ArrIt>);

        static_assert(has_address_of<SetRef>);
        static_assert(has_address_of<ArrRef>);

        BOOST_CHECK(true);
}

// The const path is a proxy too, the same one minus the assignment -- not the plain bool libstdc++ hands back.
BOOST_AUTO_TEST_CASE(TheConstPathIsAProxyAsWell)
{
        using ConstArrRef = xstd::bit_sequence_reference<Bits const>;

        static_assert(std::same_as<xstd::sequence_view<Bits const>::reference, ConstArrRef>);
        static_assert(std::is_convertible_v<ConstArrRef, bool>);
        static_assert(has_address_of<ConstArrRef>);
        static_assert(std::same_as<decltype(&std::declval<ConstArrRef const&>()), xstd::bit_sequence_iterator<Bits const>>);

        // and it is exactly the assignment that the const one drops.
        static_assert(    std::is_assignable_v<ArrRef const&, bool>);
        static_assert(not std::is_assignable_v<ConstArrRef const&, bool>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheValueArrivesByImplicitConversion)
{
        static_assert(std::is_convertible_v<SetRef, std::size_t>);
        static_assert(std::is_convertible_v<ArrRef, bool>);

        auto b = Bits();
        auto const v = xstd::set_view(b);
        v.insert({3, 5, 7});

        // No cast at any of these.
        std::size_t const key = *v.begin();
        BOOST_CHECK_EQUAL(key, 3UZ);
        BOOST_CHECK(*v.begin() == 3UZ);

        auto const a = xstd::sequence_view(b);
        bool const bit = a[3];
        BOOST_CHECK(bit);
        BOOST_CHECK(a[5] == true);
        BOOST_CHECK(a[4] == false);
}

// & . * and * . & are both the identity, which makes the pair a round trip rather than two one-way conversions.
BOOST_AUTO_TEST_CASE(TheProxyPairRoundTrips)
{
        auto b = Bits();
        auto const v = xstd::set_view(b);
        v.insert({3, 5, 7, 11});

        for (auto it = v.begin(); it != v.end(); ++it) {
                BOOST_CHECK(&*it == it);
                BOOST_CHECK(*&*it == *it);
        }

        auto const a = xstd::sequence_view(b);
        for (auto it = a.begin(); it != a.end(); ++it) {
                BOOST_CHECK(&*it == it);
                BOOST_CHECK(static_cast<bool>(*&*it) == static_cast<bool>(*it));
        }

        // and writing goes through the reference the round trip hands back.
        *&a.begin()[4] = true;
        BOOST_CHECK(b.test(4));
}

BOOST_AUTO_TEST_SUITE_END()
