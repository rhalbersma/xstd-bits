//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/input.hpp>      // context
#include <test/spec/rejection.hpp>  // has_assign_count, has_clear, has_erase_at, has_insert_at, has_pop_back, has_push_back, has_reserve, has_resize
#include <test/spec/span.hpp>       // all, is_model, margins_set, owned_bools, read, same_position, views
#include <xstd/bits/bit_vector.hpp> // bit_vector
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // convertible_to, same_as
#include <cstddef>                  // ptrdiff_t, size_t
#include <iterator>                 // bidirectional_iterator, indirectly_writable, iter_reference_t, iter_value_t, random_access_iterator, reverse_iterator
#include <ranges>                   // borrowed_range, contiguous_range, random_access_range, view
#include <span>                     // dynamic_extent
#include <type_traits>              // is_assignable_v, is_const_v, is_pointer_v, is_trivially_copyable_v, remove_const_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Views)
BOOST_AUTO_TEST_SUITE(Contiguous)
BOOST_AUTO_TEST_SUITE(ViewsSpan)
BOOST_AUTO_TEST_SUITE(SpanOverview)

using test::spec::context;
using test::spec::span::is_model;
using test::spec::span::margins_set;
using test::spec::span::owned_bools;
using test::spec::span::read;
using test::spec::span::same_position;
namespace inputs = test::spec::span::inputs;

namespace {

// A reference to the element, or a proxy for one that reads as a bool and writes only where the element is not const.
template<class T, class R>
concept reference_to_element = std::same_as<R, typename T::element_type&> or (std::convertible_to<R, bool> and std::is_assignable_v<R, bool> != std::is_const_v<typename T::element_type>);

// Storage another object owns, whose size a view has no member to change.
template<class X>
auto check_size_is_fixed()
        -> void
{
        static_assert(not test::spec::has_resize<X>);       // [span.overview]/1
        static_assert(not test::spec::has_reserve<X>);      // [span.overview]/1
        static_assert(not test::spec::has_clear<X>);        // [span.overview]/1
        static_assert(not test::spec::has_push_back<X>);    // [span.overview]/1
        static_assert(not test::spec::has_pop_back<X>);     // [span.overview]/1
        static_assert(not test::spec::has_insert_at<X>);    // [span.overview]/1
        static_assert(not test::spec::has_erase_at<X>);     // [span.overview]/1
        static_assert(not test::spec::has_assign_count<X>); // [span.overview]/1
}

// An owner that grows has every one of them, so a misspelled member cannot pass the checks above.
static_assert(test::spec::has_resize<xstd::bit_vector> and test::spec::has_reserve<xstd::bit_vector>);
static_assert(test::spec::has_clear<xstd::bit_vector> and test::spec::has_push_back<xstd::bit_vector>);
static_assert(test::spec::has_pop_back<xstd::bit_vector> and test::spec::has_insert_at<xstd::bit_vector>);
static_assert(test::spec::has_erase_at<xstd::bit_vector> and test::spec::has_assign_count<xstd::bit_vector>);

} // namespace

// [span.overview]/1: template<class ElementType, size_t Extent = dynamic_extent> class span;
BOOST_AUTO_TEST_CASE(Span)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                // Contiguous elements, which a proxy relaxes to random access: a bit has no address to lie at.
                static_assert(std::ranges::view<T> and std::ranges::borrowed_range<T>);                                                                  // [span.overview]/1
                static_assert(std::ranges::contiguous_range<T> or (std::ranges::random_access_range<T> and not std::is_pointer_v<typename T::pointer>)); // [span.overview]/1
                check_size_is_fixed<T>();
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        // Storage another object owns: the view reads what it holds, and sees a write through a copy.
                        auto const s = a.view();
                        BOOST_CHECK(read(s) == owned_bools(a)); // [span.overview]/1
                        for (auto const i : {0UZ, s.size() / 2UZ, s.size() - 1UZ}) {
                                if (i < s.size()) {
                                        BOOST_CHECK(same_position(s, i, a, i)); // [span.overview]/1
                                }
                        }
                        BOOST_CHECK(read(s) == owned_bools(a) and margins_set(a)); // [span.overview]/1
                }
        });
}

