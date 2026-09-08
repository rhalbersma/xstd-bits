//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_PRIMITIVES_HPP
#define TEST_SET_PRIMITIVES_HPP

#include <boost/test/unit_test.hpp>     // BOOST_CHECK, BOOST_CHECK_EQUAL
#include <xstd/bits/set_adaptor.hpp> // set_adaptor
#include <xstd/bits/ownership.hpp>     // ownership
#include <algorithm>                    // equal_range, lexicographical_compare_three_way
#include <compare>                      // is_gteq, is_gt, is_lteq, is_lt, strong_ordering
#include <concepts>                     // convertible_to, default_initializable, equality_comparable, integral, same_as, unsigned_integral
#include <cstddef>                      // ptrdiff_t
#include <functional>                   // hash
#include <initializer_list>             // initializer_list
#include <iterator>                     // distance, empty, iter_difference_t, iter_value_t, next, prev, reverse_iterator, size, ssize
#include <ranges>                       // count, equal, find, lexicographical_compare, lower_bound, , subrange, upper_bound
#include <type_traits>                  // add_const_t, common_type_t, make_signed_t, remove_reference_t
#include <utility>                      // declval, pair

namespace test::set {

template<class X>
struct ref_same_as_pred
{
        template<class R, class T>
        static constexpr auto value = std::same_as<R, T>;
};

// Every set adaptor hands out a proxy converting to the key, whatever its storage or ownership.
template<class Bits, xstd::ownership Own, class Traits>
struct ref_same_as_pred<xstd::set_adaptor<Bits, Own, Traits>>
{
        template<class R, class T>
        static constexpr auto value = std::convertible_to<R, std::add_const_t<std::remove_reference_t<T>>&>;
};

template<class X, class R, class T>
inline constexpr auto ref_same_as = ref_same_as_pred<X>::template value<R, T>;

template<class X, std::integral T = typename X::key_type>
struct nested_types
{
        static_assert(std::same_as<typename X::value_type, T>);                         // [container.reqmts]/2
        static_assert(requires { std::declval<X>().erase(std::declval<T>()); });        // [container.reqmts]/3

        static_assert(ref_same_as<X, typename X::reference, T&>);                       // [container.reqmts]/4
        static_assert(ref_same_as<X, typename X::const_reference, T const&>);           // [container.reqmts]/5

        static_assert(std::forward_iterator<typename X::iterator>);                     // [container.reqmts]/6
        static_assert(std::same_as<std::iter_value_t<typename X::iterator>, T>);
        static_assert(std::convertible_to<typename X::iterator, typename X::const_iterator>);

        static_assert(std::forward_iterator<typename X::const_iterator>);               // [container.reqmts]/7
        static_assert(std::same_as<std::iter_value_t<typename X::const_iterator>, T>);

                                                                                        // [container.reqmts]/8
        static_assert(std::same_as<typename X::difference_type, std::iter_difference_t<typename X::iterator>>);
        static_assert(std::same_as<typename X::difference_type, std::iter_difference_t<typename X::const_iterator>>);

        static_assert(std::bidirectional_iterator<typename X::reverse_iterator>);       // [container.rev.reqmts]/2
        static_assert(std::same_as<typename X::reverse_iterator, std::reverse_iterator<typename X::iterator>>);
        static_assert(std::same_as<std::iter_value_t<typename X::reverse_iterator>, T>);

        static_assert(std::bidirectional_iterator<typename X::const_reverse_iterator>); // [container.rev.reqmts]/3
        static_assert(std::same_as<typename X::const_reverse_iterator, std::reverse_iterator<typename X::const_iterator>>);
        static_assert(std::same_as<std::iter_value_t<typename X::const_reverse_iterator>, T>);

        using Key = T;
        static_assert(std::same_as<typename X::key_type, Key>);                         // [associative.reqmts.general]/9
        static_assert(std::same_as<typename X::value_type, Key>);                       // [associative.reqmts.general]/12
        static_assert(requires { std::declval<X>().erase(std::declval<Key>()); });      // [associative.reqmts.general]/13

