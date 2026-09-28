//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/reference.hpp>        // proxy_reference
#include <test/sequence/factory.hpp> // make_sequence, stripes
#include <test/spec/sequence.hpp>    // array_all
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                  // convertible_to, derived_from, same_as
#include <cstddef>                   // size_t
#include <tuple>                     // tuple_element_t, tuple_size, tuple_size_v
#include <type_traits>               // integral_constant
#include <utility>                   // as_const, move

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Tuple)

using namespace test::sequence;

namespace {

// [array.tuple] asks T of tuple_element, which a proxy relaxes to a type converting to T.
template<std::size_t I, class X>
auto check_tuple_element()
        -> void
{
        static_assert(std::same_as<std::tuple_element_t<I, X>, bool> or (test::proxy_reference<X> and std::convertible_to<std::tuple_element_t<I, X>, bool>)); // [array.tuple]/1
        static_assert(std::same_as<std::tuple_element_t<I, X const>, bool const> or (test::proxy_reference<X> and std::convertible_to<std::tuple_element_t<I, X const>, bool>));
}

// All four overloads of get<I> read position I, and the mutable one writes it.
template<std::size_t I, class X>
auto check_get(X const& a)
        -> void
{
        auto const expected = static_cast<bool>(a[I]);
        auto b = a;
        BOOST_CHECK_EQUAL(static_cast<bool>(get<I>(b)), expected); // [array.tuple]/3
        BOOST_CHECK_EQUAL(static_cast<bool>(get<I>(std::as_const(b))), expected);
        BOOST_CHECK_EQUAL(static_cast<bool>(get<I>(X(a))), expected);
        BOOST_CHECK_EQUAL(static_cast<bool>(get<I>(std::move(std::as_const(b)))), expected);

        get<I>(b) = not expected;
        BOOST_CHECK_EQUAL(static_cast<bool>(b[I]), not expected);
}

} // namespace

// [array.tuple]: template<class T, size_t N> struct tuple_size<array<T, N>> : integral_constant<size_t, N> { };
BOOST_AUTO_TEST_CASE(TupleSize)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, T().size()>>);
                BOOST_CHECK(true);
        });
}

// [array.tuple]/1: template<size_t I, class T, size_t N> struct tuple_element<I, array<T, N>> { using type = T; };
BOOST_AUTO_TEST_CASE(TupleElement)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                constexpr auto N = std::tuple_size_v<T>;
                BOOST_CHECK_EQUAL(N, T().size());
                if constexpr (N > 0UZ) {
                        check_tuple_element<0UZ, T>();
                        check_tuple_element<N / 2UZ, T>();
                        check_tuple_element<N - 1UZ, T>();
                }
        });
}

// [array.tuple]/2-3: template<size_t I, class T, size_t N> constexpr T& get(array<T, N>& a) noexcept;
BOOST_AUTO_TEST_CASE(Get)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // The first, a middle and the last position: what an index in the type reaches without a sweep.
                constexpr auto N = T().size();
                if constexpr (N > 0UZ) {
                        static_assert(requires (T c, T const cc) {
                                get<0>(c);
                                get<0>(cc);
                                get<0>(std::move(c));
                        });
                        auto const a = make_sequence<T>(N, stripes);
                        check_get<0UZ>(a);
                        check_get<N / 2UZ>(a);
                        check_get<N - 1UZ>(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
