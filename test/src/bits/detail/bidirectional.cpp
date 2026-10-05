//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/array_storage.hpp>                   // array_storage
#include <test/block_types.hpp>                     // all_block_types, digits_v, graded_extents
#include <test/ext_int128.hpp>                      // TEST_HAS_ABSL_INT128, TEST_HAS_BOOST_INT128, uint128
#include <test/for_each_type.hpp>                   // for_each_type
#include <test/value_reference.hpp>                 // value_reference
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set
#include <xstd/bits/bit_key_traits.hpp>             // bit_key_traits
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/detail/bidirectional.hpp>       // bidirectional_bit_iterator, bidirectional_bit_reference
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                    // array
#include <concepts>                                 // bidirectional_iterator, equality_comparable, same_as, totally_ordered, totally_ordered_with
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <iterator>                                 // iter_reference_t, iter_value_t, next, prev
#include <optional>                                 // optional
#include <ranges>                                   // iota
#include <set>                                      // set
#include <type_traits>                              // is_assignable_v, is_constructible_v, is_convertible_v, is_trivially_destructible_v
#include <utility>                                  // declval

namespace {

// A key the proxy hands out through its traits, and a strong type that takes the size_t only explicitly.
struct key
{
        std::size_t value;
};

struct key_traits
{
        [[nodiscard]] static constexpr auto to_index(key k) noexcept
                -> std::size_t
        {
                return k.value;
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t i) noexcept
                -> key
        {
                return {.value = i};
        }
};

// A class implicitly constructible from the key, which the proxy direct-initializes and never copy-initializes.
struct holder
{
        key held;

        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr explicit(false) holder(key k) noexcept
                : held(k)
        {}
};

struct index
{
        std::size_t value;

        constexpr explicit index(std::size_t v) noexcept
                : value(v)
        {}
};

// A user's namespace, associated with the proxy through its key traits, declaring a comparison of its own.
namespace user {

struct flag
{
        std::size_t value;

        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr explicit(false) flag(std::size_t v) noexcept
                : value(v)
        {}
};

[[nodiscard]] constexpr auto operator==(flag lhs, flag rhs) noexcept
        -> bool
{
        return lhs.value == rhs.value;
}

struct key_traits : xstd::bit_key_traits<std::size_t>
{};

} // namespace user

// One full block of each Block, which is all a comparison between two of its proxies asks for.
template<class Block>
using block_bits = test::array_storage<Block, test::digits_v<Block>>;

template<class Block>
using block_reference = xstd::bits::detail::bidirectional_bit_reference<block_bits<Block>>;

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
        auto c       = make(empty, model);
        auto const v = xstd::bit_set_view(c);

        auto const first = v.begin();
        auto const last  = v.end();
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
                std::size_t const k = *it;
                BOOST_CHECK_EQUAL(index(*it).value, k);
        }
        BOOST_CHECK(forward == model);

        auto backward = std::set<std::size_t>();
        auto it       = last;
        for (auto n = model.size() - 1UZ; n < model.size(); --n) {
                --it;
                backward.insert(*it);
        }
        BOOST_CHECK(backward == model);

        // The postfix forms step the same way and hand back where they were.
        if (not model.empty()) {
                it             = first;
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
                auto full       = std::set<std::size_t>();
                for (auto const i : std::views::iota(0UZ, size)) {
                        full.insert(i);
                }
                check_set_walk(empty, full);

                for (auto const i : std::views::iota(0UZ, size)) {
                        check_set_walk(empty, {i});
                        if (i + 1UZ < size) {
                                check_set_walk(empty, {i, i + 1UZ});
                        }
                }
        }
}

} // namespace

BOOST_AUTO_TEST_SUITE(Bidirectional)

using ArrayTypes = test::graded_extents<test::array_storage>;

using Bits = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 4>, 200>;

BOOST_AUTO_TEST_CASE(AnIteratorIsAPointerAndAPosition)
{
        constexpr auto two_pointers = 2UZ * sizeof(void*);

        static_assert(sizeof(xstd::bits::detail::bidirectional_bit_iterator<Bits>) == two_pointers);
        static_assert(sizeof(xstd::bits::detail::bidirectional_bit_reference<Bits>) == two_pointers);
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

        BOOST_CHECK_EQUAL(format_as(*xstd::bit_set_view(c).begin()), 42UZ);
}