        using Compare = std::less<Key>;
        static_assert(std::same_as<Compare, typename X::key_compare>);                  // [associative.reqmts.general]/14
        static_assert(std::copy_constructible<Key>);                                    // [associative.reqmts.general]/15
        static_assert(std::same_as<Compare, typename X::value_compare>);                // [associative.reqmts.general]/16
};

template<class X>
struct constructor
{
        auto operator()() const
        {
                X u;
                X u1 = X();
                BOOST_CHECK(u.empty());                                                 // [container.reqmts]/10
                BOOST_CHECK(u1.empty());                                                // [container.reqmts]/10
                static_assert(std::default_initializable<typename X::key_compare>);     // [associative.reqmts.general]/20
        }

        auto operator()(X a) const
        {
                X u(a);
                X u1 = a;
                BOOST_CHECK(u == a);                                                    // [container.reqmts]/10
                BOOST_CHECK(u1 == a);                                                   // [container.reqmts]/10
        }

        auto operator()(X&& rv) const
        {
                X u(rv);
                X u1 = rv;
                BOOST_CHECK(u == rv);                                                   // [container.reqmts]/10
                BOOST_CHECK(u1 == rv);                                                  // [container.reqmts]/10
        }

        template<std::input_iterator I>
        auto operator()(I i, I j) const
        {
                static_assert(std::default_initializable<typename X::key_compare>);     // [associative.reqmts.general]/26
                X u(i, j);
                X u1;
                u1.insert(i, j);
                BOOST_CHECK(u == u1);                                                   // [associative.reqmts.general]/27
        }

        template<std::ranges::input_range R>
        auto operator()(std::from_range_t, R&& rg) const
        {
                static_assert(std::default_initializable<typename X::key_compare>);     // [associative.reqmts.general]/32
                X u(std::from_range, rg);
                X u1;
                u1.insert(std::ranges::begin(rg), std::ranges::end(rg));
                BOOST_CHECK(u == u1);                                                   // [associative.reqmts.general]/33
        }

        auto operator()(std::initializer_list<typename X::value_type> il) const
        {
                X u(il);
                X u1(il.begin(), il.end());
                BOOST_CHECK(u == u1);                                                   // [associative.reqmts.general]/36
        }
};

struct op_assign
{
        template<class X>
        auto operator()(X& r, X a) const
        {
                r = a;
                static_assert(std::same_as<decltype(r), X&>);                           // [container.reqmts]/49
                BOOST_CHECK(r == a);                                                    // [container.reqmts]/50
        }

        template<class X>
        auto operator()(X& a, std::initializer_list<typename X::value_type> il) const   // [tab:container.assoc.req]
        {
                a = il;
                X a1(il);
                BOOST_CHECK(a == a1);
        }
};

struct mem_const_reference
{
        auto operator()(auto const& bs) const noexcept
        {
                for (auto const ref : bs) {
                        BOOST_CHECK(bs.count(ref));
                }
        }
};

struct mem_const_iterator
{
        template<class X>
        auto operator()(X& a) const noexcept
        {
                using I = X::iterator;
                static_assert(std::same_as<decltype(a.begin()), I>);            // [container.reqmts]/24
                static_assert(std::same_as<decltype(a.end()), I>);              // [container.reqmts]/27

                using R = X::reverse_iterator;
                static_assert(std::same_as<decltype(a.rbegin()), R>);           // [container.rev.reqmts]/4
                BOOST_CHECK((a.rbegin() == std::reverse_iterator(a.end())));    // [container.rev.reqmts]/5

                static_assert(std::same_as<decltype(a.rend()), R>);             // [container.rev.reqmts]/7
                BOOST_CHECK((a.rend()   == std::reverse_iterator(a.begin())));  // [container.rev.reqmts]/8
        }

