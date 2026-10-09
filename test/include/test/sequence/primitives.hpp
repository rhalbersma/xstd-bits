//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_PRIMITIVES_HPP
#define TEST_SEQUENCE_PRIMITIVES_HPP

#include <test/dynamic.hpp>          // dynamic
#include <test/inplace_vector.hpp>   // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/sanitizer.hpp>        // has_address_sanitizer
#include <test/sequence/factory.hpp> // model_of, static_capacity
#include <boost/test/unit_test.hpp>  // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_GE, BOOST_CHECK_NE, BOOST_CHECK_NO_THROW, BOOST_CHECK_THROW
#include <algorithm>                 // ranges::all_of, ranges::count, ranges::equal, ranges::min
#include <concepts>                  // derived_from, same_as
#include <cstddef>                   // ptrdiff_t, size_t
#include <format>                    // format, formattable
#include <initializer_list>          // initializer_list
#include <iterator>                  // distance, forward_iterator, input_iterator_tag, iter_value_t, iterator_traits, prev, random_access_iterator_tag
#include <memory>                    // addressof
#include <new>                       // bad_alloc
#include <ranges>                    // begin, distance, end, filter, forward_range, from_range, from_range_t, iota, subrange, take, transform
#include <stdexcept>                 // length_error, out_of_range
#include <string>                    // string
#include <string_view>               // string_view
#include <type_traits>               // type_identity
#include <utility>                   // declval
#include <vector>                    // erase_if, vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

namespace test::sequence {

// A primitive holding a BOOST_CHECK_THROW NOLINTs bugprone-exception-escape: the check reads the callee, not the guard.

// The iterator p positions in, which is how every modifier is handed its position.
template<class X>
[[nodiscard]] auto nth(X const& a, std::size_t p)
        -> X::const_iterator
{
        return a.cbegin() + static_cast<X::difference_type>(p);
}

// The position an iterator stands at, which is what a modifier's returned iterator is checked by.
template<class X>
[[nodiscard]] auto offset(X& a, typename X::iterator it)
        -> std::size_t
{
        return static_cast<std::size_t>(it - a.begin());
}

// A copy of b as a prvalue, so the overload taking an rvalue is the one called.
[[nodiscard]] constexpr auto temporary(bool b) noexcept
        -> bool
{
        return b;
}

// The model with in placed before position p.
[[nodiscard]] inline auto spliced(std::vector<bool> m, std::size_t p, std::vector<bool> const& in)
        -> std::vector<bool>
{
        m.insert(m.begin() + static_cast<std::ptrdiff_t>(p), in.begin(), in.end());
        return m;
}

// The model without positions p up to q.
[[nodiscard]] inline auto cut(std::vector<bool> m, std::size_t p, std::size_t q)
        -> std::vector<bool>
{
        m.erase(m.begin() + static_cast<std::ptrdiff_t>(p), m.begin() + static_cast<std::ptrdiff_t>(q));
        return m;
}

// Room for k more, which a capacity in the type can refuse and a heap practically never does.
template<class X>
[[nodiscard]] auto has_room(X const& a, std::size_t k)
        -> bool
{
        return k <= a.max_size() - a.size();
}

// k bools alternating from true, so what is inserted differs from its neighbours somewhere.
[[nodiscard]] inline auto alternating(std::size_t k)
        -> std::vector<bool>
{
        auto v = std::vector<bool>(k);
        for (auto const i : std::views::iota(0UZ, k)) {
                v[i] = (i % 2UZ == 0UZ);
        }
        return v;
}

// The bools of m as a random access range that counts in reads how often an element is read.
[[nodiscard]] inline auto counted(std::vector<bool> const& m, std::size_t& reads)
{
        return std::views::iota(0UZ, m.size()) | std::views::transform([&m, &reads](std::size_t i) -> bool {
                       ++reads;
                       return m[i];
               });
}

// k alternating bools with no size to reserve by, which is the other path a range can take in.
[[nodiscard]] inline auto unsized_alternating(std::size_t k)
{
        return std::views::iota(0UZ, k) | std::views::filter([](std::size_t) -> bool { return true; }) | std::views::transform([](std::size_t i) -> bool { return i % 2UZ == 0UZ; });
}

// A sequence of bool: the element type [container.reqmts] asks of the rest is bool, and its iterators random access.
template<class X>
constexpr auto nested_types()
        -> void
{
        using I  = X::iterator;
        using CI = X::const_iterator;

        static_assert(std::same_as<typename X::value_type, bool>); // [container.reqmts]/2
        static_assert(std::derived_from<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag>);
        static_assert(std::derived_from<typename std::iterator_traits<CI>::iterator_category, std::random_access_iterator_tag>);
        static_assert(std::same_as<std::iter_value_t<I>, bool>);
        static_assert(std::same_as<std::iter_value_t<CI>, bool>);
}

// The subscript is *(a.begin() + n) at every position, and writing through it changes that position alone.
struct mem_subscript
{
        template<class X>
        auto operator()(X const& a) const
        {
                static_assert(std::same_as<decltype(a[0UZ]), typename X::const_reference>); // [sequence.reqmts]/121
                auto agrees = true;
                for (auto const n : std::views::iota(0UZ, a.size())) {
                        agrees = agrees and static_cast<bool>(a[n]) == static_cast<bool>(*(a.begin() + static_cast<X::difference_type>(n))); // [sequence.reqmts]/123
                }
                BOOST_CHECK(agrees);
        }

