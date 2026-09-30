//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/constexpr_check.hpp>      // XSTD_CONSTEXPR_CHECK
#include <test/for_each_type.hpp>        // for_each_type
#include <test/inplace_vector.hpp>       // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/set/exhaustive.hpp>       // static_width
#include <test/spec/set.hpp>             // all
#include <test/spec/view.hpp>            // owner_t, view_type
#include <xstd/bits/bit_bounded_set.hpp> // IWYU pragma: keep; basic_bit_bounded_set
#include <xstd/bits/bit_set.hpp>         // basic_bit_set
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <algorithm>                     // min
#include <concepts>                      // same_as
#include <cstddef>                       // IWYU pragma: keep; size_t
#include <iterator>                      // bidirectional_iterator, distance
#include <ranges>                        // bidirectional_range, iota
#include <set>                           // IWYU pragma: keep; set
#include <utility>                       // cmp_equal, pair
#include <version>                       // IWYU pragma: keep; __cpp_lib_constexpr_set

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Overview)

namespace {

// A set that a constant expression can hold: a literal type, or one whose allocations end with the evaluation.
template<class X>
inline constexpr bool constant_evaluable = test::set::static_width<X>;

#ifdef TEST_HAS_INPLACE_VECTOR

// The bounded sets hold their blocks in std::inplace_vector where there is one, and in a static_vector otherwise.
template<class Block, std::size_t N>
inline constexpr bool constant_evaluable<xstd::basic_bit_bounded_set<Block, N>> = true;

#endif

template<class Block, class Allocator>
inline constexpr bool constant_evaluable<xstd::basic_bit_set<Block, Allocator>> = true;

#if defined(__cpp_lib_constexpr_set) && __cpp_lib_constexpr_set >= 202502L

template<class Key, class Compare, class Allocator>
inline constexpr bool constant_evaluable<std::set<Key, Compare, Allocator>> = true;

#endif

// Up to three keys, walked to the end and back again, through a view over the owner where the candidate is one.
template<class X>
[[nodiscard]] constexpr auto walks_both_ways()
        -> bool
{
        auto owner = test::spec::owner_t<X>();
        for (auto const k : std::views::iota(0UZ, std::ranges::min(owner.max_size(), 3UZ))) {
                owner.insert(k);
        }
        auto const walks = [](auto const& x) -> bool {
                return std::cmp_equal(std::distance(x.begin(), x.end()), x.size()) and std::cmp_equal(std::distance(x.rbegin(), x.rend()), x.size());
        };
        if constexpr (test::spec::view_type<X>) {
                return walks(X(owner));
        } else {
                return walks(owner);
        }
}

} // namespace

// [set.overview]/1-3: template<class Key, class Compare = less<Key>, class Allocator = allocator<Key>> class set;
BOOST_AUTO_TEST_CASE(Set)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(std::ranges::bidirectional_range<T> and std::bidirectional_iterator<typename T::iterator>); // [set.overview]/1
                static_assert(std::same_as<typename T::key_type, typename T::value_type>);                                // [set.overview]/2
                static_assert(requires (T a, T::key_type k) { { a.insert(k) } -> std::same_as<std::pair<typename T::iterator, bool>>; });                                                         // [set.overview]/2
                if constexpr (constant_evaluable<test::spec::owner_t<T>>) {
                        XSTD_CONSTEXPR_CHECK(walks_both_ways<T>()); // [set.overview]/3
                } else {
                        BOOST_CHECK(walks_both_ways<T>()); // [set.overview]/3
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
