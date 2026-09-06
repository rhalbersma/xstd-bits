//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/block_types.hpp>                   // graded_extents
#include <test/minimal_traits.hpp>                // minimal_traits
#include <test/value_reference.hpp>               // value_reference
#include <xstd/bits/bit_proxy.hpp>                // bit_sequence_iterator, bit_sequence_reference, bit_set_iterator, bit_set_reference
#include <xstd/bits/bit_traits.hpp>               // bit_traits, find_next, find_prev
#include <xstd/bits/block_sequence.hpp>           // block_array
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_traits over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bit_traits over std::bitset
#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <algorithm>                              // equal, ranges::reverse, ranges::sort, reverse, sort
#include <bitset>                                 // bitset
#include <concepts>                               // bidirectional_iterator, convertible_to, random_access_iterator, same_as, sortable
#include <cstddef>                                // ptrdiff_t, size_t
#include <cstdint>                                // uint8_t, uint64_t
#include <iterator>                               // iter_move, next, prev, reverse_iterator
#include <ranges>                                 // subrange
#include <set>                                    // set
#include <type_traits>                            // is_assignable_v, is_const_v, is_convertible_v, is_trivially_copy_constructible_v, is_trivially_destructible_v
#include <vector>                                 // vector

namespace {

template<std::size_t N, class Block>
using array_of = xstd::block_array<Block, N>;

// Strong types to receive what the proxies convert to: one that takes a size_t implicitly, one only explicitly, and a flag.
// Copy-initialized, never cast: a cast is a direct-initialization with two routes in, and MSVC calls that no route at all.
struct key
{
        std::size_t value;
        constexpr explicit(false) key(std::size_t v) noexcept : value(v) {}  // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
};

struct index
{
        std::size_t value;
        constexpr explicit index(std::size_t v) noexcept : value(v) {}
};

struct flag
{
        bool value;
        constexpr explicit(false) flag(bool v) noexcept : value(v) {}  // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
};

template<class T>
[[nodiscard]] auto make(T const& empty, std::set<std::size_t> const& model) -> T
{
        auto c = empty;
        for (auto const p : model) {
                c.set(p);
        }
        return c;
}

// The two steps, asked directly at every position an iterator never reaches; only for a trait whose entries are total.
template<class Traits, class T>
auto check_steps_are_total(T const& c, std::set<std::size_t> const& model) -> void
{
        auto const size = Traits::size(c);
        for (auto n = 0UZ; n <= size + 1UZ; ++n) {
                auto const above = model.upper_bound(n);
                BOOST_CHECK_EQUAL(xstd::detail::bits::find_next<Traits>(c, n), above == model.end() ? size : *above);

                auto const below = model.lower_bound(n < size ? n : size);
                BOOST_CHECK_EQUAL(xstd::detail::bits::find_prev<Traits>(c, n), below == model.begin() ? size : *std::prev(below));
        }
}

template<class Traits, class Iterator>
auto check_set_steps(Iterator first, Iterator last, std::set<std::size_t> const& model) -> void;

// A zero width has nothing to step over, so nothing below the two positions is instantiated for it. [design.md#per-instantiation-slots]
template<class Traits, bool Total, class T>
auto check_set_walk(T const& empty, std::set<std::size_t> const& model) -> void
{
        using iterator = xstd::bit_set_iterator<T, Traits>;
        auto const c = make(empty, model);
        auto const size = Traits::size(c);

        auto const first = iterator(&c, model.empty() ? size : *model.begin());
        auto const last  = iterator(&c, size);
        BOOST_CHECK((first == last) == model.empty());

        if constexpr (Total) {
                check_steps_are_total<Traits>(c, model);
        }
        // Behind if constexpr rather than after an early return, or MSVC reports the rest unreachable at a zero width, which it is.
        if constexpr (Traits::extent != 0UZ) {
                check_set_steps<Traits>(first, last, model);
        }
}

template<class Traits, class Iterator>
auto check_set_steps(Iterator first, Iterator last, std::set<std::size_t> const& model) -> void
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
template<class Traits, bool Total, class T>
auto check_every_set_pattern(T const& empty) -> void
{
        check_set_walk<Traits, Total>(empty, {});

        // Behind if constexpr, or MSVC's analyzer reports loops whose body never runs at a zero width, which is so.
        if constexpr (Traits::extent != 0UZ) {
                auto const size = Traits::size(empty);
                auto full = std::set<std::size_t>();
                for (auto i = 0UZ; i < size; ++i) {
                        full.insert(i);
                }
                check_set_walk<Traits, Total>(empty, full);

                for (auto i = 0UZ; i < size; ++i) {
                        check_set_walk<Traits, Total>(empty, { i });
                        if (i + 1UZ < size) {
                                check_set_walk<Traits, Total>(empty, { i, i + 1UZ });
                        }
                }
        }
}

// One position: written, read back two ways, negated back through itself, and reached again through the subscript.
template<class Iterator>
auto check_position(Iterator first, std::size_t i, std::vector<bool>& model) -> void
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
[[nodiscard]] auto as_vector(T const& c) -> std::vector<bool>
{
        using Traits = xstd::bit_traits<T>;
        auto v = std::vector<bool>(Traits::size(c));
        for (auto i = 0UZ; i < v.size(); ++i) {
                v[i] = Traits::at(c, i);
        }
        return v;
}

}       // namespace

