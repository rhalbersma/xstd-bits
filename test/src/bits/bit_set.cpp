//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/ascending.hpp>                     // yields_ascending_keys
#include <test/set/concepts.hpp>                      // bit_set, set_size_t, set_size_t_allocator, set_size_t_ranges, set_size_t_ranges_allocator
#include <xstd/bits/bit_set.hpp>                      // bit_set
#include <xstd/bits/bit_set_view.hpp>                 // bit_set_view
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/ownership.hpp>                    // ownership
#include <xstd/bits/set_adaptor.hpp>                  // set_adaptor
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                  // equal, ranges::equal
#include <array>                                      // array
#include <bitset>                                     // bitset
#include <compare>                                    // is_eq
#include <concepts>                                   // same_as
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint8_t
#include <functional>                                 // hash
#include <memory>                                     // allocator
#include <ranges>                                     // equal, iota, to
#include <set>                                        // set
#include <type_traits>                                // is_constructible_v
#include <utility>                                    // move

BOOST_AUTO_TEST_SUITE(BitSet)

using T = xstd::basic_bit_set<std::uint8_t>;

// The flagship: the set reading over a heap of blocks, an alias and nothing more.
BOOST_AUTO_TEST_CASE(TheDynamicSetIsTheSetAdaptorOverAHeapOfBlocks)
{
        static_assert(std::same_as<T, xstd::set_adaptor<xstd::detail::bits::contiguous_bit_vector<std::uint8_t>, xstd::ownership::owns>>);
        static_assert(std::same_as<xstd::basic_bit_set<std::uint8_t, std::allocator<std::uint8_t>>, T>);
        static_assert(test::set::bit_set<T>);
}

// Every line of [set], the model first so the checklist is known to be honest. std::set<std::size_t> is this
// column's counterpart, and the whole of it is answered but the node-based family and the heterogeneous overloads
// neither side has -- which is the same bargain std::flat_set strikes for the same reason.
BOOST_AUTO_TEST_CASE(ItAnswersEveryLineOfStdSetSizeT)
{
        static_assert(test::set::set_size_t<std::set<std::size_t>>);
        static_assert(test::set::set_size_t<T>);
        static_assert(test::set::set_size_t<xstd::bit_set>);

        static_assert(test::set::set_size_t_allocator<std::set<std::size_t>>);
        static_assert(test::set::set_size_t_allocator<T>);
        static_assert(test::set::set_size_t_allocator<xstd::bit_set>);

#ifdef __cpp_lib_containers_ranges

        static_assert(test::set::set_size_t_ranges<std::set<std::size_t>>);
        static_assert(test::set::set_size_t_ranges_allocator<std::set<std::size_t>>);

#endif
        static_assert(test::set::set_size_t_ranges<T>);
        static_assert(test::set::set_size_t_ranges_allocator<T>);
}

// [set.cons]'s allocator arguments, CONSTRUCTED rather than merely asked about: the checklist above names them in a
// requires-expression, and this reading had none of them at all until it was asked.
BOOST_AUTO_TEST_CASE(TheAllocatorConstructorsBuildWhatTheyName)
{
        using A = T::allocator_type;
        auto const a = A();
        auto const keys = std::array<std::size_t, 3>{1UZ, 3UZ, 5UZ};

        auto const x0 = T(a);
        BOOST_CHECK(x0.empty());

        auto const x1 = T(keys.begin(), keys.end(), a);
        BOOST_CHECK(std::ranges::equal(x1, keys));

        auto const x2 = T({1UZ, 3UZ, 5UZ}, a);
        BOOST_CHECK(x2 == x1);

        auto const x3 = T(x1, a);
        BOOST_CHECK(x3 == x1);

        auto y = x1;
        auto const x4 = T(std::move(y), a);
        BOOST_CHECK(x4 == x1);

        BOOST_CHECK(x1.get_allocator() == a);
}

// A key is default-constructible, so the empty argument list is one of the lists emplace has to take.
BOOST_AUTO_TEST_CASE(TheEmptyArgumentListEmplacesTheZeroKey)
{
        auto s = T();
        auto const [it, inserted] = s.emplace();
        BOOST_CHECK(inserted);
        BOOST_CHECK_EQUAL(*it, 0UZ);
        BOOST_CHECK(s.contains(0UZ));

        auto const hinted = s.emplace_hint(s.begin());
        BOOST_CHECK_EQUAL(*hinted, 0UZ);
        BOOST_CHECK_EQUAL(s.size(), 1UZ);
}

// A key past the width grows the width: insert is the one operation a dynamic set cannot refuse.
BOOST_AUTO_TEST_CASE(InsertingPastTheWidthGrowsIt)
{
        auto s = T();
        BOOST_CHECK(s.empty());
        BOOST_CHECK_EQUAL(s.max_size(), xstd::detail::bits::contiguous_bit_vector<std::uint8_t>().max_size());

        auto const [where, inserted] = s.insert(100);
        BOOST_CHECK(inserted);
        BOOST_CHECK(*where == 100UZ);
        BOOST_CHECK(s.contains(100));
        BOOST_CHECK_EQUAL(s.size(), 1UZ);

        // Erasing never shrinks the width, and asking below or above it stays total.
        BOOST_CHECK_EQUAL(s.erase(100), 1UZ);
        BOOST_CHECK(not s.contains(100));
        BOOST_CHECK(not s.contains(1000));
        BOOST_CHECK(s.find(1000) == s.end()); // NOLINT(readability-container-contains)
}

