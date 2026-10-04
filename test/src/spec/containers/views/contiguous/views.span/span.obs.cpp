//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/input.hpp>      // context
#include <test/spec/span.hpp>       // all, views
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                 // same_as
#include <cstddef>                  // size_t
#include <iterator>                 // distance
#include <span>                     // dynamic_extent

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Views)
BOOST_AUTO_TEST_SUITE(Contiguous)
BOOST_AUTO_TEST_SUITE(ViewsSpan)
BOOST_AUTO_TEST_SUITE(SpanObs)

using test::spec::context;
namespace inputs = test::spec::span::inputs;

// [span.obs]/1: constexpr size_type size() const noexcept;
BOOST_AUTO_TEST_CASE(Size)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s          = a.view();
                        static_assert(noexcept(s.size()) and std::same_as<decltype(s.size()), typename T::size_type>);
                        BOOST_CHECK_EQUAL(s.size(), static_cast<std::size_t>(std::distance(s.begin(), s.end()))); // [span.obs]/1
                        BOOST_CHECK(T::extent == std::dynamic_extent or s.size() == T::extent);                   // [span.obs]/1
                }
        });
}

// [span.obs]/3: constexpr bool empty() const noexcept;
BOOST_AUTO_TEST_CASE(Empty)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s          = a.view();
                        static_assert(noexcept(s.empty()) and std::same_as<decltype(s.empty()), bool>);
                        BOOST_CHECK_EQUAL(s.empty(), s.size() == 0UZ); // [span.obs]/3
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
