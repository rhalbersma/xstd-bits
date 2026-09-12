//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                      // graded_extents
#include <test/ext_int128.hpp>                       // TEST_HAS_ABSL_INT128, TEST_HAS_BOOST_INT128, uint128
#include <test/minimal_traits.hpp>                   // minimal_traits
#include <test/value_reference.hpp>                  // value_reference
#include <xstd/bits/bit_array.hpp>                   // basic_bit_array
#include <xstd/bits/bit_span.hpp>                    // bit_span
#include <xstd/bits/bit_traits.hpp>                  // bit_traits
#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/detail/random_access.hpp>        // random_access_bit_iterator, random_access_bit_reference
#include <xstd/bits/ext/boost/dynamic_bitset.hpp>    // bit_traits over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>              // bit_traits over std::bitset
#include <boost/dynamic_bitset.hpp>                  // dynamic_bitset
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                 // equal, ranges::reverse, ranges::sort, reverse, sort
#include <bitset>                                    // bitset
#include <concepts>                                  // convertible_to, equality_comparable, random_access_iterator, same_as, sortable
#include <cstddef>                                   // ptrdiff_t, size_t
#include <cstdint>                                   // uint64_t
#include <iterator>                                  // iter_move, next, prev, reverse_iterator
#include <ranges>                                    // subrange
#include <type_traits>                               // is_assignable_v, is_convertible_v, is_trivially_copy_constructible_v, is_trivially_destructible_v
#include <utility>                                   // declval
#include <vector>                                    // vector

namespace {

// A strong type to receive what the proxy converts to, copy-initialized and never cast: a cast is a
// direct-initialization with two routes in, and MSVC calls that no route at all.
struct flag
{
        bool value;
        constexpr explicit(false) flag(bool v) noexcept : value(v) {}  // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
};

// One position: written, read back two ways, negated back through itself, and reached again through the subscript.
template<class Iterator>
auto check_position(Iterator first, std::size_t i, std::vector<bool>& model)
        -> void
{
        auto const it = first + static_cast<std::ptrdiff_t>(i);
        BOOST_CHECK(&*it == it);

        *it = (i % 3 == 0);
        model[i] = (i % 3 == 0);
        BOOST_CHECK_EQUAL(static_cast<bool>(*it), model[i]);
        flag const f = *it;
        BOOST_CHECK_EQUAL(f.value, model[i]);

        *it = not *it;
        model[i] = not model[i];
        BOOST_CHECK_EQUAL(static_cast<bool>(first[static_cast<std::ptrdiff_t>(i)]), model[i]);
}

template<class T>
[[nodiscard]] auto as_vector(T const& c)
        -> std::vector<bool>
{
        using Traits = xstd::bit_traits<T>;
        auto v = std::vector<bool>(Traits::size(c));
        for (auto i = 0UZ; i < v.size(); ++i) {
                v[i] = Traits::at(c, i);
        }
        return v;
}

}       // namespace

BOOST_AUTO_TEST_SUITE(RandomAccess)

using ArrayTypes = test::graded_extents<xstd::detail::bits::contiguous_bit_array>;

using Bits = xstd::detail::bits::contiguous_bit_array<std::uint64_t, 200>;

BOOST_AUTO_TEST_CASE(AnIteratorIsAPointerAndAPosition)
{
        constexpr auto two_words = 2UZ * sizeof(void*);

        static_assert(sizeof(xstd::detail::bits::random_access_bit_iterator<Bits>)        == two_words);
        static_assert(sizeof(xstd::detail::bits::random_access_bit_reference<Bits>)       == two_words);
        static_assert(sizeof(xstd::detail::bits::random_access_bit_iterator<Bits const>)  == two_words);
        static_assert(sizeof(xstd::detail::bits::random_access_bit_reference<Bits const>) == two_words);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSequenceIteratorIsRandomAccess, T, ArrayTypes)
{
        static_assert(std::random_access_iterator<xstd::detail::bits::random_access_bit_iterator<T>>);
        static_assert(std::random_access_iterator<xstd::detail::bits::random_access_bit_iterator<T const>>);
        static_assert(std::random_access_iterator<xstd::detail::bits::random_access_bit_iterator<T, test::minimal_traits<T>>>);

        static_assert(    std::sortable<xstd::detail::bits::random_access_bit_iterator<T>>);
        static_assert(not std::sortable<xstd::detail::bits::random_access_bit_iterator<T const>>);
}

