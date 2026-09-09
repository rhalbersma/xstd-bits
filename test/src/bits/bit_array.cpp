//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <test/block_types.hpp>       // graded_extents
#include <test/sequence/concepts.hpp> // bit_sequence
#include <test/value_reference.hpp>   // value_reference
#include <xstd/bits/bit_array.hpp>    // bit_array
#include <algorithm>                  // equal, none_of
#include <array>                      // array
#include <concepts>                   // regular, totally_ordered
#include <cstddef>                    // size_t
#include <functional>                 // hash, identity
#include <iterator>                   // random_access_iterator
#include <ranges>                     // drop, random_access_range, take
#include <stdexcept>                  // out_of_range
#include <vector>                     // vector

BOOST_AUTO_TEST_SUITE(BitArray)

// Every Block model within one block and the narrow ones across boundaries; the grading is in test/block_types.hpp.
using Types = test::graded_extents<xstd::basic_bit_array>;

// The clauses one at a time, so a failure names which one; the umbrella asserts the composite.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsTotallyOrdered, T, Types)
{
        static_assert(std::totally_ordered<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsARandomAccessRange, T, Types)
{
        static_assert(std::ranges::random_access_range<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsIteratorIsRandomAccess, T, Types)
{
        using I = T::iterator;
        static_assert(std::random_access_iterator<I>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ItsConstReferenceIsAValue, T, Types)
{
        static_assert(test::value_reference<typename T::const_reference>);
}

// Every owner hashes, this one although std::array<bool, N> does not: equal values equal, at every extent. [design.md#the-hashing-invariant]
BOOST_AUTO_TEST_CASE_TEMPLATE(ItHashesAsAnOwner, T, Types)
{
        auto const h = std::hash<T>();
        BOOST_CHECK_EQUAL(h(T()), h(T()));
        if constexpr (T().size() > 0UZ) {
                auto x = T();
                x[0] = true;
                BOOST_CHECK(h(x) != h(T()));
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsABitSequence, T, Types)
{
        static_assert(test::sequence::bit_sequence<T>);
}

// [array]'s synopsis line by line, the model first so the checklist is known to be honest. [design.md#the-sequence-contract]
static_assert(test::sequence::array_bool<std::array<bool, 5>>);

BOOST_AUTO_TEST_CASE_TEMPLATE(ItAnswersEveryLineOfStdArrayBool, T, Types)
{
        static_assert(test::sequence::array_bool<T>);
}

// std::array's aggregate initialization: what is listed leads and the rest stays false.
BOOST_AUTO_TEST_CASE_TEMPLATE(ItIsListInitializedLikeAStdArray, T, Types)
{
        if constexpr (T().size() >= 3UZ) {
                auto const a = T{ true, false, true };
                auto m = std::array<bool, 3>{ true, false, true };
                BOOST_CHECK(std::ranges::equal(a | std::views::take(3), m));
                BOOST_CHECK(std::ranges::none_of(a | std::views::drop(3), std::identity()));
        }
        BOOST_CHECK(T{} == T());
}

// The behavioural half, which this suite was missing while the bitset and set suites had theirs: every operation
// run on a bit_array and on the std::array<bool, N> it is held against, and the two compared. A synopsis
// checklist says the line exists; only this says it answers the same thing. [design.md#the-sequence-contract]
namespace {

// The model at the same extent, filled the same way, so any disagreement is the packing's.
template<class T>
auto model_of(T const& a)
        -> std::vector<bool>
{
        auto m = std::vector<bool>(a.size());
        for (auto i = 0UZ; i < a.size(); ++i) {
                m[i] = a[i];
        }
        return m;
}

// Every read path at every position, counted rather than asserted one at a time: a failure then names the
// operation instead of drowning the log in one line per position. [design.md#counted-not-asserted]
//
// Taken by non-const reference and aliased to a const one inside, because these take an explicit object
// parameter and the const and non-const paths are two different functions, both of them under test.
template<class T>
auto access_disagreements(T& a, std::vector<bool> const& m)
        -> std::size_t
{
        auto const& ca = a;
        auto disagreements = 0UZ;
        for (auto i = 0UZ; i < a.size(); ++i) {
                disagreements += static_cast<std::size_t>(static_cast<bool>(a[i])     != m[i]);
                disagreements += static_cast<std::size_t>(static_cast<bool>(ca[i])    != m[i]);
                disagreements += static_cast<std::size_t>(static_cast<bool>(a.at(i))  != m[i]);
                disagreements += static_cast<std::size_t>(static_cast<bool>(ca.at(i)) != m[i]);
        }
        return disagreements;
}

// front() and back() are the same four overloads over the two ends, and a zero extent has neither.
template<class T>
auto ends_disagreements(T& a, std::vector<bool> const& m)
        -> std::size_t
{
        if (a.empty()) {
                return 0UZ;
        }
        auto const& ca = a;
        return static_cast<std::size_t>(static_cast<bool>(a.front())  != m.front())
             + static_cast<std::size_t>(static_cast<bool>(ca.front()) != m.front())
             + static_cast<std::size_t>(static_cast<bool>(a.back())   != m.back())
             + static_cast<std::size_t>(static_cast<bool>(ca.back())  != m.back());
}

// One bit of pattern p at position i. A lookup rather than a conditional chain: six patterns written as a
// chain nest six deep, and the nesting is not what the test is about.
auto pattern_bit(std::size_t p, std::size_t i, std::size_t n)
        -> bool
{
        switch (p) {
        case 0UZ: return false;
        case 1UZ: return true;
        case 2UZ: return i == 0UZ;
        case 3UZ: return i + 1UZ == n;
        case 4UZ: return (i % 2UZ) == 0UZ;
        default:  return (i % 3UZ) == 0UZ;
        }
}

// Uniform both ways, single-ended both ways, and two strides: enough that every comparison lands on both
// sides of itself, and cheaper than every pair of values.
template<class T>
auto comparison_patterns()
        -> std::vector<T>
{
        auto patterns = std::vector<T>();
        for (auto p = 0UZ; p < 6UZ; ++p) {
                auto a = T();
                for (auto i = 0UZ; i < a.size(); ++i) {
                        a[i] = pattern_bit(p, i, a.size());
                }
                patterns.push_back(a);
        }
        return patterns;
}

}       // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(ElementAccessAgreesWithTheModel, T, Types)
{
        auto a = T();
        auto m = std::vector<bool>(a.size());

        // A deterministic pattern rather than a uniform one, so a block boundary lands mid-pattern.
        for (auto i = 0UZ; i < a.size(); ++i) {
                bool const bit = (i % 3UZ) == 1UZ;
                a[i] = bit;
                m[i] = bit;
        }
        BOOST_CHECK(std::ranges::equal(a, m));

        // Both subscripts and both ends, in both qualifications.
        BOOST_CHECK_EQUAL(access_disagreements(a, m), 0UZ);
        BOOST_CHECK_EQUAL(ends_disagreements(a, m),   0UZ);

        // at() is the checked one, and std::array<bool, N>::at throws in the same place.
        auto const& ca = a;
        BOOST_CHECK_THROW(static_cast<void>(a.at(a.size())),  std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(ca.at(a.size())), std::out_of_range);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheIteratorsAgreeWithTheModel, T, Types)
{
        auto a = T();
        for (auto i = 0UZ; i < a.size(); ++i) {
                a[i] = (i % 4UZ) < 2UZ;
        }
        auto const m = model_of(a);
        auto const& ca = a;

        BOOST_CHECK(std::ranges::equal(a, m));
        BOOST_CHECK(std::equal(a.begin(),   a.end(),   m.begin(),  m.end()));
        BOOST_CHECK(std::equal(a.cbegin(),  a.cend(),  m.begin(),  m.end()));
        BOOST_CHECK(std::equal(ca.begin(),  ca.end(),  m.begin(),  m.end()));
        BOOST_CHECK(std::equal(a.rbegin(),  a.rend(),  m.rbegin(), m.rend()));
        BOOST_CHECK(std::equal(a.crbegin(), a.crend(), m.rbegin(), m.rend()));
        BOOST_CHECK(std::equal(ca.rbegin(), ca.rend(), m.rbegin(), m.rend()));

        BOOST_CHECK_EQUAL(a.empty(), m.empty());
        BOOST_CHECK_EQUAL(a.size(),  m.size());
        BOOST_CHECK_EQUAL(a.max_size(), a.size());   // a fixed extent is its own capacity [design.md#width-is-capacity]
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FillAndSwapAgreeWithTheModel, T, Types)
{
        auto a = T();
        a.fill(true);
        BOOST_CHECK(std::ranges::equal(a, std::vector<bool>(a.size(), true)));
        a.fill(false);
        BOOST_CHECK(std::ranges::equal(a, std::vector<bool>(a.size(), false)));

        auto x = T();
        auto y = T();
        for (auto i = 0UZ; i < x.size(); ++i) {
                x[i] = (i % 2UZ) == 0UZ;
                y[i] = (i % 5UZ) == 0UZ;
        }
        auto const mx = model_of(x);
        auto const my = model_of(y);

        x.swap(y);
        BOOST_CHECK(std::ranges::equal(x, my));
        BOOST_CHECK(std::ranges::equal(y, mx));

        swap(x, y);
        BOOST_CHECK(std::ranges::equal(x, mx));
        BOOST_CHECK(std::ranges::equal(y, my));
}

// std::array<bool, N> orders lexicographically over its elements, and so must this. Every pair of a graded set
// of patterns, which is cheaper than every pair of values and lands on both sides of each comparison.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheComparisonsAgreeWithTheModel, T, Types)
{
        auto const patterns = comparison_patterns<T>();
        auto disagreements = 0UZ;

        for (auto const& x : patterns) {
                for (auto const& y : patterns) {
                        auto const mx = model_of(x);
                        auto const my = model_of(y);
                        disagreements += static_cast<std::size_t>((x == y) != (mx == my));
                        disagreements += static_cast<std::size_t>((x != y) != (mx != my));
                        disagreements += static_cast<std::size_t>((x <  y) != (mx <  my));
                        disagreements += static_cast<std::size_t>((x >  y) != (mx >  my));
                        disagreements += static_cast<std::size_t>((x <= y) != (mx <= my));
                        disagreements += static_cast<std::size_t>((x >= y) != (mx >= my));
                }
        }

        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

BOOST_AUTO_TEST_SUITE_END()
