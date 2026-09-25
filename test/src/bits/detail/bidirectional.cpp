//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                      // graded_extents
#include <test/value_reference.hpp>                  // value_reference
#include <xstd/bits/bit_set_view.hpp>                // bit_set_view
#include <xstd/bits/detail/bidirectional.hpp>        // bidirectional_bit_iterator, bidirectional_bit_reference
#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                     // array
#include <concepts>                                  // bidirectional_iterator, same_as
#include <cstddef>                                   // size_t
#include <cstdint>                                   // uint64_t
#include <iterator>                                  // next, prev
#include <set>                                       // set
#include <type_traits>                               // is_assignable_v, is_convertible_v, is_trivially_destructible_v
#include <utility>                                   // declval

namespace {

// Strong types to receive what the proxy converts to: one that takes a size_t implicitly, one only explicitly.
struct key
{
        std::size_t value;

        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr explicit(false) key(std::size_t v) noexcept
                : value(v)
        {}
};

struct index
{
        std::size_t value;

        constexpr explicit index(std::size_t v) noexcept
                : value(v)
        {}
};

template<class T>
[[nodiscard]] auto make(T const& empty, std::set<std::size_t> const& model)
        -> T
{
        auto c = empty;
        for (auto const p : model) {
                c.set(p);
        }
        return c;
}

template<class Iterator>
auto check_set_steps(Iterator first, Iterator last, std::set<std::size_t> const& model) -> void;

// A zero width has nothing to step over, so nothing below the two positions is instantiated for it.
template<class T>
auto check_set_walk(T const& empty, std::set<std::size_t> const& model)
        -> void
{
        using iterator = xstd::bits::detail::bidirectional_bit_iterator<T>;
        auto const c = make(empty, model);
        auto const size = c.size();

        auto const first = iterator(&c, model.empty() ? size : *model.begin());
        auto const last = iterator(&c, size);
        BOOST_CHECK((first == last) == model.empty());

        // Behind if constexpr rather than an early return, or MSVC reports the rest unreachable at a zero width.
        if constexpr (T::extent != 0UZ) {
                check_set_steps(first, last, model);
        }
}

template<class Iterator>
auto check_set_steps(Iterator first, Iterator last, std::set<std::size_t> const& model)
        -> void
{
        auto forward = std::set<std::size_t>();
        for (auto it = first; it != last; ++it) {
                BOOST_CHECK(&*it == it);
                forward.insert(*it);
                key const k = *it;
                BOOST_CHECK_EQUAL(k.value, static_cast<std::size_t>(*it));
                BOOST_CHECK_EQUAL(index(*it).value, k.value);
        }
        BOOST_CHECK(forward == model);

        auto backward = std::set<std::size_t>();
        auto it = last;
        for (auto n = model.size(); n != 0UZ; --n) {
                --it;
                backward.insert(*it);
        }
        BOOST_CHECK(backward == model);

        // The postfix forms step the same way and hand back where they were.
        if (not model.empty()) {
                it = first;
                auto const was = it++;
                BOOST_CHECK(was == first);
                auto const back = it--;
                BOOST_CHECK(it == first);
                BOOST_CHECK(back == std::next(first));
        }
}

// Patterns rather than every subset: adjacent pairs put a set bit on both sides of every block boundary.
template<class T>
auto check_every_set_pattern(T const& empty)
        -> void
{
        check_set_walk(empty, {});

        // Behind if constexpr, or MSVC's analyzer reports loops whose body never runs at a zero width, which is so.
        if constexpr (T::extent != 0UZ) {
                auto const size = empty.size();
                auto full = std::set<std::size_t>();
                for (auto i = 0UZ; i < size; ++i) {
                        full.insert(i);
                }
                check_set_walk(empty, full);

                for (auto i = 0UZ; i < size; ++i) {
                        check_set_walk(empty, {i});
                        if (i + 1UZ < size) {
                                check_set_walk(empty, {i, i + 1UZ});
                        }
                }
        }
}

} // namespace

BOOST_AUTO_TEST_SUITE(Bidirectional)

using ArrayTypes = test::graded_extents<xstd::bits::detail::contiguous_bit_array>;

using Bits = xstd::bits::detail::contiguous_bit_array<std::uint64_t, 200>;

BOOST_AUTO_TEST_CASE(AnIteratorIsAPointerAndAPosition)
{
        constexpr auto two_words = 2UZ * sizeof(void*);

        static_assert(sizeof(xstd::bits::detail::bidirectional_bit_iterator<Bits>) == two_words);
        static_assert(sizeof(xstd::bits::detail::bidirectional_bit_reference<Bits>) == two_words);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetIteratorIsBidirectional, T, ArrayTypes)
{
        static_assert(std::bidirectional_iterator<xstd::bits::detail::bidirectional_bit_iterator<T>>);
}

