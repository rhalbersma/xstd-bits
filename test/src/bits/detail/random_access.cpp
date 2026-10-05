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
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_span.hpp>                   // bit_span
#include <xstd/bits/bit_subspan.hpp>                // bit_subspan
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/ownership.hpp>           // storage
#include <xstd/bits/detail/random_access.hpp>       // random_access_bit_iterator, random_access_bit_reference, random_access_reference
#include <xstd/bits/detail/sequence_adaptor.hpp>    // sequence_adaptor
#include <xstd/bits/detail/storage_ptr.hpp>         // storage_ptr_t
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                // equal, ranges::reverse, ranges::sort, reverse, sort
#include <array>                                    // array
#include <concepts>                                 // convertible_to, equality_comparable, random_access_iterator, same_as, sortable, totally_ordered, totally_ordered_with
#include <cstddef>                                  // ptrdiff_t, size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <format>                                   // formattable
#include <iterator>                                 // iter_move, next, prev, reverse_iterator
#include <optional>                                 // optional
#include <ranges>                                   // iota, subrange
#include <type_traits>                              // is_assignable_v, is_constructible_v, is_convertible_v, is_trivially_copy_constructible_v, is_trivially_destructible_v
#include <utility>                                  // as_const, declval
#include <vector>                                   // vector

namespace {

// A class implicitly constructible from bool, which a proxy direct-initializes and never copy-initializes.
struct flag
{
        bool value;

        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr explicit(false) flag(bool v) noexcept
                : value(v)
        {}
};

// One position: written, read back two ways, negated back through itself, and reached again through the subscript.
template<class Iterator>
auto check_position(Iterator first, std::size_t i, std::vector<bool>& model)
        -> void
{
        auto const it = first + static_cast<std::ptrdiff_t>(i);
        BOOST_CHECK(&*it == it);

        *it      = (i % 3 == 0);
        model[i] = (i % 3 == 0);
        BOOST_CHECK_EQUAL(static_cast<bool>(*it), model[i]);
        flag const f(*it);
        BOOST_CHECK_EQUAL(f.value, model[i]);

        *it      = not *it;
        model[i] = not model[i];
        BOOST_CHECK_EQUAL(static_cast<bool>(first[static_cast<std::ptrdiff_t>(i)]), model[i]);
}

// An iterator into the storage at a position, as a view borrowing it hands one out: only a view may build one.
template<class Bits>
[[nodiscard]] constexpr auto iterator_at(Bits& c, std::size_t n)
{
        return xstd::bits::detail::sequence_adaptor<Bits, xstd::bits::detail::storage::borrowed>(c).begin() + static_cast<std::ptrdiff_t>(n);
}

template<class T>
[[nodiscard]] auto as_vector(T const& c)
        -> std::vector<bool>
{
        auto v = std::vector<bool>(c.size());
        for (auto const i : std::views::iota(0UZ, v.size())) {
                v[i] = c.test(i);
        }
        return v;
}

// One full block of each Block, which is all a comparison between two of its proxies asks for.
template<class Block>
using block_bits = test::array_storage<Block, test::digits_v<Block>>;

template<class Block>
using block_reference = xstd::bits::detail::random_access_bit_reference<block_bits<Block>>;

// Dependent, so an ambiguous comparison is a substitution failure rather than a hard error.
template<class L, class R>
concept equal_to_with = requires (L const& lhs, R const& rhs) {
        lhs == rhs;
        rhs == lhs;
};

template<class L, class R>
concept less_than_with = requires (L const& lhs, R const& rhs) {
        lhs < rhs;
        rhs < lhs;
};

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

// Storage of acme's own, which makes it a template argument of every proxy handed out over it.
template<class Block>
class blocks : public test::minimal_blocks<Block>
{};

} // namespace acme

namespace {

using hostile_bits = xstd::bits::detail::bit_block_container<acme::blocks<std::uint64_t>>;

// The proxies over acme's storage compare as their bools do, and the iterators as their positions do.
[[nodiscard]] constexpr auto hostile_storage_compares_as_ours()
        -> bool
{
        auto c = hostile_bits(64UZ);
        c.set(1);
        auto const first      = iterator_at(c, 0UZ);
        auto const again      = iterator_at(c, 0UZ);
        auto const second     = std::next(first);
        auto const clear      = *first;
        auto const same       = *again;
        auto const set        = *second;
        auto const equalities = clear == same and not(clear != same) and clear != set;
        // The built-in < is the comparison under test, and it compares the two bools as ints.
        auto const orderings = not(clear < same) and clear < set; // NOLINT(readability-implicit-bool-conversion)
        return equalities and orderings and first == again and first != second and first < second;
}

} // namespace