        template<class X>
        auto operator()(X const& a, std::size_t n) const
        {
                auto b = a;
                static_assert(std::same_as<decltype(b[n]), typename X::reference>); // [sequence.reqmts]/121
                b[n]   = not static_cast<bool>(a[n]);
                auto m = model_of(a);
                m[n]   = not m[n];
                BOOST_CHECK(model_of(b) == m);
        }
};

// at() is the subscript inside the width, and out_of_range past it.
struct mem_at
{
        template<class X>
        auto operator()(X const& a, std::size_t n) const // NOLINT(bugprone-exception-escape)
        {
                static_assert(std::same_as<decltype(a.at(n)), typename X::const_reference>); // [sequence.reqmts]/125
                if (n < a.size()) {
                        BOOST_CHECK_EQUAL(static_cast<bool>(a.at(n)), static_cast<bool>(a[n])); // [sequence.reqmts]/126
                        auto b = a;
                        static_assert(std::same_as<decltype(b.at(n)), typename X::reference>);
                        b.at(n) = not static_cast<bool>(a[n]);
                        BOOST_CHECK_NE(static_cast<bool>(b[n]), static_cast<bool>(a[n]));
                } else {
                        BOOST_CHECK_THROW(static_cast<void>(a.at(n)), std::out_of_range); // [sequence.reqmts]/127
                }
        }
};

struct mem_front
{
        template<class X>
        auto operator()(X const& a) const
        {
                if (not a.empty()) {
                        static_assert(std::same_as<decltype(a.front()), typename X::const_reference>);  // [sequence.reqmts]/71
                        BOOST_CHECK_EQUAL(static_cast<bool>(a.front()), static_cast<bool>(*a.begin())); // [sequence.reqmts]/73
                        auto b = a;
                        static_assert(std::same_as<decltype(b.front()), typename X::reference>);
                        b.front() = not static_cast<bool>(a.front());
                        BOOST_CHECK_NE(static_cast<bool>(*b.begin()), static_cast<bool>(*a.begin()));
                }
        }
};

struct mem_back
{
        template<class X>
        auto operator()(X const& a) const
        {
                if (not a.empty()) {
                        static_assert(std::same_as<decltype(a.back()), typename X::const_reference>);           // [sequence.reqmts]/75
                        BOOST_CHECK_EQUAL(static_cast<bool>(a.back()), static_cast<bool>(*std::prev(a.end()))); // [sequence.reqmts]/77
                        auto b = a;
                        static_assert(std::same_as<decltype(b.back()), typename X::reference>);
                        b.back() = not static_cast<bool>(a.back());
                        BOOST_CHECK_NE(static_cast<bool>(*std::prev(b.end())), static_cast<bool>(*std::prev(a.end())));
                }
        }
};

// Iterators walked, written through and measured in a constant expression: two positions where the type can grow.
template<class X>
[[nodiscard]] constexpr auto iterates_as_a_constant()
        -> bool
{
        auto a = X();
        if constexpr (test::dynamic<X>) {
                a.resize(std::ranges::min(a.max_size(), 2UZ));
        }
        auto n = 0Z;
        for (auto it = a.begin(); it != a.end(); ++it) {
                *it = true;
                ++n;
        }
        return n == a.cend() - a.cbegin() and std::ranges::all_of(a, [](bool b) -> bool { return b; });
}

// [sequence.reqmts]'s constructors; one a capacity cannot hold is the capacity's clause to answer.
template<class X>
struct constructor
{
        auto operator()(std::size_t n, bool t) const
        {
                if (n <= X().max_size()) {
                        auto const u = X(n, t);
                        BOOST_CHECK(model_of(u) == std::vector<bool>(n, t));                               // [sequence.reqmts]/6
                        BOOST_CHECK_EQUAL(static_cast<std::size_t>(std::distance(u.begin(), u.end())), n); // [sequence.reqmts]/7
                }
        }

        template<std::forward_iterator I>
        auto operator()(I i, I j) const
        {
                auto const m = std::vector<bool>(i, j);
                if (m.size() <= X().max_size()) {
                        auto const u = X(i, j);
                        BOOST_CHECK(model_of(u) == m);                                                                          // [sequence.reqmts]/9
                        BOOST_CHECK_EQUAL(std::distance(u.begin(), u.end()), static_cast<std::ptrdiff_t>(std::distance(i, j))); // [sequence.reqmts]/10
                }
        }

        // The standard libraries without P1206R7 have no from_range constructor to check.
        template<std::ranges::forward_range R>
        auto operator()(std::from_range_t, R rg) const
        {
                if constexpr (requires { X(std::from_range, rg); }) {
                        auto const m = std::vector<bool>(std::ranges::begin(rg), std::ranges::end(rg));
                        if (m.size() <= X().max_size()) {
                                auto const u = X(std::from_range, rg);
                                BOOST_CHECK(model_of(u) == m);                                                                                // [sequence.reqmts]/12
                                BOOST_CHECK_EQUAL(std::distance(u.begin(), u.end()), static_cast<std::ptrdiff_t>(std::ranges::distance(rg))); // [sequence.reqmts]/14
                        }
                }
        }

        auto operator()(std::initializer_list<bool> il) const
        {
                if (il.size() <= X().max_size()) {
                        BOOST_CHECK(X(il) == X(il.begin(), il.end())); // [sequence.reqmts]/15
                }
        }
};

struct op_assign
{
        template<class X>
        auto operator()(X const& a, std::initializer_list<bool> il) const
        {
                if (il.size() <= a.max_size()) {
                        auto r = a;
                        static_assert(std::same_as<decltype(r = il), X&>); // [sequence.reqmts]/16
                        auto const& s = (r = il);
                        BOOST_CHECK(r == X(il));                             // [sequence.reqmts]/18
                        BOOST_CHECK(std::addressof(s) == std::addressof(r)); // [sequence.reqmts]/19
                }
        }
};

struct mem_assign
{
        template<class X>
        auto operator()(X const& a, std::size_t n, bool t) const
        {
                if (n <= a.max_size()) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.assign(n, t)), void>); // [sequence.reqmts]/66
                        b.assign(n, t);
                        BOOST_CHECK(model_of(b) == std::vector<bool>(n, t)); // [sequence.reqmts]/68
                }
        }

        template<class X, std::forward_iterator I>
        auto operator()(X const& a, I i, I j) const
        {
                auto const m = std::vector<bool>(i, j);
                if (m.size() <= a.max_size()) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.assign(i, j)), void>); // [sequence.reqmts]/57
                        b.assign(i, j);
                        BOOST_CHECK(model_of(b) == m); // [sequence.reqmts]/59
                }
        }

        template<class X>
        auto operator()(X const& a, std::initializer_list<bool> il) const
        {
                if (il.size() <= a.max_size()) {
                        auto b = a;
                        b.assign(il);
                        BOOST_CHECK(b == X(il)); // [sequence.reqmts]/65
                }
        }
};

