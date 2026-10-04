//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/input.hpp>      // context
#include <test/spec/sequence.hpp>   // bools
#include <test/spec/span.hpp>       // aliases, all, is_model, views
#include <test/spec/view.hpp>       // view_traits
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                    // array
#include <concepts>                 // convertible_to, default_initializable, equality_comparable, same_as
#include <cstddef>                  // ptrdiff_t, size_t
#include <iterator>                 // contiguous_iterator, iter_difference_t, iter_reference_t, iter_value_t, random_access_iterator
#include <span>                     // dynamic_extent
#include <type_traits>              // is_const_v

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Views)
BOOST_AUTO_TEST_SUITE(Contiguous)
BOOST_AUTO_TEST_SUITE(ViewsSpan)
BOOST_AUTO_TEST_SUITE(SpanIterators)

using test::spec::context;
using test::spec::span::aliases;
using test::spec::span::bools;
using test::spec::span::is_model;
namespace inputs = test::spec::span::inputs;

namespace {

// Every position walked from begin to end, and set where the elements are not const.
template<class S>
[[nodiscard]] constexpr auto walks(S const& s)
        -> bool
{
        auto n = 0UZ;
        for (auto it = s.begin(); it != s.end(); ++it) {
                if constexpr (not std::is_const_v<typename S::element_type>) {
                        *it = true;
                }
                ++n;
        }
        auto all = true;
        for (auto const x : s) {
                all = all and static_cast<bool>(x);
        }
        return n == s.size() and (std::is_const_v<typename S::element_type> or all);
}

// A view taken and walked in a constant expression, over an array of bools or over one of ours.
template<class T>
[[nodiscard]] constexpr auto iterates_as_a_constant()
        -> bool
{
        constexpr auto width = T::extent != std::dynamic_extent ? T::extent : 2UZ;
        if constexpr (is_model<T>) {
                auto owner = std::array<bool, width>{};
                return walks(T(owner.data(), width));
        } else {
                auto owner = test::spec::view_traits<T>::owner(bools(width, false));
                return walks(test::spec::view_traits<T>::view(owner, width));
        }
}

} // namespace

// [span.iterators]/1-2: using iterator = implementation-defined;
BOOST_AUTO_TEST_CASE(Iterator)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                using I = T::iterator;
                // Contiguous, which a proxy relaxes to random access: a bit has no address to lie at.
                static_assert(std::contiguous_iterator<I> or (std::random_access_iterator<I> and std::same_as<typename T::pointer, void>)); // [span.iterators]/1
                static_assert(std::same_as<std::iter_value_t<I>, typename T::value_type>);                                                  // [span.iterators]/1
                static_assert(std::same_as<std::iter_reference_t<I>, typename T::reference>);                                               // [span.iterators]/1
                static_assert(iterates_as_a_constant<T>());                                                                                 // [span.iterators]/1
                BOOST_CHECK(iterates_as_a_constant<T>());                                                                                   // [span.iterators]/1

                static_assert(std::default_initializable<I> and std::equality_comparable<I>);        // [span.iterators]/2
                static_assert(std::same_as<std::iter_difference_t<I>, typename T::difference_type>); // [span.iterators]/2
                if constexpr (requires { typename T::const_iterator; }) {
                        static_assert(std::convertible_to<I, typename T::const_iterator>); // [span.iterators]/2
                }
        });
}

// [span.iterators]/3: constexpr iterator begin() const noexcept;
BOOST_AUTO_TEST_CASE(Begin)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s          = a.view();
                        static_assert(noexcept(s.begin()) and std::same_as<decltype(s.begin()), typename T::iterator>);
                        BOOST_CHECK_EQUAL(s.begin() == s.end(), s.empty()); // [span.iterators]/3
                        if (not s.empty()) {
                                BOOST_CHECK(aliases(*s.begin(), s[0])); // [span.iterators]/3
                        }
                }
        });
}

// [span.iterators]/4: constexpr iterator end() const noexcept;
BOOST_AUTO_TEST_CASE(End)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s          = a.view();
                        static_assert(noexcept(s.end()) and std::same_as<decltype(s.end()), typename T::iterator>);
                        BOOST_CHECK_EQUAL(s.end() - s.begin(), static_cast<std::ptrdiff_t>(s.size())); // [span.iterators]/4
                        if (not s.empty()) {
                                BOOST_CHECK(aliases(*(s.end() - 1), s[s.size() - 1UZ])); // [span.iterators]/4
                        }
                }
        });
}

// [span.iterators]/5: constexpr reverse_iterator rbegin() const noexcept;
BOOST_AUTO_TEST_CASE(Rbegin)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s          = a.view();
                        static_assert(noexcept(s.rbegin()) and std::same_as<decltype(s.rbegin()), typename T::reverse_iterator>);
                        BOOST_CHECK(s.rbegin() == typename T::reverse_iterator(s.end())); // [span.iterators]/5
                }
        });
}

// [span.iterators]/6: constexpr reverse_iterator rend() const noexcept;
BOOST_AUTO_TEST_CASE(Rend)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s          = a.view();
                        static_assert(noexcept(s.rend()) and std::same_as<decltype(s.rend()), typename T::reverse_iterator>);
                        BOOST_CHECK(s.rend() == typename T::reverse_iterator(s.begin())); // [span.iterators]/6
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