BOOST_AUTO_TEST_SUITE(RandomAccess)

using ArrayTypes = test::graded_extents<test::array_storage>;

using Bits = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 4>, 200>;

BOOST_AUTO_TEST_CASE(AnIteratorIsAPointerAndAPosition)
{
        constexpr auto two_pointers = 2UZ * sizeof(void*);

        static_assert(sizeof(xstd::bits::detail::random_access_bit_iterator<Bits>) == two_pointers);
        static_assert(sizeof(xstd::bits::detail::random_access_bit_reference<Bits>) == two_pointers);
        static_assert(sizeof(xstd::bits::detail::random_access_bit_iterator<Bits const>) == two_pointers);
        static_assert(sizeof(xstd::bits::detail::random_access_bit_reference<Bits const>) == two_pointers);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSequenceIteratorIsRandomAccess, T, ArrayTypes)
{
        static_assert(std::random_access_iterator<xstd::bits::detail::random_access_bit_iterator<T>>);
        static_assert(std::random_access_iterator<xstd::bits::detail::random_access_bit_iterator<T const>>);

        static_assert(std::sortable<xstd::bits::detail::random_access_bit_iterator<T>>);
        static_assert(not std::sortable<xstd::bits::detail::random_access_bit_iterator<T const>>);
}

// Const is in the Bits and nowhere else: the proxy asks the storage, and a const storage has no assign to reach.
BOOST_AUTO_TEST_CASE(ConstnessLivesInTheBits)
{
        using Ref      = xstd::bits::detail::random_access_bit_reference<Bits>;
        using ConstRef = xstd::bits::detail::random_access_bit_reference<Bits const>;

        static_assert(std::is_assignable_v<Ref const&, bool>);
        static_assert(not std::is_assignable_v<ConstRef const&, bool>);

        static_assert(std::is_convertible_v<Ref, bool>);
        static_assert(std::is_convertible_v<ConstRef, bool>);

        BOOST_CHECK(true);
}

// What a container's const_reference must be: trivially copyable, never assignable, comparable by value.
BOOST_AUTO_TEST_CASE(TheReadOnlyProxiesAreValues)
{
        static_assert(test::value_reference<xstd::bits::detail::random_access_bit_reference<Bits const>>);

        // The writable proxy is the one exception by design, and trivial to copy and destroy all the same.
        static_assert(not test::value_reference<xstd::bits::detail::random_access_bit_reference<Bits>>);
        static_assert(std::is_trivially_copy_constructible_v<xstd::bits::detail::random_access_bit_reference<Bits>>);
        static_assert(std::is_trivially_destructible_v<xstd::bits::detail::random_access_bit_reference<Bits>>);
        static_assert(std::is_trivially_destructible_v<xstd::bits::detail::random_access_bit_iterator<Bits>>);

        BOOST_CHECK(true);
}