// The standard libraries without P1206R7 have no assign_range to check.
struct mem_assign_range
{
        template<class X, std::ranges::forward_range R>
        auto operator()(X const& a, R rg) const
        {
                if constexpr (requires (X& b) { b.assign_range(rg); }) {
                        auto const m = std::vector<bool>(std::ranges::begin(rg), std::ranges::end(rg));
                        if (m.size() <= a.max_size()) {
                                auto b = a;
                                static_assert(std::same_as<decltype(b.assign_range(rg)), void>); // [sequence.reqmts]/60
                                b.assign_range(rg);
                                BOOST_CHECK(model_of(b) == m); // [sequence.reqmts]/63
                        }
                }
        }
};

struct mem_emplace
{
        template<class X>
        auto operator()(X const& a, std::size_t p, bool t) const
        {
                if (has_room(a, 1UZ)) {
                        auto b       = a;
                        auto const r = b.emplace(nth(b, p), t);
                        static_assert(std::same_as<decltype(b.emplace(nth(b, p), t)), typename X::iterator>); // [sequence.reqmts]/20
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                                   // [sequence.reqmts]/23
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), p, {t}));                             // [sequence.reqmts]/22
                }
        }

        // No argument at all value-initializes, which for a bool is false.
        template<class X>
        auto operator()(X const& a, std::size_t p) const
        {
                if (has_room(a, 1UZ)) {
                        auto b       = a;
                        auto const r = b.emplace(nth(b, p));
                        BOOST_CHECK_EQUAL(offset(b, r), p);
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), p, {false}));
                }
        }
};

struct mem_insert
{
        template<class X>
        auto operator()(X const& a, std::size_t p, bool t) const
        {
                if (has_room(a, 1UZ)) {
                        auto b       = a;
                        auto const r = b.insert(nth(b, p), t);
                        static_assert(std::same_as<decltype(b.insert(nth(b, p), t)), typename X::iterator>); // [sequence.reqmts]/24
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                                  // [sequence.reqmts]/27
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), p, {t}));                            // [sequence.reqmts]/26

                        auto c       = a;
                        auto const s = c.insert(nth(c, p), temporary(t));
                        static_assert(std::same_as<decltype(c.insert(nth(c, p), temporary(t))), typename X::iterator>); // [sequence.reqmts]/28
                        BOOST_CHECK_EQUAL(offset(c, s), p);                                                             // [sequence.reqmts]/31
                        BOOST_CHECK(model_of(c) == spliced(model_of(a), p, {t}));                                       // [sequence.reqmts]/30
                }
        }

        template<class X>
        auto operator()(X const& a, std::size_t p, std::size_t n, bool t) const
        {
                if (has_room(a, n)) {
                        auto b       = a;
                        auto const r = b.insert(nth(b, p), n, t);
                        static_assert(std::same_as<decltype(b.insert(nth(b, p), n, t)), typename X::iterator>); // [sequence.reqmts]/32
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                                     // [sequence.reqmts]/35
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), p, std::vector<bool>(n, t)));           // [sequence.reqmts]/34
                }
        }

        template<class X, std::forward_iterator I>
        auto operator()(X const& a, std::size_t p, I i, I j) const
        {
                auto const in = std::vector<bool>(i, j);
                if (has_room(a, in.size())) {
                        auto b       = a;
                        auto const r = b.insert(nth(b, p), i, j);
                        static_assert(std::same_as<decltype(b.insert(nth(b, p), i, j)), typename X::iterator>); // [sequence.reqmts]/36
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                                     // [sequence.reqmts]/39
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), p, in));                                // [sequence.reqmts]/38
                }
        }

        template<class X>
        auto operator()(X const& a, std::size_t p, std::initializer_list<bool> il) const
        {
                if (has_room(a, il.size())) {
                        auto b       = a;
                        auto const r = b.insert(nth(b, p), il);
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                         // [sequence.reqmts]/44
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), p, std::vector<bool>(il))); // [sequence.reqmts]/44
                }
        }
};

// The standard libraries without P1206R7 have no insert_range to check.
struct mem_insert_range
{
        template<class X, std::ranges::forward_range R>
        auto operator()(X const& a, std::size_t p, R rg) const
        {
                if constexpr (requires (X& b) { b.insert_range(nth(b, p), rg); }) {
                        auto const in = std::vector<bool>(std::ranges::begin(rg), std::ranges::end(rg));
                        if (has_room(a, in.size())) {
                                auto b       = a;
                                auto const r = b.insert_range(nth(b, p), rg);
                                static_assert(std::same_as<decltype(b.insert_range(nth(b, p), rg)), typename X::iterator>); // [sequence.reqmts]/40
                                BOOST_CHECK_EQUAL(offset(b, r), p);                                                         // [sequence.reqmts]/43
                                BOOST_CHECK(model_of(b) == spliced(model_of(a), p, in));                                    // [sequence.reqmts]/42
                        }
                }
        }
};

struct mem_erase
{
        template<class X>
        auto operator()(X const& a, std::size_t p) const
        {
                if (p < a.size()) {
                        auto b       = a;
                        auto const r = b.erase(nth(b, p));
                        static_assert(std::same_as<decltype(b.erase(nth(b, p))), typename X::iterator>); // [sequence.reqmts]/45
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                              // [sequence.reqmts]/48
                        BOOST_CHECK(model_of(b) == cut(model_of(a), p, p + 1UZ));                        // [sequence.reqmts]/47
                }
        }

        template<class X>
        auto operator()(X const& a, std::size_t p, std::size_t q) const
        {
                if (p <= q and q <= a.size()) {
                        auto b       = a;
                        auto const r = b.erase(nth(b, p), nth(b, q));
                        static_assert(std::same_as<decltype(b.erase(nth(b, p), nth(b, q))), typename X::iterator>); // [sequence.reqmts]/49
                        BOOST_CHECK_EQUAL(offset(b, r), p);                                                         // [sequence.reqmts]/52
                        BOOST_CHECK(model_of(b) == cut(model_of(a), p, q));                                         // [sequence.reqmts]/51
                }
        }
};

struct mem_clear
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto b = a;
                static_assert(std::same_as<decltype(b.clear()), void>); // [sequence.reqmts]/53
                b.clear();
                BOOST_CHECK(b.begin() == b.end()); // [sequence.reqmts]/54
                BOOST_CHECK(b.empty());            // [sequence.reqmts]/55
        }
};