        template<class X>
        auto operator()(const X& a) const noexcept
        {
                using I = X::const_iterator;
                static_assert(std::same_as<decltype(a.begin()), I>);            // [container.reqmts]/24
                static_assert(std::same_as<decltype(a.end()), I>);              // [container.reqmts]/27

                static_assert(std::same_as<decltype(a.cbegin()), I>);           // [container.reqmts]/30
                BOOST_CHECK(a.cbegin() == const_cast<X const&>(a).begin());     // [container.reqmts]/31

                static_assert(std::same_as<decltype(a.cend()), I>);             // [container.reqmts]/33
                BOOST_CHECK(a.cend()   == const_cast<X const&>(a).end());       // [container.reqmts]/34


                using R = X::const_reverse_iterator;
                static_assert(std::same_as<decltype(a.rbegin()), R>);           // [container.rev.reqmts]/4
                BOOST_CHECK((a.rbegin() == std::reverse_iterator(a.end())));    // [container.rev.reqmts]/5

                static_assert(std::same_as<decltype(a.rend()), R>);             // [container.rev.reqmts]/7
                BOOST_CHECK((a.rend()   == std::reverse_iterator(a.begin())));  // [container.rev.reqmts]/8

                static_assert(std::same_as<decltype(a.crbegin()), R>);          // [container.rev.reqmts]/10
                BOOST_CHECK(a.crbegin() == const_cast<X const&>(a).rbegin());   // [container.rev.reqmts]/11

                static_assert(std::same_as<decltype(a.crend()), R>);            // [container.rev.reqmts]/13
                BOOST_CHECK(a.crend()   == const_cast<X const&>(a).rend());     // [container.rev.reqmts]/14
        }

        template<std::input_iterator I>
        auto operator()(I i, I j) const noexcept
        {
                if constexpr (std::random_access_iterator<I>) {                                 // [container.reqmts]/37
                        static_assert(std::same_as<decltype(i <=> j), std::strong_ordering>);   // [container.reqmts]/36
                }
        }
};

struct op_equal_to
{
        template<class X, class Key = X::value_type>
        auto operator()(const X& a, const X& b) const noexcept
        {
                static_assert(std::equality_comparable<Key>);                           // [container.reqmts]/39
                static_assert(std::convertible_to<decltype(a == b), bool>);             // [container.reqmts]/40
                BOOST_CHECK_EQUAL(a == b, std::ranges::equal(a, b));                    // [container.reqmts]/41
                static_assert(std::equivalence_relation<std::equal_to<X>, X, X>);       // [container.reqmts]/43
        }
};

// Equal values hash equal wherever a std::hash exists: the set adaptor has one as std::string_view does, std::set none. [design.md#the-hashing-invariant]
struct op_hash
{
        template<class X>
        auto operator()(const X& a) const noexcept
        {
                if constexpr (requires { std::hash<X>()(a); }) {
                        BOOST_CHECK_EQUAL(std::hash<X>()(a), std::hash<X>()(X(a)));
                }
        }

