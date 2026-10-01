//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/input.hpp>      // context
#include <test/spec/span.hpp>       // aliases, all, indexed, is_model, margins_set, owned_bools, same_position, views
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_NO_THROW, BOOST_CHECK_THROW
#include <concepts>                 // same_as
#include <cstddef>                  // ptrdiff_t, size_t
#include <stdexcept>                // out_of_range

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Views)
BOOST_AUTO_TEST_SUITE(Contiguous)
BOOST_AUTO_TEST_SUITE(ViewsSpan)
BOOST_AUTO_TEST_SUITE(SpanElem)

using test::spec::context;
using test::spec::span::aliases;
using test::spec::span::is_model;
using test::spec::span::margins_set;
using test::spec::span::owned_bools;
using test::spec::span::same_position;
namespace inputs = test::spec::span::inputs;

namespace {

// The element at idx, reached from the view, from its iterator and from the owner, and nothing around it disturbed.
template<class S, class A>
auto check_subscript(S const& s, A const& a, std::size_t i)
        -> void
{
        static_assert(std::same_as<decltype(s[i]), typename S::reference>);
        BOOST_CHECK(aliases(s[i], *(s.begin() + static_cast<std::ptrdiff_t>(i))));    // [span.elem]/2
        BOOST_CHECK(same_position(s, i, a, i));                                       // [span.elem]/2
        BOOST_CHECK(static_cast<bool>(s[i]) == owned_bools(a)[i] and margins_set(a)); // [span.elem]/2
        BOOST_CHECK_NO_THROW(static_cast<void>(s[i]));                                // [span.elem]/3
}

// The element at idx, and out_of_range at the size and past it.
template<class S>
auto check_at(S const& s, std::size_t i)
        -> void
{
        static_assert(std::same_as<decltype(s.at(i)), typename S::reference>);
        BOOST_CHECK(aliases(s.at(i), s[i]));                                               // [span.elem]/4
        BOOST_CHECK_THROW(static_cast<void>(s.at(s.size())), std::out_of_range);           // [span.elem]/5
        BOOST_CHECK_THROW(static_cast<void>(s.at(s.size() + i + 1UZ)), std::out_of_range); // [span.elem]/5
}

template<class S>
auto check_front(S const& s)
        -> void
{
        static_assert(std::same_as<decltype(s.front()), typename S::reference>);
        if (not s.empty()) {
                BOOST_CHECK(aliases(s.front(), *s.begin()));        // [span.elem]/7
                BOOST_CHECK_NO_THROW(static_cast<void>(s.front())); // [span.elem]/8
        }
}

template<class S>
auto check_back(S const& s)
        -> void
{
        static_assert(std::same_as<decltype(s.back()), typename S::reference>);
        if (not s.empty()) {
                BOOST_CHECK(aliases(s.back(), *(s.end() - 1)));    // [span.elem]/10
                BOOST_CHECK_NO_THROW(static_cast<void>(s.back())); // [span.elem]/11
        }
}

} // namespace

// [span.elem]/2-3: constexpr reference operator[](size_type idx) const;
BOOST_AUTO_TEST_CASE(Subscript)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a, i] : inputs::indexed<T>()) {
                        auto const on_failure = context(from, a, i);
                        check_subscript(a.view(), a, i);
                }
        });
}

// [span.elem]/4-5: constexpr reference at(size_type idx) const;
BOOST_AUTO_TEST_CASE(At)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                static_assert(is_model<T> or requires (T const s) { s.at(0UZ); });
                if constexpr (requires (T const s) { s.at(0UZ); }) {
                        for (auto const [from, a, i] : inputs::indexed<T>()) {
                                auto const on_failure = context(from, a, i);
                                check_at(a.view(), i);
                        }
                }
        });
}

// [span.elem]/7-8: constexpr reference front() const;
BOOST_AUTO_TEST_CASE(Front)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        check_front(a.view());
                }
        });
}

// [span.elem]/10-11: constexpr reference back() const;
BOOST_AUTO_TEST_CASE(Back)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        check_back(a.view());
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
