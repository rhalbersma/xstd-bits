//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/sequence/factory.hpp> // model_of, stripes
#include <test/spec/sequence.hpp>    // array_all
#include <xstd/bits/bit_array.hpp>   // bit_array, to_bit_array
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <algorithm>                 // equal
#include <array>                     // array, to_array, tuple_size_v
#include <concepts>                  // same_as
#include <cstddef>                   // size_t
#include <ranges>                    // begin, end, iota
#include <type_traits>               // conditional_t
#include <utility>                   // forward, move
#include <vector>                    // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Creation)

using namespace test::sequence;

namespace {

template<class T>
constexpr bool is_std_array = std::same_as<T, std::array<bool, std::tuple_size_v<T>>>;

// The creation function of T's own column: std::to_array for std::array, xstd::to_bit_array for a packed one.
template<class T, class A>
[[nodiscard]] constexpr auto to_array_of(A&& a)
{
        if constexpr (is_std_array<T>) {
                return std::to_array(std::forward<A>(a));
        } else {
                return xstd::to_bit_array(std::forward<A>(a));
        }
}

// What it returns: a std::array<bool, N> for the standard's, and a bit_array<N> whatever block the packed column has.
template<class T>
using created_t = std::conditional_t<is_std_array<T>, T, xstd::bit_array<std::tuple_size_v<T>>>;

// The stripes as a built-in array, which has no width of nought to be declared at.
template<std::size_t N>
struct striped
{
        bool a[N]; // NOLINT(modernize-avoid-c-arrays): a built-in array is what the creation functions take.

        [[nodiscard]] constexpr striped() noexcept
                : a()
        {
                for (auto const i : std::views::iota(0UZ, N)) {
                        a[i] = stripes(i);
                }
        }
};

} // namespace

// [array.creation]/3: template<class T, size_t N> constexpr array<remove_cv_t<T>, N> to_array(T (&a)[N]);
BOOST_AUTO_TEST_CASE(ToArrayLvalue)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                constexpr auto N = T().size();
                if constexpr (N != 0UZ) {
                        auto s        = striped<N>();
                        auto const& c = s;
                        static_assert(std::same_as<decltype(to_array_of<T>(s.a)), created_t<T>> and std::same_as<decltype(to_array_of<T>(c.a)), created_t<T>>); // [array.creation]/3
                        static_assert([] -> bool {
                                auto const t = striped<T().size()>();
                                return std::ranges::equal(to_array_of<T>(t.a), t.a);
                        }());
                        BOOST_CHECK(std::ranges::equal(to_array_of<T>(s.a), s.a)); // [array.creation]/3
                        BOOST_CHECK(std::ranges::equal(to_array_of<T>(c.a), c.a)); // [array.creation]/3
                }
        });
}

// [array.creation]/6: template<class T, size_t N> constexpr array<remove_cv_t<T>, N> to_array(T (&&a)[N]);
BOOST_AUTO_TEST_CASE(ToArrayRvalue)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                constexpr auto N = T().size();
                if constexpr (N != 0UZ) {
                        auto s           = striped<N>();
                        auto const model = std::vector<bool>(std::ranges::begin(s.a), std::ranges::end(s.a));
                        static_assert(std::same_as<decltype(to_array_of<T>(std::move(s.a))), created_t<T>>); // [array.creation]/6
                        BOOST_CHECK(model_of(to_array_of<T>(std::move(s.a))) == model);                      // [array.creation]/6
                }
                if constexpr (is_std_array<T>) {
                        BOOST_CHECK(model_of(std::to_array({true, false, true})) == std::vector<bool>({true, false, true})); // [array.creation]/6
                } else {
                        BOOST_CHECK(model_of(xstd::to_bit_array({true, false, true})) == std::vector<bool>({true, false, true})); // [array.creation]/6
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