        template<class X>
        auto operator()(const X& a, const X& b) const noexcept
        {
                if constexpr (requires { std::hash<X>()(a); }) {
                        BOOST_CHECK(a != b or std::hash<X>()(a) == std::hash<X>()(b));
                }
        }
};

struct op_not_equal_to
{
        auto operator()(auto const& a, auto const& b) const noexcept
        {
                BOOST_CHECK_EQUAL(a != b, not (a == b));                        // [container.reqmts]/44
        }
};

struct mem_swap
{
        auto operator()(auto& a, auto& b) const noexcept
        {
                static_assert(std::same_as<decltype(a.swap(b)), void>);         // [container.reqmts]/45
                auto a1 = a, b1 = b;
                a1.swap(b1);
                BOOST_CHECK(a1 == b and b1 == a);                               // [container.reqmts]/46
        }
};

struct fn_swap
{
        auto operator()(auto& a, auto& b) const noexcept
        {
                auto a1 = a, b1 = b;
                auto a2 = a, b2 = b;
                swap(a1, b1); a2.swap(b2);
                BOOST_CHECK(a1 == a2 and b1 == b2);                             // [container.reqmts]/48
        }
};

struct mem_size
{
        template<class X>
        auto operator()(const X& a) const noexcept
        {
                static_assert(std::same_as<decltype(a.size()), typename X::size_type>);                         // [container.reqmts]/52
                BOOST_CHECK_EQUAL(static_cast<std::ptrdiff_t>(a.size()), std::distance(a.begin(), a.end()));    // [container.reqmts]/53
        }
};

struct mem_max_size
{
        template<class X>
        auto operator()(const X& a) const noexcept
        {
                static_assert(std::same_as<decltype(a.max_size()), typename X::size_type>);     // [container.reqmts]/56
                BOOST_CHECK_LE(a.size(), a.max_size());                                         // [container.reqmts]/57
        }
};

struct mem_empty
{
        auto operator()(auto const& a) const noexcept
        {
                static_assert(std::convertible_to<decltype(a.empty()), bool>);  // [container.reqmts]/59
                BOOST_CHECK_EQUAL(a.empty(), a.begin() == a.end());             // [container.reqmts]/60
        }
};

struct mem_front
{
        template<class X, class R = X::const_reference>
        auto operator()(const X& a [[maybe_unused]]) const noexcept
        {
                if constexpr (requires { a.front(); }) {
                        static_assert(std::same_as<decltype(a.front()), R>);    // [sequence.reqmts]/70
                        BOOST_CHECK(a.empty() or (a.front() == *a.begin()));    // [sequence.reqmts]/71
                }
        }
};

struct mem_back
{
        template<class X, class R = X::const_reference>
        auto operator()(const X& a [[maybe_unused]]) const noexcept
        {
                if constexpr (requires { a.back(); }) {
                        static_assert(std::same_as<decltype(a.back()), R>);             // [sequence.reqmts]/73
                        BOOST_CHECK(a.empty() or (a.back() == *std::prev(a.end())));    // [sequence.reqmts]/74
                }
        }
};

struct mem_emplace
{
        template<class X, class... Args>
        auto operator()(X& a, Args&&... args) const
        {                                                                       // [associative.reqmts.general]/47
                static_assert(
                        std::same_as<
                                decltype(a.emplace(std::forward<Args>(args)...)),
                                std::pair<typename X::iterator, bool>
                        >
                );

                static_assert(std::constructible_from<typename X::value_type, Args...>);                // [associative.reqmts.general]/48
                // Built once and then used three times. Forwarding three times reads args twice after moving
                // from it: harmless where value_type is a size_t, wrong the moment this harness meets a type
                // where a move is not a copy, and bugprone-use-after-move is right to say so.
                auto const value = typename X::value_type(std::forward<Args>(args)...);
                auto const emplaced = not a.contains(value);
                auto r = a.emplace(value);                                                              // [associative.reqmts.general]/49
                                                                                // [associative.reqmts.general]/50
                BOOST_CHECK(r == std::make_pair(a.find(value), emplaced));
        }
};

struct mem_emplace_hint
{
        template<class X, class... Args>
        auto operator()(X& a, X::iterator p, Args&&... args) const
        {
                static_assert(
                        std::same_as<                                                           // [associative.reqmts.general]/57
                                decltype(a.emplace_hint(p, std::forward<Args>(args)...)),
                                typename X::iterator
                        >
                );
                // Built once, for the reason mem_emplace gives.
                auto const value = typename X::value_type(std::forward<Args>(args)...);
                auto r = a.emplace_hint(p, value);                                              // [associative.reqmts.general]/58
                BOOST_CHECK(r == a.find(value));                                                // [associative.reqmts.general]/59
        }
};

struct mem_insert
{
        template<class X>
        auto operator()(X& a, X::value_type const& t) const
        {
                static_assert(std::same_as<decltype(a.insert(t)), std::pair<typename X::iterator, bool>>);      // [associative.reqmts.general]/61
                static_assert(std::constructible_from<typename X::value_type, decltype(t)>);                    // [associative.reqmts.general]/62
                auto const inserted = not a.contains(t);
                auto const r = a.insert(t);                                                                     // [associative.reqmts.general]/63
                BOOST_CHECK(r == std::make_pair(a.find(t), inserted));                                          // [associative.reqmts.general]/64
        }