struct mem_emplace_back
{
        template<class X>
        auto operator()(X const& a, bool t) const
        {
                if (has_room(a, 1UZ)) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.emplace_back(t)), typename X::reference>); // [sequence.reqmts]/84
                        BOOST_CHECK_EQUAL(static_cast<bool>(b.emplace_back(t)), t);                      // [sequence.reqmts]/87
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), a.size(), {t}));                 // [sequence.reqmts]/86
                }
        }

        // No argument at all value-initializes, which for a bool is false.
        template<class X>
        auto operator()(X const& a) const
        {
                if (has_room(a, 1UZ)) {
                        auto b = a;
                        BOOST_CHECK(not static_cast<bool>(b.emplace_back()));
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), a.size(), {false}));
                }
        }
};

struct mem_push_back
{
        template<class X>
        auto operator()(X const& a, bool t) const
        {
                // [inplace.vector.modifiers] returns the reference where [sequence.reqmts] returns nothing.
                static_assert(static_capacity<X> or std::same_as<decltype(std::declval<X&>().push_back(t)), void>);            // [sequence.reqmts]/101
                static_assert(static_capacity<X> or std::same_as<decltype(std::declval<X&>().push_back(temporary(t))), void>); // [sequence.reqmts]/105
                if (has_room(a, 1UZ)) {
                        auto const m = spliced(model_of(a), a.size(), {t});
                        auto b       = a;
                        b.push_back(t);
                        BOOST_CHECK(model_of(b) == m); // [sequence.reqmts]/103
                        auto c = a;
                        c.push_back(temporary(t));
                        BOOST_CHECK(model_of(c) == m); // [sequence.reqmts]/107
                }
        }
};

struct mem_append_range
{
        template<class X, std::ranges::forward_range R>
        auto operator()(X const& a, R rg) const
        {
                if constexpr (requires (X& b) { b.append_range(rg); }) {
                        auto const in = std::vector<bool>(std::ranges::begin(rg), std::ranges::end(rg));
                        if (has_room(a, in.size())) {
                                auto b = a;
                                static_assert(std::same_as<decltype(b.append_range(rg)), void>); // [sequence.reqmts]/109
                                b.append_range(rg);
                                BOOST_CHECK(model_of(b) == spliced(model_of(a), a.size(), in)); // [sequence.reqmts]/111
                        }
                }
        }
};

// A range is read one element at a time, each exactly once, however the container takes its size.
struct reads_once
{
        template<class X>
        auto operator()(std::type_identity<X>, std::vector<bool> const& m) const
        {
                if (m.size() <= X().max_size()) {
                        auto reads    = 0UZ;
                        auto const rg = counted(m, reads);
                        BOOST_CHECK(model_of(X(rg.begin(), rg.end())) == m);
                        BOOST_CHECK_EQUAL(reads, m.size()); // [sequence.reqmts]/9
                        if constexpr (requires { X(std::from_range, rg); }) {
                                reads = 0UZ;
                                BOOST_CHECK(model_of(X(std::from_range, rg)) == m);
                                BOOST_CHECK_EQUAL(reads, m.size()); // [sequence.reqmts]/12
                        }
                }
        }

        template<class X>
        auto operator()(X const& a, std::size_t p, std::vector<bool> const& in) const
        {
                if (has_room(a, in.size())) {
                        auto reads    = 0UZ;
                        auto const rg = counted(in, reads);
                        auto b        = a;
                        b.insert(nth(b, p), rg.begin(), rg.end());
                        BOOST_CHECK_EQUAL(reads, in.size()); // [sequence.reqmts]/38
                        if constexpr (requires { b.insert_range(nth(b, p), rg); }) {
                                reads  = 0UZ;
                                auto c = a;
                                c.insert_range(nth(c, p), rg);
                                BOOST_CHECK_EQUAL(reads, in.size()); // [sequence.reqmts]/42
                                reads  = 0UZ;
                                auto d = a;
                                d.append_range(rg);
                                BOOST_CHECK_EQUAL(reads, in.size()); // [sequence.reqmts]/111
                                BOOST_CHECK(b == c and model_of(d) == spliced(model_of(a), a.size(), in));
                        }
                }
        }
};

struct mem_pop_back
{
        template<class X>
        auto operator()(X const& a) const
        {
                if (not a.empty()) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.pop_back()), void>); // [sequence.reqmts]/117
                        b.pop_back();
                        BOOST_CHECK(model_of(b) == cut(model_of(a), a.size() - 1UZ, a.size())); // [sequence.reqmts]/119
                }
        }
};

// [vector.capacity] and [inplace.vector.capacity] alike: a longer width takes t, or false, where there is room.
struct mem_resize
{
        template<class X>
        auto operator()(X const& a, std::size_t n) const
        {
                if (n <= a.max_size()) {
                        auto b = a;
                        b.resize(n);
                        auto m = model_of(a);
                        m.resize(n);
                        BOOST_CHECK(model_of(b) == m); // [vector.capacity]/15 [inplace.vector.capacity]/3
                }
        }

        template<class X>
        auto operator()(X const& a, std::size_t n, bool t) const
        {
                if (n <= a.max_size()) {
                        auto b = a;
                        b.resize(n, t);
                        auto m = model_of(a);
                        m.resize(n, t);
                        BOOST_CHECK(model_of(b) == m); // [vector.capacity]/18 [inplace.vector.capacity]/6
                }
        }
};

struct mem_capacity
{
        template<class X>
        auto operator()(X const& a) const
        {
                static_assert(std::same_as<decltype(a.capacity()), typename X::size_type>);
                BOOST_CHECK_GE(a.capacity(), a.size()); // [vector.capacity]/1
        }
};

template<class X>
inline constexpr auto is_std_vector_bool_v = false;

template<class Allocator>
inline constexpr auto is_std_vector_bool_v<std::vector<bool, Allocator>> = true;

#ifdef _MSVC_STL_VERSION

// MSVC's vector<bool>::reserve allocates the blocks for n unchecked, where [vector.capacity]/5 throws length_error.
inline constexpr auto std_vector_bool_reserve_checks_max_size = false;

#else

inline constexpr auto std_vector_bool_reserve_checks_max_size = true;

#endif

template<class X>
concept reserve_checks_max_size = std_vector_bool_reserve_checks_max_size or not is_std_vector_bool_v<X>;

// P3612R1's two swaps between a reference and a bool&, which MSVC 2022's STL lacks.
template<class X>
concept reference_swaps_with_bool = requires (X c, X::size_type n, bool& b) {
        swap(c[n], b);
        swap(b, c[n]);
};