// The key arrives through the traits, in one implicit step; a type the traits do not name is no conversion.
BOOST_AUTO_TEST_CASE(TheProxyConvertsToTheKeyItsTraitsName)
{
        using iterator = xstd::bits::detail::bidirectional_bit_iterator<Bits, key, key_traits>;
        static_assert(std::same_as<std::iter_value_t<iterator>, key>);
        static_assert(std::bidirectional_iterator<iterator>);
        static_assert(not std::is_convertible_v<std::iter_reference_t<iterator>, std::size_t>);

        auto s = xstd::basic_bit_fixed_set<key, std::uint64_t, 200, key_traits>();
        s.insert(key{42});
        static_assert(std::same_as<decltype(s.begin()), iterator>);
        key const k = *s.begin();
        BOOST_CHECK_EQUAL(k.value, 42UZ);
        BOOST_CHECK_EQUAL(format_as(*s.begin()).value, 42UZ);
        BOOST_CHECK_EQUAL(key_traits::to_index(*s.begin()), 42UZ);
        static_assert(not std::is_convertible_v<std::iter_reference_t<iterator>, holder>);
        static_assert(std::is_constructible_v<holder, std::iter_reference_t<iterator>>);
        holder const h(*s.begin());
        BOOST_CHECK_EQUAL(h.held.value, 42UZ);
}

// Every comparison is the key's own, reached through the one conversion, whatever the Block.
BOOST_AUTO_TEST_CASE(TheProxyComparesThroughItsOneConversion)
{
        test::for_each_type<test::all_block_types>([]<class Block> -> void {
                using reference  = block_reference<Block>;
                using value_type = reference::value_type;

                static_assert(std::equality_comparable<reference>);
                static_assert(std::totally_ordered<reference>);
                static_assert(std::totally_ordered_with<reference, value_type>);

                static_assert(not std::is_convertible_v<reference, user::flag>);
                static_assert(std::is_constructible_v<user::flag, reference>);
                static_assert(std::is_convertible_v<reference, std::optional<value_type>>);

                // A user's non-template operator== in an associated namespace is no second reading of ours.
                using user_reference = xstd::bits::detail::bidirectional_bit_reference<block_bits<Block>, std::size_t, user::key_traits>;
                static_assert(std::equality_comparable<user_reference>);
                static_assert(std::totally_ordered<user_reference>);
        });

        static_assert(std::totally_ordered_with<block_reference<std::uint8_t>, block_reference<std::uint64_t>>);

#if defined(TEST_HAS_ABSL_INT128) && defined(TEST_HAS_BOOST_INT128)

        static_assert(std::totally_ordered_with<block_reference<absl::uint128>, block_reference<boost::int128::uint128>>);

#endif

        auto narrow = block_bits<std::uint8_t>();
        auto wide   = block_bits<std::uint64_t>();
        narrow.set(3);
        wide.set(3);
        wide.set(5);
        auto const narrow_view = xstd::bit_set_view(narrow);
        auto const wide_view   = xstd::bit_set_view(wide);
        BOOST_CHECK(*narrow_view.begin() == *wide_view.begin());
        BOOST_CHECK(*narrow_view.begin() < *std::next(wide_view.begin()));
        BOOST_CHECK(user::flag(*narrow_view.begin()) == user::flag(3));
}

BOOST_AUTO_TEST_SUITE_END()

// The set view hands out this proxy and nothing of its own.
BOOST_AUTO_TEST_SUITE(BidirectionalThroughTheView)

namespace {

using Viewed = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 1>, 64>;

using SetIt  = xstd::bits::detail::bidirectional_bit_iterator<Viewed>;
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

        auto b       = Viewed();
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
        auto b       = Viewed();
        auto const v = xstd::bit_set_view(b);
        v.insert({3, 5, 7, 11});

        for (auto it = v.begin(); it != v.end(); ++it) {
                BOOST_CHECK(&*it == it);
                BOOST_CHECK(*&*it == *it);
        }
}

BOOST_AUTO_TEST_SUITE_END()