        template<class X>
        auto operator()(X& a, X::value_type&& t) const
        {                                                                       // [associative.reqmts.general]/61
                static_assert(std::same_as<decltype(a.insert(t)), std::pair<typename X::iterator, bool>>);
                                                                                // [associative.reqmts.general]/62
                static_assert(std::constructible_from<typename X::value_type, decltype(t)>);
                auto const inserted = not a.contains(t);
                auto const r = a.insert(t);                                     // [associative.reqmts.general]/63
                BOOST_CHECK(r == std::make_pair(a.find(t), inserted));          // [associative.reqmts.general]/64
        }

        template<class X>
        auto operator()(X& a, X::iterator p, X::value_type const& t) const
        {
                auto r = a.insert(p, t);
                static_assert(std::same_as<decltype(r), typename X::iterator>); // [associative.reqmts.general]/70
                static_assert(requires { a.insert(p, t); });                    // [associative.reqmts.general]/71
                BOOST_CHECK(r == a.find(t));                                    // [associative.reqmts.general]/73
        }

        template<class X>
        auto operator()(X& a, X::iterator p, X::value_type&& t) const
        {
                auto r = a.insert(p, t);
                static_assert(std::same_as<decltype(r), typename X::iterator>); // [associative.reqmts.general]/70
                static_assert(requires { a.insert(p, t); });                    // [associative.reqmts.general]/71
                BOOST_CHECK(r == a.find(t));                                    // [associative.reqmts.general]/73
        }

        template<class X, std::input_iterator I>
        auto operator()(X& a, I i, I j) const
        {
                static_assert(std::same_as<decltype(a.insert(i, j)), void>);    // [associative.reqmts.general]/75
                                                                                // [associative.reqmts.general]/76
                static_assert(std::constructible_from<typename X::value_type, decltype(*i)>);
                auto a1 = a;
                a.insert(i, j);
                for (auto const& t : std::ranges::subrange(i, j)) {
                        a1.insert(t);
                }
                BOOST_CHECK(a == a1);                                           // [associative.reqmts.general]/77
        }

        template<class X, std::ranges::input_range R>
        auto operator()(X& a, R&& rg) const
        {                                                                       // [associative.reqmts.general]/79
                static_assert(std::same_as<decltype(a.insert_range(rg)), void>);
                                                                                // [associative.reqmts.general]/80
                static_assert(std::constructible_from<typename X::value_type, decltype(*rg.begin())>);
                auto a1 = a;
                a.insert_range(rg);
                for (auto const& t : rg) {
                        a1.insert(t);
                }
                BOOST_CHECK(a == a1);                                           // [associative.reqmts.general]/81
        }

        template<class X>
        auto operator()(X& a, std::initializer_list<typename X::value_type> il) const
        {
                auto a1 = a;
                a.insert(il);
                a1.insert(il.begin(), il.end());
                BOOST_CHECK(a == a1);                                           // [associative.reqmts.general]/83
        }
};

struct mem_erase
{
        template<class X>
        auto operator()(X& a, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(a.erase(k)), typename X::size_type>);       // [associative.reqmts.general]/118
                auto const erased = a.count(k);
                auto const returns = a.erase(k);                                                // [associative.reqmts.general]/119
                BOOST_CHECK(returns == erased);                                                 // [associative.reqmts.general]/120
        }

        template<class X>
        auto operator()(X& a, X::const_iterator q) const
        {
                static_assert(std::same_as<decltype(a.erase(q)), typename X::iterator>);        // [associative.reqmts.general]/126
                BOOST_CHECK(q != a.end());
                auto const expected = std::next(q);                                             // assumes erase does not invalidate iterators
                auto const returns = a.erase(q);                                                // [associative.reqmts.general]/127
                BOOST_CHECK(returns == expected);                                               // [associative.reqmts.general]/128
        }