// Storage for n at least, and the value unchanged; past max_size() there is none to be had.
struct mem_reserve
{
        template<class X>
        auto operator()(X const& a, std::size_t n) const // NOLINT(bugprone-exception-escape)
        {
                auto b = a;
                if (n <= a.max_size()) {
                        b.reserve(n);
                        BOOST_CHECK_GE(b.capacity(), n); // [vector.capacity]/4
                } else {
                        past_max_size(b, n);
                }
                BOOST_CHECK(b == a); // [vector.capacity]/7
        }

private:
        template<class X>
        static auto past_max_size(X& b, std::size_t n)
                -> void
        {
                if constexpr (reserve_checks_max_size<X>) {
                        throws_length_error(b, n);
                } else if constexpr (not has_address_sanitizer) {
                        // No allocator serves that many blocks, and AddressSanitizer aborts on the request.
                        throws_bad_alloc(b, n);
                }
        }

        static auto throws_length_error(auto& b, std::size_t n)
                -> void
        {
                BOOST_CHECK_THROW(b.reserve(n), std::length_error); // [vector.capacity]/5
        }

        static auto throws_bad_alloc(auto& b, std::size_t n)
                -> void
        {
                BOOST_CHECK_THROW(b.reserve(n), std::bad_alloc);
        }
};

// A non-binding request, so all that holds after it is the value and room for it.
struct mem_shrink_to_fit
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto b = a;
                b.reserve(a.size() + 64UZ);
                b.shrink_to_fit();
                BOOST_CHECK_GE(b.capacity(), b.size()); // [vector.capacity]/9
                BOOST_CHECK(b == a);

                // Where the request reallocates nothing, an iterator taken before it still stands where it stood.
                auto c = a;
                c.shrink_to_fit();
                auto const capacity = c.capacity();
                auto const first    = c.begin();
                c.shrink_to_fit();
                if (c.capacity() == capacity) {
                        BOOST_CHECK(first == c.begin()); // [vector.capacity]/11
                }
        }
};

// The contents and the capacities change places.
struct mem_swap_capacity
{
        // Blocks held inline stay where they are, so a small vector exchanges only the contents.
        template<class X>
        auto operator()(X const& a, X const& b, bool inline_blocks) const
        {
                auto x = a;
                auto y = b;
                x.reserve(a.size() + 64UZ);
                auto const cx = x.capacity();
                auto const cy = y.capacity();
                x.swap(y);
                BOOST_CHECK(x == b and y == a); // [vector.capacity]/12
                if (not inline_blocks) {
                        BOOST_CHECK_EQUAL(x.capacity(), cy); // [vector.capacity]/12
                        BOOST_CHECK_EQUAL(y.capacity(), cx); // [vector.capacity]/12
                }
        }
};

// Room for one more, then one inserted at p: the iterators before p keep their places and their values.
struct mem_insert_keeps_prefix
{
        template<class X>
        auto operator()(X const& a, std::size_t p) const
        {
                if (has_room(a, 1UZ) and p > 0UZ) {
                        auto b = a;
                        b.reserve(a.size() + 1UZ);
                        auto const before   = b.begin() + static_cast<X::difference_type>(p - 1UZ);
                        auto const capacity = b.capacity();
                        b.insert(nth(b, p), true);
                        if (b.capacity() == capacity) {
                                BOOST_CHECK(before == b.begin() + static_cast<X::difference_type>(p - 1UZ));  // [vector.modifiers]/2
                                BOOST_CHECK_EQUAL(static_cast<bool>(*before), static_cast<bool>(a[p - 1UZ])); // [vector.modifiers]/2
                        }
                }
        }
};

// One erased at p: nothing is thrown, and the iterators before p keep their places and their values.
struct mem_erase_keeps_prefix
{
        template<class X>
        auto operator()(X const& a, std::size_t p) const
        {
                if (p < a.size()) {
                        auto b           = a;
                        auto const first = b.begin();
                        BOOST_CHECK_NO_THROW(b.erase(nth(b, p))); // [vector.modifiers]/5 [inplace.vector.modifiers]/20
                        if (p > 0UZ) {
                                BOOST_CHECK(first == b.begin()); // [vector.modifiers]/4 [inplace.vector.modifiers]/19
                                auto const before = b.begin() + static_cast<X::difference_type>(p - 1UZ);
                                BOOST_CHECK_EQUAL(static_cast<bool>(*before), static_cast<bool>(a[p - 1UZ])); // [vector.modifiers]/4
                        }
                }
        }
};

// One function per door: a BOOST_CHECK_THROW is several branches, and the doors together pass the complexity threshold.

// A full sequence refuses even one more, in place or at the end.
template<class X>
auto check_one_past_capacity(X& b) -> void // NOLINT(bugprone-exception-escape)
{
        BOOST_CHECK_THROW(b.insert(b.cbegin(), true), std::bad_alloc);
        BOOST_CHECK_THROW(b.emplace(b.cbegin(), true), std::bad_alloc);
}

// However many it holds, one more than its room is refused, counted or ranged.
template<class X>
auto check_counted_past_capacity(X& b, std::vector<bool> const& more) -> void // NOLINT(bugprone-exception-escape)
{
        BOOST_CHECK_THROW(b.insert(b.cbegin(), more.size(), true), std::bad_alloc);
        BOOST_CHECK_THROW(b.insert(b.cbegin(), more.begin(), more.end()), std::bad_alloc);
}

// The standard libraries without P1206R7 have no range members to check.
template<class X>
auto check_ranged_past_capacity(X& b, std::vector<bool> const& more) -> void // NOLINT(bugprone-exception-escape)
{
        if constexpr (requires { b.append_range(more); }) {
                BOOST_CHECK_THROW(b.insert_range(b.cend(), more), std::bad_alloc);
                BOOST_CHECK_THROW(b.append_range(more), std::bad_alloc);
        }
}

// [inplace.vector.modifiers]/3: an insertion past the capacity throws bad_alloc and leaves the value alone.
struct mem_insert_past_capacity
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto b          = a;
                auto const more = alternating(a.max_size() - a.size() + 1UZ);
                if (not has_room(a, 1UZ)) {
                        check_one_past_capacity(b);
                }
                check_counted_past_capacity(b, more);
                check_ranged_past_capacity(b, more);
                BOOST_CHECK(b == a); // [inplace.vector.modifiers]/3
        }
};