BOOST_AUTO_TEST_SUITE(BitProxy)

using ArrayTypes = test::graded_extents<array_of>;

using Bits = xstd::block_array<std::uint64_t, 200>;

BOOST_AUTO_TEST_CASE(AnIteratorIsAPointerAndAPosition)
{
        constexpr auto two_words = 2UZ * sizeof(void*);

        static_assert(sizeof(xstd::bit_set_iterator<Bits>)             == two_words);
        static_assert(sizeof(xstd::bit_set_reference<Bits>)            == two_words);
        static_assert(sizeof(xstd::bit_sequence_iterator<Bits>)        == two_words);
        static_assert(sizeof(xstd::bit_sequence_reference<Bits>)       == two_words);
        static_assert(sizeof(xstd::bit_sequence_iterator<Bits const>)  == two_words);
        static_assert(sizeof(xstd::bit_sequence_reference<Bits const>) == two_words);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetIteratorIsBidirectionalAndTheSequenceIteratorRandomAccess, T, ArrayTypes)
{
        static_assert(std::bidirectional_iterator<xstd::bit_set_iterator<T>>);
        static_assert(std::bidirectional_iterator<xstd::bit_set_iterator<T, test::minimal_traits<T>>>);

        static_assert(std::random_access_iterator<xstd::bit_sequence_iterator<T>>);
        static_assert(std::random_access_iterator<xstd::bit_sequence_iterator<T const>>);
        static_assert(std::random_access_iterator<xstd::bit_sequence_iterator<T, test::minimal_traits<T>>>);

        static_assert(    std::sortable<xstd::bit_sequence_iterator<T>>);
        static_assert(not std::sortable<xstd::bit_sequence_iterator<T const>>);
}

// The same over the two foreign types, which is what bit_traits is for.
BOOST_AUTO_TEST_CASE(TheForeignTypesIterateThroughTheirTraits)
{
        static_assert(std::bidirectional_iterator<xstd::bit_set_iterator<std::bitset<9>>>);
        static_assert(std::bidirectional_iterator<xstd::bit_set_iterator<boost::dynamic_bitset<>>>);

        static_assert(std::random_access_iterator<xstd::bit_sequence_iterator<std::bitset<9>>>);
        static_assert(std::random_access_iterator<xstd::bit_sequence_iterator<boost::dynamic_bitset<>>>);

        static_assert(std::sortable<xstd::bit_sequence_iterator<std::bitset<9>>>);
        static_assert(std::sortable<xstd::bit_sequence_iterator<boost::dynamic_bitset<>>>);
}

// Const is in the Bits, not in a flag, and a trait with only the required entries has no way to write either.
BOOST_AUTO_TEST_CASE(ConstnessLivesInTheBitsAndWritabilityInTheTraits)
{
        using Ref      = xstd::bit_sequence_reference<Bits>;
        using ConstRef = xstd::bit_sequence_reference<Bits const>;
        using MinimalRef = xstd::bit_sequence_reference<Bits, test::minimal_traits<Bits>>;

        static_assert(    std::is_assignable_v<Ref const&, bool>);
        static_assert(not std::is_assignable_v<ConstRef const&, bool>);
        static_assert(not std::is_assignable_v<MinimalRef const&, bool>);

        static_assert(std::is_convertible_v<Ref, bool>);
        static_assert(std::is_convertible_v<ConstRef, bool>);
        static_assert(std::is_convertible_v<MinimalRef, bool>);

        // The set proxy never writes, so nothing distinguishes its const spelling.
        static_assert(not std::is_assignable_v<xstd::bit_set_reference<Bits> const&, std::size_t>);
        static_assert(std::is_convertible_v<xstd::bit_set_reference<Bits>, std::size_t>);
        static_assert(std::is_convertible_v<xstd::bit_set_reference<Bits const>, std::size_t>);
}