// Built from a range as std::set is, iterated as std::set is, and ordered as std::set is.
BOOST_AUTO_TEST_CASE(ItIsBuiltAndOrderedLikeAStdSet)
{
        auto const s = std::views::iota(0UZ, 300UZ) | std::views::filter([](auto i) { return i % 7 == 0; }) | std::ranges::to<T>();
        auto const k = std::views::iota(0UZ, 300UZ) | std::views::filter([](auto i) { return i % 7 == 0; }) | std::ranges::to<std::set<std::size_t>>();
        BOOST_CHECK(std::ranges::equal(s, k));

        auto t = s;
        t.insert(1);
        BOOST_CHECK(t < s);
        BOOST_CHECK(s.is_subset_of(t));
        BOOST_CHECK(intersects(t, s));

        // The view over it refers into the contiguous_bit_vector, as over every owner.
        auto const v = xstd::bit_set_view(t);
        BOOST_CHECK(*v.begin() == 0UZ);
        BOOST_CHECK_EQUAL(v.size(), t.size());
}

// The width is capacity, never value: two sets holding the same positions agree on everything std::set answers, whatever their storages' widths.
BOOST_AUTO_TEST_CASE(TheWidthIsCapacityNotValue)
{
        auto const narrow = T({1, 3});
        auto wide = T({1, 3});
        wide.insert(100);
        wide.erase(100);
        auto const digest = std::hash<T>();
        BOOST_CHECK(narrow == wide);
        BOOST_CHECK(std::is_eq(narrow <=> wide));
        BOOST_CHECK_EQUAL(digest(narrow), digest(wide));
        BOOST_CHECK(digest(narrow) != digest(T({1})));
        BOOST_CHECK(narrow.is_subset_of(wide) and wide.is_subset_of(narrow));
        BOOST_CHECK(not narrow.is_proper_subset_of(wide));
        BOOST_CHECK(intersects(narrow, wide));
}

// The compound operators and predicates at two widths that differ, either way round, against the answers over the elements.
BOOST_AUTO_TEST_CASE(TheSetOperationsIgnoreTheWidth)
{
        auto const a = T({1, 3, 200});
        auto const b = T({3, 5});
        BOOST_CHECK((a | b) == T({1, 3, 5, 200}));
        BOOST_CHECK((b | a) == T({1, 3, 5, 200}));
        BOOST_CHECK((a & b) == T({3}));
        BOOST_CHECK((b & a) == T({3}));
        BOOST_CHECK((a ^ b) == T({1, 5, 200}));
        BOOST_CHECK((b ^ a) == T({1, 5, 200}));
        BOOST_CHECK((a - b) == T({1, 200}));
        BOOST_CHECK((b - a) == T({5}));
        BOOST_CHECK(a < b);
        BOOST_CHECK(b.is_proper_subset_of(a | b));
        BOOST_CHECK(intersects(a | b, b));
        BOOST_CHECK(not b.is_subset_of(a));
        BOOST_CHECK(not a.is_subset_of(b));
        BOOST_CHECK(not a.is_proper_subset_of(b));
        BOOST_CHECK(not b.is_proper_subset_of(a));
        BOOST_CHECK(not intersects(T({5}), a));
        BOOST_CHECK(not intersects(a, T({5})));
}

// The shifts translate: left grows the width to hold the result, right empties past it, and neither has the width as a precondition.
BOOST_AUTO_TEST_CASE(TheShiftsTranslateWhateverTheWidth)
{
        auto const b = T({3, 5});
        BOOST_CHECK((b << 300) == T({303, 305}));
        BOOST_CHECK((b >> 4) == T({1}));
        BOOST_CHECK((b >> 300).empty());
        BOOST_CHECK((T() << 3).empty());
}

// Ascending keys, whatever the insertion order: what makes this a set rather than a bag of positions.
BOOST_AUTO_TEST_CASE(ItYieldsAscendingKeys)
{
        auto c = T();
        test::set::yields_ascending_keys(c); // empty is trivially ascending

        // Inserted high to low and across block boundaries, so the ascending answer is the container's doing and not the insertion order's.
        for (auto const key : {70UZ, 64UZ, 63UZ, 9UZ, 1UZ, 0UZ}) {
                c.insert(key);
        }
        test::set::yields_ascending_keys(c);
}

// A run-time width has neither std::bitset conversion, and that is the width policy and not an omission: a
// std::bitset names one N at compile time and a growing set has no single one to mean, so the question a
// static width answers exactly has no answer here at all. bit_static_set carries the pair; this does not.
BOOST_AUTO_TEST_CASE(AStdBitsetIsNoConversionAtARunTimeWidth)
{
        static_assert(not std::is_constructible_v<xstd::bit_set, std::bitset<64>>);
        static_assert(not std::is_constructible_v<std::bitset<64>, xstd::bit_set>);
}

BOOST_AUTO_TEST_SUITE_END()
