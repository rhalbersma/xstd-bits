//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/array_storage.hpp>                   // array_storage
#include <test/block_types.hpp>                     // all_block_types, digits_v, graded_extents
#include <test/closed_proxy.hpp>                    // closed_proxies
#include <test/ext_int128.hpp>                      // TEST_HAS_ABSL_INT128, TEST_HAS_BOOST_INT128, uint128
#include <test/for_each_type.hpp>                   // for_each_type
#include <test/minimal_blocks.hpp>                  // minimal_blocks
#include <test/value_reference.hpp>                 // value_reference
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set
#include <xstd/bits/bit_key_traits.hpp>             // bit_key_traits
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/ownership.hpp>           // storage
#include <xstd/bits/detail/set_adaptor.hpp>         // set_adaptor, set_reference
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                    // array
#include <compare>                                  // strong_ordering
#include <concepts>                                 // bidirectional_iterator, equality_comparable, equality_comparable_with, same_as, totally_ordered, totally_ordered_with
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // greater
#include <format>                                   // formattable
#include <iterator>                                 // iter_reference_t, iter_value_t, next, prev
#include <optional>                                 // optional
#include <ranges>                                   // iota
#include <set>                                      // set
#include <type_traits>                              // is_assignable_v, is_constructible_v, is_convertible_v, is_trivially_destructible_v
#include <utility>                                  // cmp_equal, declval

namespace {

// A key the proxy hands out through its traits, and a strong type that takes the size_t only explicitly.
struct key
{
        std::size_t value;

        [[nodiscard]] friend auto operator<=>(key const&, key const&) -> std::strong_ordering = default;
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

// A user's namespace holding the proxy's key traits, and a comparison of its own beside them.
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

// A class key whose comparisons are hidden friends, which only ADL through the key itself finds.
struct slot
{
        std::size_t value;

        [[nodiscard]] friend constexpr auto operator==(slot lhs, slot rhs) noexcept
                -> bool
        {
                return lhs.value == rhs.value;
        }

        [[nodiscard]] friend constexpr auto operator==(slot lhs, int rhs) noexcept
                -> bool
        {
                return std::cmp_equal(lhs.value, rhs);
        }

        [[nodiscard]] friend auto operator<=>(slot, slot) -> std::strong_ordering = default;
};

} // namespace user

// One full block of each Block, which is all a comparison between two of its proxies asks for.
template<class Block>
using block_bits = test::array_storage<Block, test::digits_v<Block>>;

// The set view over a storage, whose iterator and proxy are the ones under test.
template<class Bits, class Key = std::size_t, class KeyTraits = xstd::bit_key_traits<Key>>
using borrowed_set = xstd::bits::detail::set_adaptor<Bits, xstd::bits::detail::storage::borrowed, void, Key, KeyTraits>;

template<class Block>
using block_reference = borrowed_set<block_bits<Block>>::reference;

} // namespace