// The same over the two foreign types, which is what bit_traits is for.
BOOST_AUTO_TEST_CASE(TheForeignTypesIterateThroughTheirTraits)
{
        static_assert(std::random_access_iterator<xstd::detail::bits::random_access_bit_iterator<std::bitset<9>>>);
        static_assert(std::random_access_iterator<xstd::detail::bits::random_access_bit_iterator<boost::dynamic_bitset<>>>);

        static_assert(std::sortable<xstd::detail::bits::random_access_bit_iterator<std::bitset<9>>>);
        static_assert(std::sortable<xstd::detail::bits::random_access_bit_iterator<boost::dynamic_bitset<>>>);

        BOOST_CHECK(true);
}

// Const is in the Bits, not in a flag, and a trait with only the required entries has no way to write either.
BOOST_AUTO_TEST_CASE(ConstnessLivesInTheBitsAndWritabilityInTheTraits)
{
        using Ref        = xstd::detail::bits::random_access_bit_reference<Bits>;
        using ConstRef   = xstd::detail::bits::random_access_bit_reference<Bits const>;
        using MinimalRef = xstd::detail::bits::random_access_bit_reference<Bits, test::minimal_traits<Bits>>;

        static_assert(    std::is_assignable_v<Ref const&, bool>);
        static_assert(not std::is_assignable_v<ConstRef const&, bool>);
        static_assert(not std::is_assignable_v<MinimalRef const&, bool>);

        static_assert(std::is_convertible_v<Ref, bool>);
        static_assert(std::is_convertible_v<ConstRef, bool>);
        static_assert(std::is_convertible_v<MinimalRef, bool>);

        BOOST_CHECK(true);
}