// An iterator over the bools that admits to input and nothing more, so a container gets exactly one pass.
class single_pass_iterator
{
public:
        using iterator_concept  = std::input_iterator_tag;
        using iterator_category = std::input_iterator_tag;
        using value_type        = bool;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = bool;

        single_pass_iterator() = default;

        explicit single_pass_iterator(std::vector<bool>::const_iterator it)
                : m_it(it)
        {}

        [[nodiscard]] auto operator*() const
                -> bool
        {
                return *m_it;
        }

        auto operator++()
                -> single_pass_iterator&
        {
                ++m_it;
                return *this;
        }

        auto operator++(int)
                -> single_pass_iterator
        {
                auto const old = *this;
                ++m_it;
                return old;
        }

        [[nodiscard]] friend auto operator==(single_pass_iterator const&, single_pass_iterator const&) -> bool = default;

private:
        std::vector<bool>::const_iterator m_it;
};

// The bools as an input range with no size, whose count a container learns only by reaching its end.
[[nodiscard]] inline auto single_pass(std::vector<bool> const& v)
{
        return std::ranges::subrange(single_pass_iterator(v.cbegin()), single_pass_iterator(v.cend()));
}

// The bools as a forward range with no size, which a container may count by walking it before it writes.
[[nodiscard]] inline auto unsized_forward(std::vector<bool> const& v)
{
        return v | std::views::filter([](bool) -> bool { return true; });
}

template<class X>
inline constexpr auto is_std_inplace_vector_v = false;

#ifdef TEST_HAS_INPLACE_VECTOR

template<std::size_t N>
inline constexpr auto is_std_inplace_vector_v<std::inplace_vector<bool, N>> = true;

#endif

#ifdef __GLIBCXX__

// libstdc++ fills a single pass up to the capacity and then throws, where [inplace.vector.modifiers]/3 has no effects.
inline constexpr auto std_inplace_vector_single_pass_has_no_effects = false;

#else

inline constexpr auto std_inplace_vector_single_pass_has_no_effects = true;

#endif

template<class X>
concept single_pass_overflow_has_no_effects = std_inplace_vector_single_pass_has_no_effects or not is_std_inplace_vector_v<X>;

// No effects where the library gives them, and the weaker remainder of [inplace.vector.modifiers]/3 where not.
template<class X>
auto check_single_pass_left_alone(X const& a, X const& b) -> void
{
        if constexpr (single_pass_overflow_has_no_effects<X>) {
                BOOST_CHECK(b == a);
        } else {
                BOOST_CHECK_GE(b.size(), a.size());
                BOOST_CHECK(std::ranges::equal(a, b | std::views::take(a.size())));
        }
}

// A single pass one past the room, in place at position p: the throw comes only once the capacity is overrun.
template<class X>
auto check_single_pass_past_capacity(X const& a, std::size_t p, std::vector<bool> const& more) -> void // NOLINT(bugprone-exception-escape)
{
        auto b = a;
        BOOST_CHECK_THROW(b.insert(nth(b, p), single_pass_iterator(more.cbegin()), single_pass_iterator(more.cend())), std::bad_alloc);
        check_single_pass_left_alone(a, b);
        if constexpr (requires { b.insert_range(nth(b, p), single_pass(more)); }) {
                auto c = a;
                BOOST_CHECK_THROW(c.insert_range(nth(c, p), single_pass(more)), std::bad_alloc);
                check_single_pass_left_alone(a, c);
        }
}

// The same overrun from a forward range with no size, which every library can count before writing.
template<class X>
auto check_unsized_forward_past_capacity(X const& a, std::size_t p, std::vector<bool> const& more) -> void // NOLINT(bugprone-exception-escape)
{
        if constexpr (requires (X& b) { b.insert_range(nth(b, p), unsized_forward(more)); }) {
                auto b = a;
                BOOST_CHECK_THROW(b.insert_range(nth(b, p), unsized_forward(more)), std::bad_alloc);
                BOOST_CHECK(b == a);
        }
}

// At the end, where append_range is the door as well as insert_range.
template<class X>
auto check_single_pass_append_past_capacity(X const& a, std::vector<bool> const& more) -> void // NOLINT(bugprone-exception-escape)
{
        if constexpr (requires (X& b) { b.append_range(single_pass(more)); }) {
                auto b = a;
                BOOST_CHECK_THROW(b.append_range(single_pass(more)), std::bad_alloc);
                check_single_pass_left_alone(a, b);
        }
}

template<class X>
auto check_unsized_forward_append_past_capacity(X const& a, std::vector<bool> const& more) -> void // NOLINT(bugprone-exception-escape)
{
        if constexpr (requires (X& b) { b.append_range(unsized_forward(more)); }) {
                auto b = a;
                BOOST_CHECK_THROW(b.append_range(unsized_forward(more)), std::bad_alloc);
                BOOST_CHECK(b == a);
        }
}

// Exactly the room from a single pass fits, in place at position p or at the end.
template<class X>
auto check_single_pass_fits(X const& a, std::size_t p, std::vector<bool> const& fits) -> void
{
        auto const m = spliced(model_of(a), p, fits);
        auto b       = a;
        b.insert(nth(b, p), single_pass_iterator(fits.cbegin()), single_pass_iterator(fits.cend()));
        BOOST_CHECK(model_of(b) == m);
        if constexpr (requires { b.insert_range(nth(b, p), single_pass(fits)); }) {
                auto c = a;
                c.insert_range(nth(c, p), single_pass(fits));
                BOOST_CHECK(model_of(c) == m);
                auto d = a;
                d.append_range(single_pass(fits));
                BOOST_CHECK(model_of(d) == spliced(model_of(a), a.size(), fits));
        }
}

// [inplace.vector.modifiers]/3 from ranges with no size: up to the room they fit, and one past it has no effect.
struct mem_insert_unsized_past_capacity
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto const room = a.max_size() - a.size();
                auto const fits = alternating(room);
                auto const more = alternating(room + 1UZ);
                for (auto const p : std::views::iota(0UZ, a.size() + 1UZ)) {
                        check_single_pass_fits(a, p, fits);
                        check_single_pass_past_capacity(a, p, more);
                        check_unsized_forward_past_capacity(a, p, more);
                }
                check_single_pass_append_past_capacity(a, more);
                check_unsized_forward_append_past_capacity(a, more);
        }
};