// A namespace whose comparisons take anything exactly, so they decide every comparison ADL brings them into.
namespace acme {

template<class A, class B>
[[nodiscard]] constexpr auto operator==(A const& /* lhs */, B const& /* rhs */) noexcept
        -> bool
{
        return false;
}

template<class A, class B>
[[nodiscard]] constexpr auto operator<(A const& /* lhs */, B const& /* rhs */) noexcept
        -> bool
{
        return false;
}

// Key traits of acme's own, a template argument of every set adaptor over them.
struct key_traits : xstd::bit_key_traits<std::size_t>
{};

// Key traits of acme's own for a key of the user's.
struct slot_traits
{
        [[nodiscard]] static constexpr auto to_index(user::slot key) noexcept
                -> std::size_t
        {
                return key.value;
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> user::slot
        {
                return {.value = index};
        }
};

// Storage of acme's own, a template argument of every adaptor over it.
template<class Block>
class blocks : public test::minimal_blocks<Block>
{};

} // namespace acme

namespace {

using hostile_set = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 64, acme::key_traits>;

// The proxies under acme's key traits compare as their keys do, and the iterators as their positions do.
[[nodiscard]] constexpr auto hostile_key_traits_compare_as_ours()
        -> bool
{
        auto s = hostile_set();
        s.insert(3UZ);
        s.insert(5UZ);
        auto const first  = s.begin();
        auto const again  = s.begin();
        auto const second = std::next(first);
        auto const low    = *first;
        auto const same   = *again;
        auto const high   = *second;
        return low == same and not(low != same) and not(low < same) and low != high and low < high and first == again and first != second;
}

using hostile_slot_set = xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<acme::blocks<std::uint64_t>>, xstd::bits::detail::storage::owned, void, user::slot, acme::slot_traits>;

// A user's key under acme's storage and key traits: the key's own comparisons decide, and acme's are not found.
[[nodiscard]] constexpr auto hostile_slot_set_compares_as_its_keys()
        -> bool
{
        auto s = hostile_slot_set();
        s.insert(user::slot{.value = 3});
        s.insert(user::slot{.value = 5});
        auto const first  = s.begin();
        auto const second = std::next(first);
        return *first == *s.begin() and *first != *second and *first == 3 and *second == 5 and not(*first == 5) and *first < *second;
}

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