// What a container's const_reference must be: trivially copyable, never assignable, comparable by value.
BOOST_AUTO_TEST_CASE(TheReadOnlyProxiesAreValues)
{
        static_assert(test::value_reference<xstd::detail::bits::random_access_bit_reference<Bits const>>);
        static_assert(test::value_reference<xstd::detail::bits::random_access_bit_reference<Bits, test::minimal_traits<Bits>>>);

        // The writable proxy is the one exception, by design: its assignment writes the bit. Trivial to copy and destroy all the same.
        static_assert(not test::value_reference<xstd::detail::bits::random_access_bit_reference<Bits>>);
        static_assert(std::is_trivially_copy_constructible_v<xstd::detail::bits::random_access_bit_reference<Bits>>);
        static_assert(std::is_trivially_destructible_v<xstd::detail::bits::random_access_bit_reference<Bits>>);
        static_assert(std::is_trivially_destructible_v<xstd::detail::bits::random_access_bit_iterator<Bits>>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AMutableSequenceIteratorConvertsToItsConstTwin)
{
        using It      = xstd::detail::bits::random_access_bit_iterator<Bits>;
        using ConstIt = xstd::detail::bits::random_access_bit_iterator<Bits const>;

        static_assert(    std::convertible_to<It, ConstIt>);
        static_assert(not std::convertible_to<ConstIt, It>);

        auto b = Bits();
        auto const it = It(&b, 7UZ);
        ConstIt const cit = it;
        BOOST_CHECK(cit == ConstIt(&b, 7UZ));
        BOOST_CHECK(*cit == false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSequenceIteratorReadsAndWritesThroughTheTraits, T, ArrayTypes)
{
        constexpr auto N = xstd::bit_traits<T>::extent;

        auto c = T();
        // Written through check_position below, which the check cannot see past a dependent call. [design.md#clang-tidy-false-positives]
        auto model = std::vector<bool>(N);  // NOLINT(misc-const-correctness)
        auto const first = xstd::detail::bits::random_access_bit_iterator<T>(&c, 0UZ);
        BOOST_CHECK(first == xstd::detail::bits::random_access_bit_iterator<T const>(&c, 0UZ));

        // Nothing to step over at a zero width, so nothing is instantiated for it. [design.md#per-instantiation-slots]
        if constexpr (N != 0UZ) {
                for (auto i = 0UZ; i < N; ++i) {
                        check_position(first, i, model);
                }
                BOOST_CHECK(as_vector(c) == model);
        }
}

// Proxy-to-proxy assignment copies the bit and never rebinds, which is what makes the swaps work.
BOOST_AUTO_TEST_CASE(ProxyAssignmentCopiesTheBitAndSwapSwapsTheBits)
{
        auto c = Bits();
        auto const first = xstd::detail::bits::random_access_bit_iterator<Bits>(&c, 0UZ);
        auto const second = std::next(first);

        *first = true;
        *second = false;
        *second = *first;
        BOOST_CHECK(*second == true);

        *second = false;
        swap(*first, *second);
        BOOST_CHECK(*first == false and *second == true);

        bool b = false;
        swap(*second, b);
        BOOST_CHECK(b and *second == false);
        swap(b, *first);
        BOOST_CHECK(not b and *first == true);
}

BOOST_AUTO_TEST_CASE(TheSequenceIteratorArithmeticIsIndexArithmetic)
{
        auto c = Bits();
        auto const first = xstd::detail::bits::random_access_bit_iterator<Bits>(&c, 0UZ);
        auto const last  = xstd::detail::bits::random_access_bit_iterator<Bits>(&c, 200UZ);
        BOOST_CHECK_EQUAL(last - first, 200);
        BOOST_CHECK(first <= last);
        BOOST_CHECK(first <= first and last >= last);

        auto it = first;
        it += 2;
        it -= 1;
        BOOST_CHECK(it == std::next(first));
        BOOST_CHECK(it > first and first < it);
        BOOST_CHECK(it - 1 == first);
        BOOST_CHECK(1 + first == it);

        auto const was = it++;
        BOOST_CHECK(was == std::next(first) and it == first + 2);
        auto const back = it--;
        BOOST_CHECK(back == first + 2 and it == std::next(first));
        --it;
        BOOST_CHECK(it == first);
}

BOOST_AUTO_TEST_CASE(RangesAlgorithmsReachTheBitsThroughIterMoveAndIterSwap)
{
        using iterator = xstd::detail::bits::random_access_bit_iterator<Bits>;

        auto c = Bits();
        auto model = std::vector<bool>(200);
        for (auto const p : { 0UZ, 5UZ, 63UZ, 64UZ, 130UZ, 199UZ }) {
                c.set(p);
                model[p] = true;
        }
        auto const first = iterator(&c, 0UZ);
        auto const last  = iterator(&c, 200UZ);
        BOOST_CHECK(std::ranges::equal(std::ranges::subrange(first, last), model));

        static_assert(std::same_as<decltype(std::ranges::iter_move(first)), bool>);
        BOOST_CHECK_EQUAL(std::ranges::iter_move(first), true);

        std::ranges::sort(first, last);
        std::ranges::sort(model);
        BOOST_CHECK(as_vector(c) == model);

        std::ranges::reverse(first, last);
        std::ranges::reverse(model);
        BOOST_CHECK(as_vector(c) == model);

        // The pre-ranges algorithms on purpose: they reach the bits through std::iter_swap and the swap friends, not iter_swap.
        std::sort(first, last);  // NOLINT(modernize-use-ranges)
        std::ranges::sort(model);
        BOOST_CHECK(as_vector(c) == model);

        std::reverse(first, last);  // NOLINT(modernize-use-ranges)
        std::ranges::reverse(model);
        BOOST_CHECK(as_vector(c) == model);

        std::ranges::iter_swap(first, std::prev(last));
        bool const t = model.front();
        model.front() = model.back();
        model.back() = t;
        BOOST_CHECK(as_vector(c) == model);

        // and the const twin is reachable by the reverse adaptor, being a proper bidirectional iterator.
        auto const rfirst = std::reverse_iterator(xstd::detail::bits::random_access_bit_iterator<Bits const>(last));
        BOOST_CHECK_EQUAL(static_cast<bool>(*rfirst), model.back());
}

// The foreign sequence proxies write through their own unchecked subscript.
BOOST_AUTO_TEST_CASE(TheForeignSequenceProxiesWriteThroughTheTraits)
{
        auto s = std::bitset<9>();
        auto const sit = xstd::detail::bits::random_access_bit_iterator<std::bitset<9>>(&s, 4UZ);
        *sit = true;
        BOOST_CHECK(s.test(4));
        *sit = false;
        BOOST_CHECK(not s.test(4));

        auto d = boost::dynamic_bitset<>(9);
        auto const dit = xstd::detail::bits::random_access_bit_iterator<boost::dynamic_bitset<>>(&d, 8UZ);
        *dit = true;
        BOOST_CHECK(d.test(8));
        BOOST_CHECK(dit - 8 < dit and dit > dit - 8 and dit <= dit);
        *(dit - 1) = *dit;
        BOOST_CHECK(d.test(7));
        swap(*(dit - 2), *(dit - 1));
        BOOST_CHECK(d.test(6) and not d.test(7));
        std::ranges::sort(dit - 8, dit + 1);
        BOOST_CHECK(d.test(8) and d.test(7) and d.count() == 2UZ);
}

// format_as is what fmt calls, unqualified, so calling it the same way is the test.
BOOST_AUTO_TEST_CASE(TheProxyFormatsAsItsValue)
{
        auto c = Bits();
        c.set(42);

        BOOST_CHECK_EQUAL(format_as(*xstd::detail::bits::random_access_bit_iterator<Bits>(&c, 42UZ)), true);
        BOOST_CHECK_EQUAL(format_as(*xstd::detail::bits::random_access_bit_iterator<Bits const>(&c, 41UZ)), false);
}

BOOST_AUTO_TEST_SUITE_END()

// The sequence view hands out this proxy and nothing of its own; what ranges.hpp once answered, now answered here. [design.md#the-iterator-is-the-primitive]
BOOST_AUTO_TEST_SUITE(RandomAccessThroughTheView)

namespace {

using Viewed = std::bitset<64>;

using ArrIt  = xstd::detail::bits::random_access_bit_iterator<Viewed>;
using ArrRef = xstd::detail::bits::random_access_bit_reference<Viewed>;

// Dependent, so a type without the member is a substitution failure rather than a hard error.
template<class R>
constexpr bool has_address_of = requires(R r) { r.operator&(); };

}       // namespace

BOOST_AUTO_TEST_CASE(TheViewIteratesWithTheSharedProxy)
{
        static_assert(std::same_as<xstd::bit_span<Viewed>::iterator,  ArrIt>);
        static_assert(std::same_as<xstd::bit_span<Viewed>::reference, ArrRef>);

        BOOST_CHECK(true);
}

// One shape asked twice: * gives a proxy, & gives an iterator back, and the value comes only by converting; the standard says nothing here, so only our own guarantees are asserted.
BOOST_AUTO_TEST_CASE(DereferencingYieldsAProxyRatherThanTheValue)
{
        static_assert(std::same_as<decltype(*std::declval<ArrIt const&>()), ArrRef>);
        static_assert(not std::same_as<decltype(*std::declval<ArrIt const&>()), bool>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AddressOfAProxyYieldsAnIterator)
{
        static_assert(std::same_as<decltype(&std::declval<ArrRef const&>()), ArrIt>);
        static_assert(has_address_of<ArrRef>);

        BOOST_CHECK(true);
}

// The const path is a proxy too, the same one minus the assignment -- not the plain bool libstdc++ hands back.
BOOST_AUTO_TEST_CASE(TheConstPathIsAProxyAsWell)
{
        using ConstArrRef = xstd::detail::bits::random_access_bit_reference<Viewed const>;

        static_assert(std::same_as<xstd::bit_span<Viewed const>::reference, ConstArrRef>);
        static_assert(std::is_convertible_v<ConstArrRef, bool>);
        static_assert(has_address_of<ConstArrRef>);
        static_assert(std::same_as<decltype(&std::declval<ConstArrRef const&>()), xstd::detail::bits::random_access_bit_iterator<Viewed const>>);

        // and it is exactly the assignment that the const one drops.
        static_assert(    std::is_assignable_v<ArrRef const&, bool>);
        static_assert(not std::is_assignable_v<ConstArrRef const&, bool>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheValueArrivesByImplicitConversion)
{
        static_assert(std::is_convertible_v<ArrRef, bool>);

        auto b = Viewed();
        b.set(3);
        b.set(5);

        auto const a = xstd::bit_span(b);
        bool const bit = a[3];
        BOOST_CHECK(bit);
        BOOST_CHECK(a[5] == true);
        BOOST_CHECK(a[4] == false);
}

// ... but not to an integer, however class-shaped it is. The conversion above takes any class constructible
// from bool, and a 128-bit integer class is one -- which would give every operator on a proxy two equally good
// readings, convert both sides to bool or convert both sides to the Block, and cost the proxy the
// equality_comparable that ranges::equal needs. A bit is not an integer, and this pins that both ways: the
// conversion is gone and the comparison still works. Nothing here is reachable with a builtin Block, which is
// the whole reason the integer classes are worth a Block. [design.md#uint128-support]
#if defined(TEST_HAS_MSVC_INT128) || defined(TEST_HAS_ABSL_INT128) || defined(TEST_HAS_BOOST_INT128)
BOOST_AUTO_TEST_CASE(AProxyNeverBecomesAnIntegerBlock)
{
#ifdef TEST_HAS_MSVC_INT128
        {
                using B = xstd::basic_bit_array<xstd::uint128, 257>;
                static_assert(    std::convertible_to<B::reference, bool>);
                static_assert(not std::convertible_to<B::reference, xstd::uint128>);
                static_assert(std::equality_comparable<B::reference>);
                static_assert(std::equality_comparable<B::const_reference>);

                auto a = B();
                a[0] = true;
                a[256] = true;
                BOOST_CHECK(a[0] == a[256]);
                BOOST_CHECK(a[0] != a[1]);
                BOOST_CHECK(a[0] == true);
        }
#endif
#ifdef TEST_HAS_ABSL_INT128
        {
                using B = xstd::basic_bit_array<absl::uint128, 257>;
                static_assert(    std::convertible_to<B::reference, bool>);
                static_assert(not std::convertible_to<B::reference, absl::uint128>);
                static_assert(std::equality_comparable<B::reference>);
                static_assert(std::equality_comparable<B::const_reference>);

                auto a = B();
                a[0] = true;
                a[256] = true;
                BOOST_CHECK(a[0] == a[256]);
                BOOST_CHECK(a[0] != a[1]);
                BOOST_CHECK(a[0] == true);
        }
#endif
#ifdef TEST_HAS_BOOST_INT128
        {
                using B = xstd::basic_bit_array<boost::int128::uint128, 257>;
                static_assert(    std::convertible_to<B::reference, bool>);
                static_assert(not std::convertible_to<B::reference, boost::int128::uint128>);
                static_assert(std::equality_comparable<B::reference>);
                static_assert(std::equality_comparable<B::const_reference>);

                auto a = B();
                a[0] = true;
                a[256] = true;
                BOOST_CHECK(a[0] == a[256]);
                BOOST_CHECK(a[0] != a[1]);
                BOOST_CHECK(a[0] == true);
        }
#endif
}
#endif

// & . * and * . & are both the identity, which makes the pair a round trip rather than two one-way conversions.
BOOST_AUTO_TEST_CASE(TheProxyPairRoundTrips)
{
        auto b = Viewed();
        b.set(3);
        b.set(5);
        b.set(7);
        b.set(11);

        auto const a = xstd::bit_span(b);
        for (auto it = a.begin(); it != a.end(); ++it) {
                BOOST_CHECK(&*it == it);
                BOOST_CHECK(static_cast<bool>(*&*it) == static_cast<bool>(*it));
        }

        // and writing goes through the reference the round trip hands back.
        *&a.begin()[4] = true;
        BOOST_CHECK(b.test(4));
}

BOOST_AUTO_TEST_SUITE_END()