// A full sequence refuses to grow at the end, and is left as it was.
template<class X>
auto check_push_back_past_capacity(X const& a, bool t) -> void // NOLINT(bugprone-exception-escape)
{
        auto b = a;
        auto c = a;
        BOOST_CHECK_THROW(b.push_back(t), std::bad_alloc);    // [inplace.vector.modifiers]/5
        BOOST_CHECK_THROW(c.emplace_back(t), std::bad_alloc); // [inplace.vector.modifiers]/5
        BOOST_CHECK(b == a and c == a);                       // [inplace.vector.modifiers]/7
}

// [inplace.vector.modifiers]/4-5,7: the end grows by one where there is room, and throws with no effect where not.
struct mem_push_back_or_throw
{
        template<class X>
        auto operator()(X const& a, bool t) const
        {
                auto b = a;
                auto c = a;
                static_assert(std::same_as<decltype(b.push_back(t)), typename X::reference>);
                static_assert(std::same_as<decltype(c.emplace_back(t)), typename X::reference>);
                if (has_room(a, 1UZ)) {
                        auto const m = spliced(model_of(a), a.size(), {t});
                        BOOST_CHECK_EQUAL(static_cast<bool>(b.push_back(t)), t); // [inplace.vector.modifiers]/4
                        BOOST_CHECK(model_of(b) == m);
                        BOOST_CHECK_EQUAL(static_cast<bool>(c.emplace_back(t)), t);
                        BOOST_CHECK(model_of(c) == m);
                } else {
                        check_push_back_past_capacity(a, t);
                }
        }
};

// [inplace.vector.modifiers]/10-12: the end grows by one where there is room, and the answer is disengaged where not.
struct mem_try_emplace_back
{
        template<class X>
        auto operator()(X const& a, bool t) const
        {
                auto b       = a;
                auto c       = a;
                auto const r = b.try_emplace_back(t);
                auto const s = c.try_push_back(t);
                if (has_room(a, 1UZ)) {
                        auto const m = spliced(model_of(a), a.size(), {t});
                        BOOST_CHECK(static_cast<bool>(r) and static_cast<bool>(*r) == t); // [inplace.vector.modifiers]/11
                        BOOST_CHECK(model_of(b) == m);                                    // [inplace.vector.modifiers]/10
                        BOOST_CHECK(static_cast<bool>(s) and static_cast<bool>(*s) == t);
                        BOOST_CHECK(model_of(c) == m);
                } else {
                        BOOST_CHECK(not static_cast<bool>(r)); // [inplace.vector.modifiers]/11
                        BOOST_CHECK(not static_cast<bool>(s));
                        BOOST_CHECK(b == a and c == a); // [inplace.vector.modifiers]/10
                        check_full_does_not_throw(b, t);
                }
        }

private:
        // A full vector answers without an exception, either door.
        template<class X>
        static auto check_full_does_not_throw(X& b, bool t)
                -> void
        {
                BOOST_CHECK_NO_THROW(static_cast<void>(b.try_push_back(t)));    // [inplace.vector.modifiers]/12
                BOOST_CHECK_NO_THROW(static_cast<void>(b.try_emplace_back(t))); // [inplace.vector.modifiers]/12
        }
};

// [inplace.vector.modifiers]/16: the room is the caller's to establish, and the end grows by one into it.
struct mem_unchecked_emplace_back
{
        template<class X>
        auto operator()(X const& a, bool t) const
        {
                if (has_room(a, 1UZ)) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.unchecked_emplace_back(t)), typename X::reference>);
                        BOOST_CHECK_EQUAL(static_cast<bool>(b.unchecked_emplace_back(t)), t);
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), a.size(), {t})); // [inplace.vector.modifiers]/16
                }
        }
};

// [inplace.vector.modifiers]/18: as unchecked_emplace_back, from the value.
struct mem_unchecked_push_back
{
        template<class X>
        auto operator()(X const& a, bool t) const
        {
                if (has_room(a, 1UZ)) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.unchecked_push_back(t)), typename X::reference>);
                        BOOST_CHECK_EQUAL(static_cast<bool>(b.unchecked_push_back(t)), t);
                        BOOST_CHECK(model_of(b) == spliced(model_of(a), a.size(), {t})); // [inplace.vector.modifiers]/18
                }
        }
};

// [vector.bool.pspc]'s flip: every position negated.
struct mem_flip
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto b = a;
                static_assert(std::same_as<decltype(b.flip()), void>);
                b.flip();
                auto m = model_of(a);
                m.flip();
                BOOST_CHECK(model_of(b) == m); // [vector.bool.pspc]/12
        }
};

// A copy of the proxy refers to the bit the original does, so writing through it writes that bit.
struct ref_copy
{
        template<class X>
        auto operator()(X const& a, std::size_t i) const
        {
                auto b = a;
                auto r = b[i]; // NOLINT(misc-const-correctness): the copy is assigned through as a mutable proxy
                r      = not static_cast<bool>(a[i]);
                BOOST_CHECK_NE(static_cast<bool>(b[i]), static_cast<bool>(a[i])); // [vector.bool.pspc]/5
                auto m = model_of(a);
                m[i]   = not m[i];
                BOOST_CHECK(model_of(b) == m);
        }
};

// The proxy's end leaves the bit it referred to as it was.
struct ref_destructor
{
        template<class X>
        auto operator()(X const& a, std::size_t i) const
        {
                auto b = a;
                {
                        auto const r = b[i];
                        static_cast<void>(static_cast<bool>(r));
                }
                BOOST_CHECK(b == a); // [vector.bool.pspc]/6
        }
};

// The proxy is assigned from a bool, from another proxy and through const, and hands itself back each time.
struct mem_reference_assign
{
        template<class X>
        auto operator()(X const& a, std::size_t i, std::size_t j) const
        {
                auto b = a;
                b[i]   = b[j];
                auto m = model_of(a);
                m[i]   = m[j];
                BOOST_CHECK(model_of(b) == m); // [vector.bool.pspc]/7

                for (auto const x : {false, true}) {
                        auto c        = a;
                        auto r        = c[i];
                        auto const& s = (r = x);
                        BOOST_CHECK_EQUAL(static_cast<bool>(c[i]), x);       // [vector.bool.pspc]/7
                        BOOST_CHECK(std::addressof(s) == std::addressof(r)); // [vector.bool.pspc]/8

                        // P2321R2's const-qualified assignment, where the library has it.
                        auto d       = a;
                        auto const t = d[i];
                        if constexpr (requires { t = x; }) {
                                auto const& u = (t = x);
                                BOOST_CHECK_EQUAL(static_cast<bool>(d[i]), x);       // [vector.bool.pspc]/7
                                BOOST_CHECK(std::addressof(u) == std::addressof(t)); // [vector.bool.pspc]/8
                        }
                }
        }
};