        static_assert(sizeof(borrowed_set<Bits>::iterator) == two_pointers);
        static_assert(sizeof(borrowed_set<Bits>::reference) == two_pointers);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetIteratorIsBidirectional, T, ArrayTypes)
{
        static_assert(std::bidirectional_iterator<typename borrowed_set<T>::iterator>);
}

// The set proxy never writes, so nothing distinguishes its const spelling.
BOOST_AUTO_TEST_CASE(TheSetProxyNeverWrites)
{
        static_assert(not std::is_assignable_v<borrowed_set<Bits>::reference const&, std::size_t>);
        static_assert(std::is_convertible_v<borrowed_set<Bits>::reference, std::size_t>);
        static_assert(std::is_convertible_v<borrowed_set<Bits const>::reference, std::size_t>);

        BOOST_CHECK(true);
}

// What a container's const_reference must be: trivially copyable, never assignable, comparable by value.
BOOST_AUTO_TEST_CASE(TheReadOnlyProxiesAreValues)
{
        static_assert(test::value_reference<borrowed_set<Bits>::reference>);
        static_assert(test::value_reference<borrowed_set<Bits const>::reference>);
        static_assert(std::is_trivially_destructible_v<borrowed_set<Bits>::iterator>);

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
        using set_type = xstd::basic_bit_fixed_set<key, std::uint64_t, 200, key_traits>;
        using iterator = set_type::iterator;
        static_assert(std::same_as<std::iter_value_t<iterator>, key>);
        static_assert(std::bidirectional_iterator<iterator>);
        static_assert(not std::is_convertible_v<std::iter_reference_t<iterator>, std::size_t>);

        auto s = set_type();
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

                // A user's non-template operator== beside the key traits is no second reading of ours.
                using user_reference = borrowed_set<block_bits<Block>, std::size_t, user::key_traits>::reference;
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

// acme's operators decide a comparison between its own key traits, and none between proxies or iterators using them.
BOOST_AUTO_TEST_CASE(AKeyTraitsNamespaceIsNotAssociatedWithItsProxies)
{
        auto const lhs = acme::key_traits();
        auto const rhs = acme::key_traits();
        BOOST_CHECK(not(lhs == rhs));
        BOOST_CHECK(not(lhs < rhs));

        static_assert(hostile_key_traits_compare_as_ours());
        BOOST_CHECK(hostile_key_traits_compare_as_ours());
}

// The key's own namespace is the one a proxy keeps, so comparisons only ADL finds for the key reach the proxy too.
BOOST_AUTO_TEST_CASE(TheKeysNamespaceIsAssociatedWithItsProxies)
{
        using reference = borrowed_set<Bits, key, key_traits>::reference;
        static_assert(std::totally_ordered<reference>);
        static_assert(std::totally_ordered_with<reference, key>);

        auto s = xstd::basic_bit_fixed_set<key, std::uint64_t, 200, key_traits>();
        s.insert(key{3});
        s.insert(key{5});
        BOOST_CHECK(*s.begin() == *s.begin());
        BOOST_CHECK(*s.begin() < *std::next(s.begin()));
        BOOST_CHECK(*s.begin() == key{3});
}

// The key's hidden friends reach the proxy, against its own and its mirror's type, whoever owns storage and traits.
BOOST_AUTO_TEST_CASE(AKeysHiddenFriendsDecideBesideAHostileStorageAndKeyTraits)
{
        using reference = std::iter_reference_t<hostile_slot_set::iterator>;
        static_assert(std::equality_comparable<reference>);
        static_assert(std::equality_comparable_with<reference, user::slot>);

        static_assert(hostile_slot_set_compares_as_its_keys());
        BOOST_CHECK(hostile_slot_set_compares_as_its_keys());
}

// Every set and view hands out a pair that * and & close, its const_iterator being its iterator.
BOOST_AUTO_TEST_CASE(EverySetClosesItsProxyPair)
{
        static_assert(test::closed_proxies<xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 65>>);
        static_assert(test::closed_proxies<xstd::basic_bit_set<std::size_t, std::uint64_t>>);
        static_assert(test::closed_proxies<xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 65>>);
        static_assert(test::closed_proxies<xstd::basic_bit_small_set<std::size_t, std::uint64_t, 65>>);
        static_assert(test::closed_proxies<xstd::basic_bit_fixed_set<key, std::uint64_t, 65, key_traits>>);
        static_assert(test::closed_proxies<xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 65, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>>); // NOLINT(modernize-use-transparent-functors): the descending direction, as a set's comparator names it
        static_assert(test::closed_proxies<xstd::bit_set_view<std::array<std::uint64_t, 2>>>);
        static_assert(test::closed_proxies<xstd::bit_set_view<std::array<std::uint64_t, 2> const>>);
        static_assert(test::closed_proxies<hostile_set>);
        static_assert(test::closed_proxies<hostile_slot_set>);

        BOOST_CHECK(true);
}

// What std::formatter is specialized for: the proxies themselves, which no deduction reaches through the class.
BOOST_AUTO_TEST_CASE(TheFormatterIsSpecializedForExactlyTheProxies)
{
        static_assert(xstd::bits::detail::set_reference<borrowed_set<Bits>::reference>);
        static_assert(xstd::bits::detail::set_reference<borrowed_set<Bits, key, key_traits>::reference>);
        static_assert(not xstd::bits::detail::set_reference<borrowed_set<Bits>::iterator>);
        static_assert(not xstd::bits::detail::set_reference<std::size_t>);

        static_assert(std::formattable<borrowed_set<Bits>::reference, char>);
        static_assert(std::formattable<std::iter_reference_t<hostile_set::iterator>, char>);
        static_assert(not std::formattable<borrowed_set<Bits, key, key_traits>::reference, char>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

// The set view hands out a pair of its own, shaped as every other.
BOOST_AUTO_TEST_SUITE(BidirectionalThroughTheView)

namespace {

using Viewed = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 1>, 64>;

using SetView = xstd::bit_set_view<std::array<std::uint64_t, 1>>;

using SetIt  = SetView::iterator;
using SetRef = SetView::reference;

// Dependent, so a type without the member is a substitution failure rather than a hard error.
template<class R>
constexpr bool has_address_of = requires (R r) { r.operator&(); };

} // namespace

// Each container has its own pair, as std::string_view's iterator is not std::string's, though both read one storage.
BOOST_AUTO_TEST_CASE(TheViewIteratesWithItsOwnProxy)
{
        static_assert(not std::same_as<SetIt, borrowed_set<Viewed>::iterator>);
        static_assert(not std::same_as<SetRef, borrowed_set<Viewed>::reference>);
        static_assert(not std::same_as<SetIt, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 64>::iterator>);

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