// The set proxy never writes, so nothing distinguishes its const spelling.
BOOST_AUTO_TEST_CASE(TheSetProxyNeverWrites)
{
        static_assert(not std::is_assignable_v<xstd::bits::detail::bidirectional_bit_reference<Bits> const&, std::size_t>);
        static_assert(std::is_convertible_v<xstd::bits::detail::bidirectional_bit_reference<Bits>, std::size_t>);
        static_assert(std::is_convertible_v<xstd::bits::detail::bidirectional_bit_reference<Bits const>, std::size_t>);

        BOOST_CHECK(true);
}

// What a container's const_reference must be: trivially copyable, never assignable, comparable by value.
BOOST_AUTO_TEST_CASE(TheReadOnlyProxiesAreValues)
{
        static_assert(test::value_reference<xstd::bits::detail::bidirectional_bit_reference<Bits>>);
        static_assert(test::value_reference<xstd::bits::detail::bidirectional_bit_reference<Bits const>>);
        static_assert(std::is_trivially_destructible_v<xstd::bits::detail::bidirectional_bit_iterator<Bits>>);

        BOOST_CHECK(true);
}

// The storage keeps its own preconditions and the iterator is stepped within them.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetIteratorWalksThePositionsInBothDirections, T, ArrayTypes)
{
        check_every_set_pattern(T());
}

// format_as is what the proxy's own std::formatter calls unqualified, and what fmt would call, so the test calls it so.
BOOST_AUTO_TEST_CASE(TheProxyFormatsAsItsValue)
{
        auto c = Bits();
        c.set(42);

        BOOST_CHECK_EQUAL(format_as(*xstd::bits::detail::bidirectional_bit_iterator<Bits>(&c, 42UZ)), 42UZ);
}

BOOST_AUTO_TEST_SUITE_END()

// The set view hands out this proxy and nothing of its own.
BOOST_AUTO_TEST_SUITE(BidirectionalThroughTheView)

namespace {

using Viewed = xstd::bits::detail::contiguous_bit_array<std::uint64_t, 64>;

using SetIt = xstd::bits::detail::bidirectional_bit_iterator<Viewed>;
using SetRef = xstd::bits::detail::bidirectional_bit_reference<Viewed>;

// Dependent, so a type without the member is a substitution failure rather than a hard error.
template<class R>
constexpr bool has_address_of = requires (R r) { r.operator&(); };

} // namespace

BOOST_AUTO_TEST_CASE(TheViewIteratesWithTheSharedProxy)
{
        static_assert(std::same_as<xstd::bit_set_view<std::array<std::uint64_t, 1>>::iterator, SetIt>);
        static_assert(std::same_as<xstd::bit_set_view<std::array<std::uint64_t, 1>>::reference, SetRef>);

        BOOST_CHECK(true);
}

// One shape asked twice: * gives a proxy, & an iterator back, and only our own guarantees are asserted.
BOOST_AUTO_TEST_CASE(DereferencingYieldsAProxyRatherThanTheValue)
{
        static_assert(std::same_as<decltype(*std::declval<SetIt const&>()), SetRef>);
        static_assert(not std::same_as<decltype(*std::declval<SetIt const&>()), std::size_t>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AddressOfAProxyYieldsAnIterator)
{
        static_assert(std::same_as<decltype(&std::declval<SetRef const&>()), SetIt>);
        static_assert(has_address_of<SetRef>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheValueArrivesByImplicitConversion)
{
        static_assert(std::is_convertible_v<SetRef, std::size_t>);

        auto b = Viewed();
        auto const v = xstd::bit_set_view(b);
        v.insert({3, 5, 7});

        // No cast at either of these.
        std::size_t const key = *v.begin();
        BOOST_CHECK_EQUAL(key, 3UZ);
        BOOST_CHECK(*v.begin() == 3UZ);
}

// & . * and * . & are both the identity, which makes the pair a round trip rather than two one-way conversions.
BOOST_AUTO_TEST_CASE(TheProxyPairRoundTrips)
{
        auto b = Viewed();
        auto const v = xstd::bit_set_view(b);
        v.insert({3, 5, 7, 11});

        for (auto it = v.begin(); it != v.end(); ++it) {
                BOOST_CHECK(&*it == it);
                BOOST_CHECK(*&*it == *it);
        }
}

BOOST_AUTO_TEST_SUITE_END()