// A pointer and a position make a proxy only inside the library: a user reaches one through a sequence.
BOOST_AUTO_TEST_CASE(OnlyTheLibraryBuildsAProxyFromAPointerAndAPosition)
{
        using Ptr = xstd::bits::detail::storage_ptr_t<Bits>;
        static_assert(not std::is_constructible_v<xstd::bits::detail::random_access_bit_iterator<Bits>, Ptr, std::size_t>);
        static_assert(not std::is_constructible_v<xstd::bits::detail::random_access_bit_reference<Bits>, Ptr, std::size_t>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AMutableSequenceIteratorConvertsToItsConstTwin)
{
        using It      = xstd::bits::detail::random_access_bit_iterator<Bits>;
        using ConstIt = xstd::bits::detail::random_access_bit_iterator<Bits const>;

        static_assert(std::convertible_to<It, ConstIt>);
        static_assert(not std::convertible_to<ConstIt, It>);

        auto b            = Bits();
        auto const it     = iterator_at(b, 7UZ);
        ConstIt const cit = it;
        BOOST_CHECK(cit == iterator_at(std::as_const(b), 7UZ));
        BOOST_CHECK(*cit == false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSequenceIteratorReadsAndWritesThroughTheStorage, T, ArrayTypes)
{
        constexpr auto N = T::extent;

        auto c = T();
        // Written through check_position below, which the check cannot see past a dependent call.
        auto model       = std::vector<bool>(N); // NOLINT(misc-const-correctness)
        auto const first = iterator_at(c, 0UZ);
        BOOST_CHECK(first == iterator_at(std::as_const(c), 0UZ));

        // Nothing to step over at a zero width, so nothing is instantiated for it.
        if constexpr (N != 0UZ) {
                for (auto const i : std::views::iota(0UZ, N)) {
                        check_position(first, i, model);
                }
                BOOST_CHECK(as_vector(c) == model);
        }
}

// Proxy-to-proxy assignment copies the bit and never rebinds, which is what makes the swaps work.
BOOST_AUTO_TEST_CASE(ProxyAssignmentCopiesTheBitAndSwapSwapsTheBits)
{
        auto c            = Bits();
        auto const first  = iterator_at(c, 0UZ);
        auto const second = std::next(first);

        *first  = true;
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
        auto c           = Bits();
        auto const first = iterator_at(c, 0UZ);
        auto const last  = iterator_at(c, 200UZ);
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
        auto c     = Bits();
        auto model = std::vector<bool>(200);
        for (auto const p : {0UZ, 5UZ, 63UZ, 64UZ, 130UZ, 199UZ}) {
                c.set(p);
                model[p] = true;
        }
        auto const first = iterator_at(c, 0UZ);
        auto const last  = iterator_at(c, 200UZ);
        BOOST_CHECK(std::ranges::equal(std::ranges::subrange(first, last), model));

        static_assert(std::same_as<decltype(std::ranges::iter_move(first)), bool>);
        BOOST_CHECK_EQUAL(std::ranges::iter_move(first), true);

        std::ranges::sort(first, last);
        std::ranges::sort(model);
        BOOST_CHECK(as_vector(c) == model);

        std::ranges::reverse(first, last);
        std::ranges::reverse(model);
        BOOST_CHECK(as_vector(c) == model);

        // The pre-ranges algorithms on purpose: they reach the bits through std::iter_swap and the swap friends.
        std::sort(first, last); // NOLINT(modernize-use-ranges)
        std::ranges::sort(model);
        BOOST_CHECK(as_vector(c) == model);

        std::reverse(first, last); // NOLINT(modernize-use-ranges)
        std::ranges::reverse(model);
        BOOST_CHECK(as_vector(c) == model);

        std::ranges::iter_swap(first, std::prev(last));
        bool const t  = model.front();
        model.front() = model.back();
        model.back()  = t;
        BOOST_CHECK(as_vector(c) == model);

        // and the const twin is reachable by the reverse adaptor, being a proper bidirectional iterator.
        auto const rfirst = std::reverse_iterator(xstd::bits::detail::random_access_bit_iterator<Bits const>(last));
        BOOST_CHECK_EQUAL(static_cast<bool>(*rfirst), model.back());
}

// format_as is what the proxy's own std::formatter calls unqualified, and what fmt would call, so the test calls it so.
BOOST_AUTO_TEST_CASE(TheProxyFormatsAsItsValue)
{
        auto c = Bits();
        c.set(42);

        BOOST_CHECK_EQUAL(format_as(*iterator_at(c, 42UZ)), true);
        BOOST_CHECK_EQUAL(format_as(*iterator_at(std::as_const(c), 41UZ)), false);
}

// Every comparison is the built-in one on bool, reached through the one conversion, whatever the Block.
BOOST_AUTO_TEST_CASE(TheProxyComparesThroughItsOneConversion)
{
        test::for_each_type<test::all_block_types>([]<class Block> -> void {
                using reference       = block_reference<Block>;
                using const_reference = xstd::bits::detail::random_access_bit_reference<block_bits<Block> const>;
                using value_type      = reference::value_type;

                static_assert(std::equality_comparable<reference>);
                static_assert(std::totally_ordered<reference>);
                static_assert(std::totally_ordered<const_reference>);
                static_assert(std::totally_ordered_with<reference, value_type>);
                static_assert(equal_to_with<reference, int>);
                static_assert(less_than_with<reference, int>);

                static_assert(not std::is_convertible_v<reference, flag>);
                static_assert(std::is_constructible_v<flag, reference>);
                static_assert(std::is_convertible_v<reference, std::optional<value_type>>);
        });

        static_assert(std::totally_ordered_with<block_reference<std::uint8_t>, block_reference<std::uint64_t>>);

#if defined(TEST_HAS_ABSL_INT128) && defined(TEST_HAS_BOOST_INT128)

        static_assert(std::totally_ordered_with<block_reference<absl::uint128>, block_reference<boost::int128::uint128>>);

#endif

        auto narrow = block_bits<std::uint8_t>();
        auto wide   = block_bits<std::uint64_t>();
        narrow.set(1);
        wide.set(1);
        BOOST_CHECK(*iterator_at(narrow, 1UZ) == *iterator_at(wide, 1UZ));
        BOOST_CHECK(*iterator_at(narrow, 0UZ) < *iterator_at(wide, 1UZ));
        BOOST_CHECK(*iterator_at(narrow, 0UZ) != true);
}

// acme's operators decide a comparison between its own storages, and none between the proxies or iterators over one.
BOOST_AUTO_TEST_CASE(AStoragesNamespaceIsNotAssociatedWithItsProxies)
{
        auto const lhs = acme::blocks<std::uint64_t>();
        auto const rhs = acme::blocks<std::uint64_t>();
        BOOST_CHECK(not(lhs == rhs));
        BOOST_CHECK(not(lhs < rhs));

        static_assert(hostile_storage_compares_as_ours());
        BOOST_CHECK(hostile_storage_compares_as_ours());
}

// Every sequence and view hands out a pair that * and & close, over its storage mutable and const.
BOOST_AUTO_TEST_CASE(EverySequenceClosesItsProxyPair)
{
        static_assert(test::closed_proxies<xstd::basic_bit_array<std::uint64_t, 65>>);
        static_assert(test::closed_proxies<xstd::basic_bit_vector<std::uint64_t>>);
        static_assert(test::closed_proxies<xstd::basic_bit_bounded_vector<std::uint64_t, 65>>);
        static_assert(test::closed_proxies<xstd::basic_bit_small_vector<std::uint64_t, 65>>);
        static_assert(test::closed_proxies<xstd::bit_span<std::array<std::uint64_t, 2>>>);
        static_assert(test::closed_proxies<xstd::bit_span<std::array<std::uint64_t, 2> const>>);
        static_assert(test::closed_proxies<xstd::bit_subspan<std::array<std::uint64_t, 2>>>);
        static_assert(test::closed_proxies<xstd::bit_subspan<std::array<std::uint64_t, 2> const>>);
        static_assert(test::closed_proxies<xstd::bits::detail::sequence_adaptor<hostile_bits>>);

        BOOST_CHECK(true);
}

// What std::formatter is specialized for: the proxies themselves, which no deduction reaches through the class.
BOOST_AUTO_TEST_CASE(TheFormatterIsSpecializedForExactlyTheProxies)
{
        static_assert(xstd::bits::detail::random_access_reference<xstd::bits::detail::random_access_bit_reference<Bits>>);
        static_assert(xstd::bits::detail::random_access_reference<xstd::bits::detail::random_access_bit_reference<Bits const>>);
        static_assert(not xstd::bits::detail::random_access_reference<xstd::bits::detail::random_access_bit_iterator<Bits>>);
        static_assert(not xstd::bits::detail::random_access_reference<bool>);
        static_assert(not xstd::bits::detail::random_access_reference<std::vector<bool>::reference>);

        static_assert(std::formattable<xstd::bits::detail::random_access_bit_reference<Bits>, char>);
        static_assert(std::formattable<xstd::bits::detail::random_access_bit_reference<hostile_bits>, char>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

// The sequence view hands out this proxy and nothing of its own.
BOOST_AUTO_TEST_SUITE(RandomAccessThroughTheView)

namespace {

using Viewed = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 1>, 64>;

using ArrIt  = xstd::bits::detail::random_access_bit_iterator<Viewed>;
using ArrRef = xstd::bits::detail::random_access_bit_reference<Viewed>;

// Dependent, so a type without the member is a substitution failure rather than a hard error.
template<class R>
constexpr bool has_address_of = requires (R r) { r.operator&(); };

} // namespace

BOOST_AUTO_TEST_CASE(TheViewIteratesWithTheSharedProxy)
{
        static_assert(std::same_as<xstd::bit_span<std::array<std::uint64_t, 1>>::iterator, ArrIt>);
        static_assert(std::same_as<xstd::bit_span<std::array<std::uint64_t, 1>>::reference, ArrRef>);

        BOOST_CHECK(true);
}

// One shape asked twice: * gives a proxy, & an iterator back, and only our own guarantees are asserted.
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
        using ConstArrRef = xstd::bits::detail::random_access_bit_reference<Viewed const>;

        static_assert(std::same_as<xstd::bit_span<std::array<std::uint64_t, 1> const>::reference, ConstArrRef>);
        static_assert(std::is_convertible_v<ConstArrRef, bool>);
        static_assert(has_address_of<ConstArrRef>);
        static_assert(std::same_as<decltype(&std::declval<ConstArrRef const&>()), xstd::bits::detail::random_access_bit_iterator<Viewed const>>);

        // and it is exactly the assignment that the const one drops.
        static_assert(std::is_assignable_v<ArrRef const&, bool>);
        static_assert(not std::is_assignable_v<ConstArrRef const&, bool>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheValueArrivesByImplicitConversion)
{
        static_assert(std::is_convertible_v<ArrRef, bool>);

        auto b = Viewed();
        b.set(3);
        b.set(5);

        auto const a   = xstd::bit_span(b);
        bool const bit = a[3];
        BOOST_CHECK(bit);
        BOOST_CHECK(a[5] == true);
        BOOST_CHECK(a[4] == false);
}

// ... but not to an integer, however class-shaped it is.
#if defined(TEST_HAS_MSVC_INT128) || defined(TEST_HAS_ABSL_INT128) || defined(TEST_HAS_BOOST_INT128)

BOOST_AUTO_TEST_CASE(AProxyNeverBecomesAnIntegerBlock)
{
#ifdef TEST_HAS_MSVC_INT128

        {
                using B = xstd::basic_bit_array<xstd::uint128, 257>;
                static_assert(std::convertible_to<B::reference, bool>);
                static_assert(not std::convertible_to<B::reference, xstd::uint128>);
                static_assert(std::equality_comparable<B::reference>);
                static_assert(std::equality_comparable<B::const_reference>);

                auto a = B();
                a[0]   = true;
                a[256] = true;
                BOOST_CHECK(a[0] == a[256]);
                BOOST_CHECK(a[0] != a[1]);
                BOOST_CHECK(a[0] == true);
        }

#endif
#ifdef TEST_HAS_ABSL_INT128

        {
                using B = xstd::basic_bit_array<absl::uint128, 257>;
                static_assert(std::convertible_to<B::reference, bool>);
                static_assert(not std::convertible_to<B::reference, absl::uint128>);
                static_assert(std::equality_comparable<B::reference>);
                static_assert(std::equality_comparable<B::const_reference>);

                auto a = B();
                a[0]   = true;
                a[256] = true;
                BOOST_CHECK(a[0] == a[256]);
                BOOST_CHECK(a[0] != a[1]);
                BOOST_CHECK(a[0] == true);
        }

#endif
#ifdef TEST_HAS_BOOST_INT128

        {
                using B = xstd::basic_bit_array<boost::int128::uint128, 257>;
                static_assert(std::convertible_to<B::reference, bool>);
                static_assert(not std::convertible_to<B::reference, boost::int128::uint128>);
                static_assert(std::equality_comparable<B::reference>);
                static_assert(std::equality_comparable<B::const_reference>);

                auto a = B();
                a[0]   = true;
                a[256] = true;
                BOOST_CHECK(a[0] == a[256]);
                BOOST_CHECK(a[0] != a[1]);
                BOOST_CHECK(a[0] == true);
                BOOST_CHECK(a[0] == 1);
                BOOST_CHECK(a[0] > false);
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