// The proxy reads as the bit it refers to.
struct ref_operator_bool
{
        template<class X>
        auto operator()(X const& a, std::size_t i) const
        {
                auto b       = a;
                auto const m = model_of(a);
                BOOST_CHECK_EQUAL(static_cast<bool>(b[i]), static_cast<bool>(m[i])); // [vector.bool.pspc]/9
                b[i] = true;
                BOOST_CHECK(static_cast<bool>(b[i])); // [vector.bool.pspc]/9
                b[i] = false;
                BOOST_CHECK(not static_cast<bool>(b[i])); // [vector.bool.pspc]/9
        }
};

// [vector.bool.pspc]/10: the proxy's flip negates the one position it refers to.
struct mem_reference_flip
{
        template<class X>
        auto operator()(X const& a, std::size_t i) const
        {
                auto b = a;
                static_assert(std::same_as<decltype(b[i].flip()), void>);
                b[i].flip();
                auto m = model_of(a);
                m[i]   = not m[i];
                BOOST_CHECK(model_of(b) == m); // [vector.bool.pspc]/10
        }
};

// The three swaps found by argument-dependent lookup exchange two positions, or a position and a bool.
struct fn_swap_reference
{
        template<class X>
        auto operator()(X const& a, std::size_t i, std::size_t j) const
        {
                auto m = model_of(a);
                m[i]   = a[j];
                m[j]   = a[i];
                auto b = a;
                swap(b[i], b[j]);
                BOOST_CHECK(model_of(b) == m); // [vector.bool.pspc]/11

                if constexpr (reference_swaps_with_bool<X>) {
                        auto x = not static_cast<bool>(a[i]);
                        auto c = a;
                        swap(c[i], x);
                        BOOST_CHECK(x == static_cast<bool>(a[i]) and static_cast<bool>(c[i]) != x); // [vector.bool.pspc]/11
                        swap(x, c[i]);
                        BOOST_CHECK(c == a); // [vector.bool.pspc]/11
                }
        }
};

// The static member swap that C++26 keeps only as deprecated.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
struct mem_static_swap
{
        template<class X>
        auto operator()(X const& a, std::size_t i, std::size_t j) const
        {
                auto m = model_of(a);
                m[i]   = a[j];
                m[j]   = a[i];
                auto b = a;
                X::swap(b[i], b[j]);
                BOOST_CHECK(model_of(b) == m); // [depr.vector.bool.swap]/2
        }
};
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#else
#pragma warning(pop)
#endif

// [vector.erasure]: what equals the value, or satisfies the predicate, goes, and the count says how much did.
struct fn_erase
{
        template<class X>
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 17
        // GCC 17 trunk's ranger VRP segfaults on this body at -O3 with BMI2 and Intel tuning.
        [[gnu::optimize("no-tree-vrp")]]
#endif
        auto operator()(X const& a, bool t) const
        {
                auto b       = a;
                auto const m = model_of(a);
                auto const r = erase(b, t);
                static_assert(std::same_as<decltype(erase(b, t)), typename X::size_type>);
                BOOST_CHECK_EQUAL(r, static_cast<std::size_t>(std::ranges::count(m, t)));
                BOOST_CHECK(model_of(b) == std::vector<bool>(a.size() - r, not t)); // [vector.erasure]/1 [inplace.vector.erasure]/1
        }
};

struct fn_erase_if
{
        template<class X>
        auto operator()(X const& a, auto pred) const
        {
                auto b       = a;
                auto m       = model_of(a);
                auto const r = erase_if(b, pred);
                static_assert(std::same_as<decltype(erase_if(b, pred)), typename X::size_type>);
                auto const n = std::erase_if(m, pred);
                BOOST_CHECK_EQUAL(r, n);
                BOOST_CHECK(model_of(b) == m); // [vector.erasure]/2 [inplace.vector.erasure]/2
        }
};

// A bool spelled as [format.formatter.spec] spells one: its word by default, and 0 or 1 under d.
[[nodiscard]] inline auto bool_text(bool b, bool as_digit)
        -> std::string_view
{
        if (as_digit) {
                return b ? "1" : "0";
        }
        return b ? "true" : "false";
}

// Bracketed and comma-separated, as [format.range.formatter] writes any sequence.
template<class X>
[[nodiscard]] auto sequence_text(X const& a, bool as_digit)
        -> std::string
{
        auto text = std::string("[");
        for (auto const i : std::views::iota(0UZ, a.size())) {
                if (i != 0UZ) {
                        text += ", ";
                }
                text += bool_text(static_cast<bool>(a[i]), as_digit);
        }
        return text + "]";
}

// The proxy formats as the bool it stands for, under the same spec, and a sequence of them follows.
struct fn_format
{
        template<class X>
        auto operator()(X const& a) const
        {
                if constexpr (std::formattable<typename X::reference, char>) {
                        BOOST_CHECK_EQUAL(std::format("{}", a), sequence_text(a, false));
                        BOOST_CHECK_EQUAL(std::format("{::d}", a), sequence_text(a, true));
                        auto b = a;
                        for (auto const i : std::views::iota(0UZ, b.size())) {
                                auto const t = static_cast<bool>(b[i]);
                                BOOST_CHECK_EQUAL(std::format("{}", b[i]), bool_text(t, false));        // [vector.bool.fmt]/2
                                BOOST_CHECK_EQUAL(std::format("{:d}", b[i]), bool_text(t, true));       // [vector.bool.fmt]/1
                                BOOST_CHECK_EQUAL(std::format("{:>7}", b[i]), std::format("{:>7}", t)); // [vector.bool.fmt]/1
                        }
                }
        }
};

} // namespace test::sequence

#endif // TEST_SEQUENCE_PRIMITIVES_HPP