        template<class X>
        auto operator()(X& a, X::const_iterator q1, X::const_iterator q2) const
        {
                static_assert(std::same_as<decltype(a.erase(q1, q2)), typename X::iterator>);   // [associative.reqmts.general]/134
                auto const expected = q2;                                                       // assumes erase does not invalidate iterators
                auto const returns = a.erase(q1, q2);                                           // [associative.reqmts.general]/135
                BOOST_CHECK(returns == expected);                                               // [associative.reqmts.general]/136
        }
};

struct mem_clear
{
        auto operator()(auto& a) const noexcept
        {
                auto a1 = a;
                a.clear();
                a1.erase(a1.begin(), a1.end());
                BOOST_CHECK(a == a1);                                                           // [associative.reqmts.general]/138
                BOOST_CHECK(a.empty());                                                         // [associative.reqmts.general]/139
        }
};

struct mem_find
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.find(k)), typename X::iterator>);         // [associative.reqmts.general]/141
                BOOST_CHECK(b.find(k) == std::ranges::find(b, k));                              // [associative.reqmts.general]/142
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.find(k)), typename X::const_iterator>);   // [associative.reqmts.general]/141
                BOOST_CHECK(b.find(k) == std::ranges::find(b, k));                              // [associative.reqmts.general]/142
        }
};

struct mem_count
{
        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.count(k)), typename X::size_type>);               // [associative.reqmts.general]/147
                BOOST_CHECK_EQUAL(static_cast<std::ptrdiff_t>(b.count(k)), std::ranges::count(b, k));   // [associative.reqmts.general]/148
        }
};

struct mem_contains
{
        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.contains(k)), bool>);
                BOOST_CHECK_EQUAL(b.contains(k), b.find(k) != b.end());                                 // [associative.reqmts.general]/154
        }
};

struct mem_lower_bound
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.lower_bound(k)), typename X::iterator>);          // [associative.reqmts.general]/157
                BOOST_CHECK(b.lower_bound(k) == std::ranges::lower_bound(b, k));                        // [associative.reqmts.general]/158
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.lower_bound(k)), typename X::const_iterator>);    // [associative.reqmts.general]/157
                BOOST_CHECK(b.lower_bound(k) == std::ranges::lower_bound(b, k));                        // [associative.reqmts.general]/158
        }
};

struct mem_upper_bound
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.upper_bound(k)), typename X::iterator>);          // [associative.reqmts.general]/163
                BOOST_CHECK(b.upper_bound(k) == std::ranges::upper_bound(b, k));                        // [associative.reqmts.general]/164
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.upper_bound(k)), typename X::const_iterator>);    // [associative.reqmts.general]/163
                BOOST_CHECK(b.upper_bound(k) == std::ranges::upper_bound(b, k));                        // [associative.reqmts.general]/164
        }
};

struct mem_equal_range
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                using iterator = X::iterator;
                static_assert(std::same_as<decltype(b.equal_range(k)), std::pair<iterator, iterator>>); // [associative.reqmts.general]/169
                BOOST_CHECK(b.equal_range(k) == std::make_pair(b.lower_bound(k), b.upper_bound(k)));    // [associative.reqmts.general]/170
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                using const_iterator = X::const_iterator;
                static_assert(std::same_as<decltype(b.equal_range(k)), std::pair<const_iterator, const_iterator>>);     // [associative.reqmts.general]/169
                BOOST_CHECK(b.equal_range(k) == std::make_pair(b.lower_bound(k), b.upper_bound(k)));                    // [associative.reqmts.general]/170
        }
};

struct op_compare_three_way
{
        auto operator()(auto const& a) const noexcept
        {
                // std::strong_ordering does not have output streaming operator<< required for BOOST_CHECK_EQUAL
                BOOST_CHECK((a <=> a) == std::strong_ordering::equal);          // reflexive
        }