// [span.overview]: using element_type = ElementType; and eleven more
BOOST_AUTO_TEST_CASE(ConstantsAndTypes)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                using E = T::element_type;
                static_assert(std::same_as<std::remove_const_t<E>, bool>);
                static_assert(std::same_as<typename T::value_type, bool>);
                static_assert(std::same_as<typename T::size_type, std::size_t>);
                static_assert(std::same_as<typename T::difference_type, std::ptrdiff_t>);

                // A proxy has no element to point at, and no pointer to hand out.
                static_assert(std::same_as<typename T::pointer, E*> or std::same_as<typename T::pointer, void>);
                static_assert(std::same_as<typename T::const_pointer, E const*> or std::same_as<typename T::const_pointer, void>);
                static_assert(reference_to_element<T, typename T::reference>);
                static_assert(std::same_as<typename T::const_reference, E const&> or (std::convertible_to<typename T::const_reference, bool> and not std::is_assignable_v<typename T::const_reference, bool>));

                static_assert(std::random_access_iterator<typename T::iterator>);
                static_assert(std::same_as<std::iter_value_t<typename T::iterator>, typename T::value_type>);
                static_assert(std::same_as<std::iter_reference_t<typename T::iterator>, typename T::reference>);
                static_assert(std::same_as<typename T::reverse_iterator, std::reverse_iterator<typename T::iterator>>);

                // Read-only either way: std::const_iterator's adaptor, or an iterator over const bits.
                static_assert(is_model<T> or requires { typename T::const_iterator; typename T::const_reverse_iterator; });
                if constexpr (requires { typename T::const_iterator; typename T::const_reverse_iterator; }) {
                        static_assert(std::random_access_iterator<typename T::const_iterator>);
                        static_assert(std::same_as<std::iter_value_t<typename T::const_iterator>, typename T::value_type>);
                        static_assert(not std::indirectly_writable<typename T::const_iterator, bool>);
                        static_assert(std::random_access_iterator<typename T::const_reverse_iterator>);
                        static_assert(not std::indirectly_writable<typename T::const_reverse_iterator, bool>);
                }

                // A static extent is the size of every view of the type.
                static_assert(std::same_as<decltype(T::extent), std::size_t const>);
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        BOOST_CHECK(T::extent == std::dynamic_extent or a.size() == T::extent);
                }
        });
}

// [span.overview]: constexpr const_iterator cbegin() const noexcept { return begin(); } and three more
BOOST_AUTO_TEST_CASE(ConstantIterators)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                static_assert(is_model<T> or requires (T const s) { s.cbegin(); s.cend(); s.crbegin(); s.crend(); });
                if constexpr (requires (T const s) { s.cbegin(); s.cend(); s.crbegin(); s.crend(); }) {
                        for (auto const [from, a] : inputs::views<T>()) {
                                auto const on_failure = context(from, a);
                                auto const s = a.view();
                                static_assert(noexcept(s.cbegin()) and noexcept(s.cend()) and noexcept(s.crbegin()) and noexcept(s.crend()));
                                static_assert(std::same_as<decltype(s.cbegin()), typename T::const_iterator>);
                                static_assert(std::same_as<decltype(s.crbegin()), typename T::const_reverse_iterator>);
                                BOOST_CHECK(s.cend() - s.cbegin() == s.end() - s.begin());
                                BOOST_CHECK(s.crend() - s.crbegin() == s.rend() - s.rbegin());
                                BOOST_CHECK(read(s.cbegin(), s.cend()) == read(s));
                                BOOST_CHECK(read(s.crbegin(), s.crend()) == read(s.rbegin(), s.rend()));
                        }
                }
        });
}

// [span.overview]/3: span<ElementType, Extent> is a trivially copyable type
BOOST_AUTO_TEST_CASE(TriviallyCopyable)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                static_assert(std::is_trivially_copyable_v<T>); // [span.overview]/3
                BOOST_CHECK(std::is_trivially_copyable_v<T>);   // [span.overview]/3
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
