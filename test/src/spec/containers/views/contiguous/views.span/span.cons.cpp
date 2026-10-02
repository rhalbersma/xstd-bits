//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/spec/input.hpp>       // context
#include <test/spec/span.hpp>        // all, pairs, read, same_position, views
#include <xstd/bits/bit_span.hpp>    // bit_span
#include <xstd/bits/bit_subspan.hpp> // bit_subspan
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                  // same_as
#include <cstddef>                   // size_t
#include <span>                      // dynamic_extent, span
#include <type_traits>               // is_const_v, is_constructible_v, is_convertible_v, is_nothrow_constructible_v, is_nothrow_copy_assignable_v, is_nothrow_copy_constructible_v

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Views)
BOOST_AUTO_TEST_SUITE(Contiguous)
BOOST_AUTO_TEST_SUITE(ViewsSpan)
BOOST_AUTO_TEST_SUITE(SpanCons)

using test::spec::context;
using test::spec::span::read;
using test::spec::span::same_position;
namespace inputs = test::spec::span::inputs;

namespace {

// The same candidate at another extent: std::span's element type or a window's blocks, and nothing for a whole view.
template<class T, std::size_t E>
struct at_extent
{};

template<class Element, std::size_t X, std::size_t E>
struct at_extent<std::span<Element, X>, E>
{
        using type = std::span<Element, E>;
};

template<class Blocks, std::size_t X, std::size_t N, std::size_t E>
struct at_extent<xstd::bit_subspan<Blocks, X, N>, E>
{
        using type = xstd::bit_subspan<Blocks, E, N>;
};

template<class T, std::size_t E>
using at_extent_t = at_extent<T, E>::type;

// The same candidate over writable elements, for one over const elements: std::span's element type or a view's blocks.
template<class T>
struct writable
{};

template<class Element, std::size_t X>
struct writable<std::span<Element const, X>>
{
        using type = std::span<Element, X>;
};

template<class Blocks, std::size_t N>
struct writable<xstd::bit_span<Blocks const, N>>
{
        using type = xstd::bit_span<Blocks, N>;
};

template<class Blocks, std::size_t X, std::size_t N>
struct writable<xstd::bit_subspan<Blocks const, X, N>>
{
        using type = xstd::bit_subspan<Blocks, X, N>;
};

template<class T>
using writable_t = writable<T>::type;

// Whether u views what s does: as many positions, each the same one.
template<class U, class S>
[[nodiscard]] auto views_the_same(U const& u, S const& s)
        -> bool
{
        if (u.size() != s.size()) {
                return false;
        }
        for (auto const i : {0UZ, s.size() - 1UZ}) {
                if (i < s.size() and not same_position(u, i, s, i)) {
                        return false;
                }
        }
        return read(u) == read(s);
}

} // namespace

// [span.cons]/21: constexpr span(const span& other) noexcept = default;
BOOST_AUTO_TEST_CASE(CopyConstructor)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                static_assert(std::is_nothrow_copy_constructible_v<T>);
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s = a.view();
                        auto const u(s);
                        BOOST_CHECK(u.begin() == s.begin() and u.end() == s.end()); // [span.cons]/21
                        BOOST_CHECK(views_the_same(u, s));                          // [span.cons]/21
                }
        });
}

// [span.cons]/22,24-26: constexpr explicit(see below) span(const span<OtherElementType, OtherExtent>& s) noexcept;
BOOST_AUTO_TEST_CASE(ConvertingConstructor)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                // Const elements never become writable ones, which would write through what was handed over to be read.
                if constexpr (std::is_const_v<typename T::element_type>) {
                        static_assert(not std::is_constructible_v<writable_t<T>, T const&>); // [span.cons]/22
                }
                // A whole view has one extent, its owner's width: only std::span and a window have another.
                if constexpr (requires { typename at_extent_t<T, std::dynamic_extent>; }) {
                        using D = at_extent_t<T, std::dynamic_extent>;
                        static_assert(std::is_nothrow_constructible_v<D, T const&>);                             // [span.cons]/22
                        static_assert(std::is_convertible_v<T const&, D>);                                       // [span.cons]/26
                        static_assert(std::is_constructible_v<T, D const&>);                                     // [span.cons]/22
                        static_assert(std::is_convertible_v<D const&, T> == (T::extent == std::dynamic_extent)); // [span.cons]/26
                        if constexpr (T::extent != std::dynamic_extent) {
                                static_assert(not std::is_constructible_v<T, at_extent_t<T, T::extent + 1UZ> const&>); // [span.cons]/22
                        }
                        for (auto const [from, a] : inputs::views<T>()) {
                                auto const on_failure = context(from, a);
                                auto const s = a.view();
                                auto const d = D(s);
                                auto const t = T(d);
                                BOOST_CHECK(views_the_same(d, s)); // [span.cons]/24-25
                                BOOST_CHECK(views_the_same(t, s)); // [span.cons]/24-25
                        }
                }
        });
}

// [span.cons]/27: constexpr span& operator=(const span& other) noexcept = default;
BOOST_AUTO_TEST_CASE(CopyAssign)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                static_assert(std::is_nothrow_copy_assignable_v<T>);
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        auto u = a.view();
                        auto const s = b.view();
                        static_assert(std::same_as<decltype(u = s), T&>);
                        auto const& r = (u = s);
                        BOOST_CHECK(&r == &u);
                        BOOST_CHECK(u.begin() == s.begin() and u.end() == s.end()); // [span.cons]/27
                        BOOST_CHECK(views_the_same(u, s));                          // [span.cons]/27
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