        auto operator()(auto const& a, auto const& b) const noexcept
        {
                static_assert(std::same_as<decltype(a <=> b), std::strong_ordering>);                                           // [tab:container.req]
                BOOST_CHECK((a <=> b) == std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end()));       // [tab:container.opt]
        }
};

struct op_less
{
        auto operator()(auto const& a) const noexcept
        {                                                                       // [tab:container.opt]
                BOOST_CHECK(not (a < a));                                       // irreflexive
        }

        auto operator()(auto const& a, auto const& b) const noexcept
        {                                                                       // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a < b), bool>);
                BOOST_CHECK_EQUAL(a < b, std::is_lt(a <=> b));
                BOOST_CHECK_EQUAL(a < b, std::ranges::lexicographical_compare(a, b));
                BOOST_CHECK(not (a < b) or not (b < a));                        // asymmetric
        }

        auto operator()(auto const& a, auto const& b, auto const& c) const noexcept
        {                                                                       // [tab:container.opt]
                BOOST_CHECK(not (a < b and b < c) or a < c);                    // transitive
        }
};

struct op_greater
{
        auto operator()(auto const& a, auto const& b) const noexcept
        {                                                                       // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a > b), bool>);
                BOOST_CHECK_EQUAL(a > b, std::is_gt(a <=> b));
                BOOST_CHECK_EQUAL(a > b, b < a);
        }
};

struct op_less_equal
{
        auto operator()(auto const& a, auto const& b) const noexcept
        {                                                                       // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a <= b), bool>);
                BOOST_CHECK_EQUAL(a <= b, std::is_lteq(a <=> b));
                BOOST_CHECK_EQUAL(a <= b, not (b < a));
        }
};

struct op_greater_equal
{
        auto operator()(auto const& a, auto const& b) const noexcept
        {                                                                       // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a >= b), bool>);
                BOOST_CHECK_EQUAL(a >= b, std::is_gteq(a <=> b));
                BOOST_CHECK_EQUAL(a >= b, not (a < b));
        }
};

struct fn_iterator
{
        auto operator()(auto& c) const noexcept
        {
                BOOST_CHECK(std::begin(c)   == c.begin());                      // [iterator.range]/2
                BOOST_CHECK(std::end(c)     == c.end());                        // [iterator.range]/3

                BOOST_CHECK(std::rbegin(c)  == c.rbegin());                     // [iterator.range]/8
                BOOST_CHECK(std::rend(c)    == c.rend());                       // [iterator.range]/9
        }

        auto operator()(auto const& c) const noexcept
        {
                BOOST_CHECK(std::begin(c)   == c.begin());                      // [iterator.range]/2
                BOOST_CHECK(std::end(c)     == c.end());                        // [iterator.range]/3

                BOOST_CHECK(std::cbegin(c)  == std::begin(c));                  // [iterator.range]/6
                BOOST_CHECK(std::cend(c)    == std::end(c));                    // [iterator.range]/7

                BOOST_CHECK(std::rbegin(c)  == c.rbegin());                     // [iterator.range]/8
                BOOST_CHECK(std::rend(c)    == c.rend());                       // [iterator.range]/9

                BOOST_CHECK(std::crbegin(c) == std::rbegin(c));                 // [iterator.range]/14
                BOOST_CHECK(std::crend(c)   == std::rend(c));                   // [iterator.range]/15
        }
};

struct fn_size
{
        auto operator()(auto const& c) const noexcept
        {
                BOOST_CHECK_EQUAL(std::size(c), c.size());                      // [iterator.range]/16
        }
};

struct fn_ssize
{
        auto operator()(auto const& c) const noexcept
        {
                using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
                BOOST_CHECK_EQUAL(std::ssize(c), static_cast<R>(c.size()));     // [iterator.range]/18
        }
};

struct fn_empty
{
        auto operator()(auto const& c) const noexcept
        {
                BOOST_CHECK_EQUAL(std::empty(c), c.empty());                    // [iterator.range]/20
        }
};

} // namespace test::set

#endif // TEST_SET_PRIMITIVES_HPP
