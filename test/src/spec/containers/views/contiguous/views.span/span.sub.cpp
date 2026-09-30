//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/input.hpp>      // context
#include <test/spec/sequence.hpp>   // bools
#include <test/spec/span.hpp>       // all, read, same_position, spans, views
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // same_as
#include <cstddef>                  // ptrdiff_t, size_t
#include <span>                     // dynamic_extent
#include <type_traits>              // remove_const_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Views)
BOOST_AUTO_TEST_SUITE(Contiguous)
BOOST_AUTO_TEST_SUITE(ViewsSpan)
BOOST_AUTO_TEST_SUITE(SpanSub)

using test::spec::context;
using test::spec::span::bools;
using test::spec::span::read;
using test::spec::span::same_position;
namespace inputs = test::spec::span::inputs;

namespace {

inline constexpr auto dyn = std::dynamic_extent;

// Whether w views the count positions of s from offset on: the same bools, and at either end the same position.
template<class W, class S>
[[nodiscard]] auto is_window_of(W const& w, S const& s, std::size_t offset, std::size_t count)
        -> bool
{
        auto const all = read(s);
        auto const first = all.begin() + static_cast<std::ptrdiff_t>(offset);
        if (w.size() != count or read(w) != bools(first, first + static_cast<std::ptrdiff_t>(count))) {
                return false;
        }
        return count == 0UZ or (same_position(w, 0UZ, s, offset) and same_position(w, count - 1UZ, s, offset + count - 1UZ));
}

template<class X>
using element_t = std::remove_const_t<X>::element_type;

// The subview a member hands back: its extent the one given, its elements those of the view it came from.
template<class R, class T>
inline constexpr bool subview_at = std::same_as<element_t<R>, element_t<T>>;

template<std::size_t... N>
auto for_each_value(auto fun)
        -> void
{
        (fun.template operator()<N>(), ...);
}

// Whether offset and count lie within size, a count of dynamic_extent taking every position from offset on.
[[nodiscard]] constexpr auto fits(std::size_t size, std::size_t offset, std::size_t count) noexcept
        -> bool
{
        return offset <= size and (count == dyn or count <= size - offset);
}

// A static subspan, where it fits the extent in the view's type and the view's size.
template<std::size_t Offset, std::size_t Count, class S>
auto check_static_subspan(S const& s)
        -> void
{
        if constexpr (S::extent == dyn or fits(S::extent, Offset, Count)) {
                if (fits(s.size(), Offset, Count)) {
                        auto const w = s.template subspan<Offset, Count>();
                        constexpr auto extent = Count != dyn ? Count : (S::extent != dyn ? S::extent - Offset : dyn);
                        static_assert(subview_at<decltype(w), S> and decltype(w)::extent == extent);       // [span.sub]/10
                        BOOST_CHECK(is_window_of(w, s, Offset, Count != dyn ? Count : s.size() - Offset)); // [span.sub]/9
                }
        }
}

// Every count at one offset, spelled out rather than nested in a second generic lambda, on which MSVC 2022 fails.
template<std::size_t Offset, class S>
auto check_static_subspans(S const& s)
        -> void
{
        check_static_subspan<Offset, dyn>(s);
        check_static_subspan<Offset, 0UZ>(s);
        check_static_subspan<Offset, 2UZ>(s);
}

} // namespace

// [span.sub]/3: template<size_t Count> constexpr span<element_type, Count> first() const;
BOOST_AUTO_TEST_CASE(StaticFirst)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s = a.view();
                        for_each_value<0UZ, 1UZ, 2UZ, 9UZ>([&]<std::size_t Count> -> void {
                                if constexpr (T::extent == dyn or Count <= T::extent) {
                                        if (Count <= s.size()) {
                                                auto const w = s.template first<Count>();
                                                static_assert(subview_at<decltype(w), T> and decltype(w)::extent == Count);
                                                BOOST_CHECK(is_window_of(w, s, 0UZ, Count)); // [span.sub]/3
                                        }
                                }
                        });
                }
        });
}

// [span.sub]/6: template<size_t Count> constexpr span<element_type, Count> last() const;
BOOST_AUTO_TEST_CASE(StaticLast)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s = a.view();
                        for_each_value<0UZ, 1UZ, 2UZ, 9UZ>([&]<std::size_t Count> -> void {
                                if constexpr (T::extent == dyn or Count <= T::extent) {
                                        if (Count <= s.size()) {
                                                auto const w = s.template last<Count>();
                                                static_assert(subview_at<decltype(w), T> and decltype(w)::extent == Count);
                                                BOOST_CHECK(is_window_of(w, s, s.size() - Count, Count)); // [span.sub]/6
                                        }
                                }
                        });
                }
        });
}

// [span.sub]/9-10: template<size_t Offset, size_t Count = dynamic_extent> constexpr span<...> subspan() const;
BOOST_AUTO_TEST_CASE(StaticSubspan)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::views<T>()) {
                        auto const on_failure = context(from, a);
                        auto const s = a.view();
                        check_static_subspans<0UZ>(s);
                        check_static_subspans<1UZ>(s);
                        check_static_subspans<3UZ>(s);
                }
        });
}

// [span.sub]/12: constexpr span<element_type, dynamic_extent> first(size_type count) const;
BOOST_AUTO_TEST_CASE(First)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a, offset, count] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, offset, count);
                        auto const s = a.view();
                        if (count <= s.size()) {
                                auto const w = s.first(count);
                                static_assert(subview_at<decltype(w), T> and decltype(w)::extent == dyn);
                                BOOST_CHECK(is_window_of(w, s, 0UZ, count)); // [span.sub]/12
                        }
                }
        });
}

// [span.sub]/14: constexpr span<element_type, dynamic_extent> last(size_type count) const;
BOOST_AUTO_TEST_CASE(Last)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a, offset, count] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, offset, count);
                        auto const s = a.view();
                        if (count <= s.size()) {
                                auto const w = s.last(count);
                                static_assert(subview_at<decltype(w), T> and decltype(w)::extent == dyn);
                                BOOST_CHECK(is_window_of(w, s, s.size() - count, count)); // [span.sub]/14
                        }
                }
        });
}

// [span.sub]/16: constexpr span<element_type, dynamic_extent> subspan(size_type offset, size_type count = ...) const;
BOOST_AUTO_TEST_CASE(Subspan)
{
        test::for_each_type<test::spec::span::all>([]<class T> -> void {
                for (auto const [from, a, offset, count] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, offset, count);
                        auto const s = a.view();
                        if (offset <= s.size()) {
                                auto const rest = s.subspan(offset);
                                static_assert(subview_at<decltype(rest), T> and decltype(rest)::extent == dyn);
                                BOOST_CHECK(is_window_of(rest, s, offset, s.size() - offset));                   // [span.sub]/16
                                BOOST_CHECK(is_window_of(s.subspan(offset, dyn), s, offset, s.size() - offset)); // [span.sub]/16
                                if (count <= s.size() - offset) {
                                        BOOST_CHECK(is_window_of(s.subspan(offset, count), s, offset, count)); // [span.sub]/16
                                }
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