// What a container's const_reference must be: trivially copyable, never assignable, comparable by value.
BOOST_AUTO_TEST_CASE(TheReadOnlyProxiesAreValues)
{
        static_assert(test::value_reference<xstd::bit_set_reference<Bits>>);
        static_assert(test::value_reference<xstd::bit_set_reference<Bits const>>);
        static_assert(test::value_reference<xstd::bit_sequence_reference<Bits const>>);
        static_assert(test::value_reference<xstd::bit_sequence_reference<Bits, test::minimal_traits<Bits>>>);

        // The writable proxy is the one exception, by design: its assignment writes the bit. Trivial to copy and destroy all the same.
        static_assert(not test::value_reference<xstd::bit_sequence_reference<Bits>>);
        static_assert(std::is_trivially_copy_constructible_v<xstd::bit_sequence_reference<Bits>>);
        static_assert(std::is_trivially_destructible_v<xstd::bit_sequence_reference<Bits>>);
        static_assert(std::is_trivially_destructible_v<xstd::bit_sequence_iterator<Bits>> and std::is_trivially_destructible_v<xstd::bit_set_iterator<Bits>>);
}

BOOST_AUTO_TEST_CASE(AMutableSequenceIteratorConvertsToItsConstTwin)
{
        using It      = xstd::bit_sequence_iterator<Bits>;
        using ConstIt = xstd::bit_sequence_iterator<Bits const>;

        static_assert(    std::convertible_to<It, ConstIt>);
        static_assert(not std::convertible_to<ConstIt, It>);

        auto b = Bits();
        auto const it = It(&b, 7UZ);
        ConstIt const cit = it;
        BOOST_CHECK(cit == ConstIt(&b, 7UZ));
        BOOST_CHECK(*cit == false);
}

// The native trait keeps block_sequence's preconditions and is stepped within them; the minimal trait is total and asked everything.
// The minimal trait from width 2, where an iterator first reaches the element-wise forward walk rather than its width guard.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetIteratorWalksThePositionsInBothDirections, T, ArrayTypes)
{
        check_every_set_pattern<xstd::bit_traits<T>, false>(T());
        if constexpr (xstd::bit_traits<T>::extent >= 2UZ) {
                check_every_set_pattern<test::minimal_traits<T>, true>(T());
        }
}

// std::bitset scans forward natively and backward synthesized on libstdc++, boost the same, and both traits are total.
BOOST_AUTO_TEST_CASE(TheForeignSetIteratorsWalkThePositionsInBothDirections)
{
        check_every_set_pattern<xstd::bit_traits<std::bitset<0>>, true>(std::bitset<0>());
        check_every_set_pattern<xstd::bit_traits<std::bitset<1>>, true>(std::bitset<1>());
        check_every_set_pattern<xstd::bit_traits<std::bitset<65>>, true>(std::bitset<65>());
        check_every_set_pattern<xstd::bit_traits<boost::dynamic_bitset<>>, true>(boost::dynamic_bitset<>(66));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSequenceIteratorReadsAndWritesThroughTheTraits, T, ArrayTypes)
{
        constexpr auto N = xstd::bit_traits<T>::extent;

        auto c = T();
        // Written through check_position below, which the check cannot see past a dependent call. [design.md#clang-tidy-false-positives]
        auto model = std::vector<bool>(N);  // NOLINT(misc-const-correctness)
        auto const first = xstd::bit_sequence_iterator<T>(&c, 0UZ);
        BOOST_CHECK(first == xstd::bit_sequence_iterator<T const>(&c, 0UZ));

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
        auto const first = xstd::bit_sequence_iterator<Bits>(&c, 0UZ);
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
        auto const first = xstd::bit_sequence_iterator<Bits>(&c, 0UZ);
        auto const last  = xstd::bit_sequence_iterator<Bits>(&c, 200UZ);
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
        using iterator = xstd::bit_sequence_iterator<Bits>;

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
        auto const rfirst = std::reverse_iterator(xstd::bit_sequence_iterator<Bits const>(last));
        BOOST_CHECK_EQUAL(static_cast<bool>(*rfirst), model.back());
}

// The foreign sequence proxies write through their own unchecked subscript.
BOOST_AUTO_TEST_CASE(TheForeignSequenceProxiesWriteThroughTheTraits)
{
        auto s = std::bitset<9>();
        auto const sit = xstd::bit_sequence_iterator<std::bitset<9>>(&s, 4UZ);
        *sit = true;
        BOOST_CHECK(s.test(4));
        *sit = false;
        BOOST_CHECK(not s.test(4));

        auto d = boost::dynamic_bitset<>(9);
        auto const dit = xstd::bit_sequence_iterator<boost::dynamic_bitset<>>(&d, 8UZ);
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
BOOST_AUTO_TEST_CASE(TheProxiesFormatAsTheirValues)
{
        auto c = Bits();
        c.set(42);

        BOOST_CHECK_EQUAL(format_as(*xstd::bit_set_iterator<Bits>(&c, 42UZ)), 42UZ);
        BOOST_CHECK_EQUAL(format_as(*xstd::bit_sequence_iterator<Bits>(&c, 42UZ)), true);
        BOOST_CHECK_EQUAL(format_as(*xstd::bit_sequence_iterator<Bits const>(&c, 41UZ)), false);
}

BOOST_AUTO_TEST_SUITE_END()
