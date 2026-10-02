//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_PRIMITIVES_HPP
#define TEST_SET_PRIMITIVES_HPP

#include <test/container/allocator.hpp> // strong_guarantee
#include <test/reference.hpp>           // proxy_reference
#include <boost/test/unit_test.hpp>     // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_LE, BOOST_CHECK_NO_THROW
#include <algorithm>                    // adjacent_find, all_of, copy, equal_range, includes, is_sorted, lexicographical_compare_three_way, max, set_difference, set_intersection, set_symmetric_difference, set_union, sort
#include <compare>                      // is_gteq, is_gt, is_lteq, is_lt, strong_ordering
#include <concepts>                     // convertible_to, default_initializable, equality_comparable, integral, same_as, unsigned_integral
#include <cstddef>                      // ptrdiff_t
#include <functional>                   // hash, identity, less
#include <initializer_list>             // initializer_list
#include <iterator>                     // back_inserter, distance, empty, iter_difference_t, iter_value_t, next, prev, reverse_iterator, size, ssize
#include <limits>                       // numeric_limits
#include <ranges>                       // count, equal, find, lexicographical_compare, lower_bound, , subrange, upper_bound
#include <set>                          // erase_if, set
#include <type_traits>                  // add_const_t, common_type_t, make_signed_t, remove_reference_t
#include <utility>                      // declval, move, pair
#include <vector>                       // vector

namespace test::set {

// The reference type asked for, or where X hands out a proxy for it, one converting to a reference to the key.
template<class X, class R, class T>
concept ref_same_as = std::same_as<R, T> or (test::proxy_reference<X> and std::convertible_to<R, std::add_const_t<std::remove_reference_t<T>>&>);

template<class X, std::integral T = typename X::key_type> // NOLINT(readability-redundant-typename): MSVC 17 reads the constrained parameter as C2061 without it.
constexpr auto nested_types()
        -> void
{
        static_assert(std::same_as<typename X::value_type, T>); // [container.reqmts]/2
        static_assert(requires { std::declval<X>().erase(std::declval<T>()); });

        static_assert(ref_same_as<X, typename X::reference, T&>);             // [container.reqmts]/4
        static_assert(ref_same_as<X, typename X::const_reference, T const&>); // [container.reqmts]/5

        static_assert(std::forward_iterator<typename X::iterator>); // [container.reqmts]/6
        static_assert(std::same_as<std::iter_value_t<typename X::iterator>, T>);
        static_assert(std::convertible_to<typename X::iterator, typename X::const_iterator>);

        static_assert(std::forward_iterator<typename X::const_iterator>); // [container.reqmts]/7
        static_assert(std::same_as<std::iter_value_t<typename X::const_iterator>, T>);

        // [container.reqmts]/8
        static_assert(std::same_as<typename X::difference_type, std::iter_difference_t<typename X::iterator>>);
        static_assert(std::same_as<typename X::difference_type, std::iter_difference_t<typename X::const_iterator>>);

        static_assert(std::bidirectional_iterator<typename X::reverse_iterator>); // [container.rev.reqmts]/2
        static_assert(std::same_as<typename X::reverse_iterator, std::reverse_iterator<typename X::iterator>>);
        static_assert(std::same_as<std::iter_value_t<typename X::reverse_iterator>, T>);

        static_assert(std::bidirectional_iterator<typename X::const_reverse_iterator>); // [container.rev.reqmts]/3
        static_assert(std::same_as<typename X::const_reverse_iterator, std::reverse_iterator<typename X::const_iterator>>);
        static_assert(std::same_as<std::iter_value_t<typename X::const_reverse_iterator>, T>);

        using Key = T;
        static_assert(std::same_as<typename X::value_type, typename X::key_type>); // [associative.reqmts.general]/5
        static_assert(std::same_as<typename X::key_type, Key>);                    // [associative.reqmts.general]/9
        static_assert(std::same_as<typename X::value_type, Key>);                  // [associative.reqmts.general]/12
        static_assert(std::destructible<Key>);                                     // [associative.reqmts.general]/13

        using Compare = std::less<Key>;
        static_assert(std::same_as<Compare, typename X::key_compare>);   // [associative.reqmts.general]/14
        static_assert(std::copy_constructible<Compare>);                 // [associative.reqmts.general]/15
        static_assert(std::same_as<Compare, typename X::value_compare>); // [associative.reqmts.general]/16

        // Constant iterators: a key can be read through either, and written through neither.
        static_assert(std::bidirectional_iterator<typename X::iterator>);             // [associative.reqmts.general]/6
        static_assert(not std::indirectly_writable<typename X::iterator, Key>);       // [associative.reqmts.general]/6
        static_assert(not std::indirectly_writable<typename X::const_iterator, Key>); // [associative.reqmts.general]/6
}

// Two comparators order alike where they agree on a pair either way round and on a key against itself.
template<class Compare>
[[nodiscard]] constexpr auto same_order(Compare const& lhs, Compare const& rhs)
        -> bool
{
        return lhs(0UZ, 1UZ) == rhs(0UZ, 1UZ) and lhs(1UZ, 0UZ) == rhs(1UZ, 0UZ) and lhs(1UZ, 1UZ) == rhs(1UZ, 1UZ);
}

template<class X>
struct constructor
{
        auto operator()() const
        {
                X u;
                X u1 = X();
                BOOST_CHECK(u.empty());                                             // [container.reqmts]/10
                BOOST_CHECK(u1.empty());                                            // [container.reqmts]/10
                static_assert(std::default_initializable<typename X::key_compare>); // [associative.reqmts.general]/20
        }

        auto operator()(X const& a) const
        {
                X u(a);               // NOLINT(performance-unnecessary-copy-initialization): the copy is what is under test
                X u1 = a;             // NOLINT(performance-unnecessary-copy-initialization): the copy is what is under test
                BOOST_CHECK(u == a);  // [container.reqmts]/13
                BOOST_CHECK(u1 == a); // [container.reqmts]/13
        }

        auto operator()(X&& rv) const
        {
                X u(rv);
                X u1 = rv;
                BOOST_CHECK(u == rv);  // [container.reqmts]/13
                BOOST_CHECK(u1 == rv); // [container.reqmts]/13
        }

        template<std::input_iterator I>
        auto operator()(I i, I j) const
        {
                static_assert(std::default_initializable<typename X::key_compare>); // [associative.reqmts.general]/26
                X u(i, j);
                X u1;
                u1.insert(i, j);
                BOOST_CHECK(u == u1); // [associative.reqmts.general]/27
        }

        template<std::ranges::input_range R>
        auto operator()(std::from_range_t, R&& rg) const
        {
                static_assert(std::default_initializable<typename X::key_compare>); // [associative.reqmts.general]/32
                X u(std::from_range, rg);
                X u1;
                u1.insert(std::ranges::begin(rg), std::ranges::end(rg));
                BOOST_CHECK(u == u1); // [associative.reqmts.general]/33
        }

        auto operator()(std::initializer_list<typename X::value_type> il) const
        {
                X u(il);
                X u1(il.begin(), il.end());
                BOOST_CHECK(u == u1); // [associative.reqmts.general]/36
        }

        // The forms taking a comparison object, which std::less makes observable only by what it answers.
        auto operator()(X::key_compare const& c) const
        {
                X u(c);
                BOOST_CHECK(u.empty() and same_order(u.key_comp(), c)); // [associative.reqmts.general]/18
        }

        template<std::input_iterator I>
        auto operator()(I i, I j, X::key_compare const& c) const
        {
                static_assert(std::constructible_from<typename X::value_type, std::iter_reference_t<I>>); // [associative.reqmts.general]/23
                X u(i, j, c);
                X u1(c);
                u1.insert(i, j);
                BOOST_CHECK(u == u1); // [associative.reqmts.general]/24
        }

        template<std::ranges::input_range R>
        auto operator()(std::from_range_t, R&& rg, X::key_compare const& c) const
        {
                static_assert(std::constructible_from<typename X::value_type, std::ranges::range_reference_t<R>>); // [associative.reqmts.general]/29
                X u(std::from_range, rg, c);
                X u1(c);
                u1.insert(std::ranges::begin(rg), std::ranges::end(rg));
                BOOST_CHECK(u == u1); // [associative.reqmts.general]/30
        }

        auto operator()(std::initializer_list<typename X::value_type> il, X::key_compare const& c) const
        {
                X u(il, c);
                X u1(il.begin(), il.end(), c);
                BOOST_CHECK(u == u1); // [associative.reqmts.general]/35
        }
};

struct op_assign
{
        template<class X>
        auto operator()(X& r, X const& a) const
        {
                r = a;
                static_assert(std::same_as<decltype(r), X&>); // [container.reqmts]/17
                BOOST_CHECK(r == a);                          // [container.reqmts]/18
        }

        template<class X>
        auto operator()(X& a, std::initializer_list<typename X::value_type> il) const
        {
                static_assert(std::same_as<decltype(a = il), X&>);                                                                                               // [associative.reqmts.general]/37
                static_assert(std::copy_constructible<typename X::value_type> and std::assignable_from<typename X::value_type&, typename X::value_type const&>); // [associative.reqmts.general]/38
                a = il;
                X a1(il);
                BOOST_CHECK(a == a1); // [associative.reqmts.general]/39
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
                static_assert(std::same_as<decltype(a.begin()), I>); // [container.reqmts]/27
                static_assert(std::same_as<decltype(a.end()), I>);   // [container.reqmts]/30

                using R = X::reverse_iterator;
                static_assert(std::same_as<decltype(a.rbegin()), R>);        // [container.rev.reqmts]/4
                BOOST_CHECK((a.rbegin() == std::reverse_iterator(a.end()))); // [container.rev.reqmts]/5

                static_assert(std::same_as<decltype(a.rend()), R>);          // [container.rev.reqmts]/7
                BOOST_CHECK((a.rend() == std::reverse_iterator(a.begin()))); // [container.rev.reqmts]/8
        }

        template<class X>
        auto operator()(const X& a) const noexcept
        {
                using I = X::const_iterator;
                static_assert(std::same_as<decltype(a.begin()), I>); // [container.reqmts]/27
                static_assert(std::same_as<decltype(a.end()), I>);   // [container.reqmts]/30

                static_assert(std::same_as<decltype(a.cbegin()), I>);       // [container.reqmts]/33
                BOOST_CHECK(a.cbegin() == const_cast<X const&>(a).begin()); // [container.reqmts]/34

                static_assert(std::same_as<decltype(a.cend()), I>);     // [container.reqmts]/36
                BOOST_CHECK(a.cend() == const_cast<X const&>(a).end()); // [container.reqmts]/37

                using R = X::const_reverse_iterator;
                static_assert(std::same_as<decltype(a.rbegin()), R>);        // [container.rev.reqmts]/4
                BOOST_CHECK((a.rbegin() == std::reverse_iterator(a.end()))); // [container.rev.reqmts]/5

                static_assert(std::same_as<decltype(a.rend()), R>);          // [container.rev.reqmts]/7
                BOOST_CHECK((a.rend() == std::reverse_iterator(a.begin()))); // [container.rev.reqmts]/8

                static_assert(std::same_as<decltype(a.crbegin()), R>);        // [container.rev.reqmts]/10
                BOOST_CHECK(a.crbegin() == const_cast<X const&>(a).rbegin()); // [container.rev.reqmts]/11

                static_assert(std::same_as<decltype(a.crend()), R>);      // [container.rev.reqmts]/13
                BOOST_CHECK(a.crend() == const_cast<X const&>(a).rend()); // [container.rev.reqmts]/14
        }

        template<std::input_iterator I>
        auto operator()(I i, I j) const noexcept
        {
                if constexpr (std::random_access_iterator<I>) {                               // [container.reqmts]/40
                        static_assert(std::same_as<decltype(i <=> j), std::strong_ordering>); // [container.reqmts]/39
                }
        }
};

struct op_equal_to
{
        template<class X, class Key = X::value_type>
        auto operator()(const X& a, const X& b) const noexcept
        {
                static_assert(std::equality_comparable<Key>);
                static_assert(std::convertible_to<decltype(a == b), bool>);       // [container.reqmts]/43
                BOOST_CHECK_EQUAL(a == b, std::ranges::equal(a, b));              // [container.reqmts]/44
                static_assert(std::equivalence_relation<std::equal_to<X>, X, X>); // [container.reqmts]/46
        }
};

// Equal values hash equal wherever a std::hash exists: the set adaptor has one as std::string_view does, std::set none.
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
                BOOST_CHECK_EQUAL(a != b, not(a == b)); // [container.reqmts]/47
        }
};

struct mem_swap
{
        auto operator()(auto& a, auto& b) const noexcept
        {
                static_assert(std::same_as<decltype(a.swap(b)), void>); // [container.reqmts]/48
                auto a1 = a;
                auto b1 = b;
                a1.swap(b1);
                BOOST_CHECK(a1 == b and b1 == a); // [container.reqmts]/49
        }
};

struct fn_swap
{
        auto operator()(auto& a, auto& b) const noexcept
        {
                auto a1 = a;
                auto b1 = b;
                auto a2 = a;
                auto b2 = b;
                swap(a1, b1);
                a2.swap(b2);
                BOOST_CHECK(a1 == a2 and b1 == b2); // [container.reqmts]/51
        }
};

struct mem_size
{
        template<class X>
        auto operator()(const X& a) const noexcept
        {
                static_assert(std::same_as<decltype(a.size()), typename X::size_type>);                      // [container.reqmts]/52
                BOOST_CHECK_EQUAL(static_cast<std::ptrdiff_t>(a.size()), std::distance(a.begin(), a.end())); // [container.reqmts]/53
        }
};

struct mem_max_size
{
        template<class X>
        auto operator()(const X& a) const noexcept
        {
                static_assert(std::same_as<decltype(a.max_size()), typename X::size_type>); // [container.reqmts]/56
                BOOST_CHECK_LE(a.size(), a.max_size());                                     // [container.reqmts]/57
        }
};

struct mem_empty
{
        auto operator()(auto const& a) const noexcept
        {
                static_assert(std::convertible_to<decltype(a.empty()), bool>); // [container.reqmts]/59
                BOOST_CHECK_EQUAL(a.empty(), a.begin() == a.end());            // [container.reqmts]/60
        }
};

struct mem_front
{
        template<class X, class R = X::const_reference>
        auto operator()(const X& a [[maybe_unused]]) const noexcept
        {
                if constexpr (requires { a.front(); }) {
                        static_assert(std::same_as<decltype(a.front()), R>); // [sequence.reqmts]/71
                        BOOST_CHECK(a.empty() or (a.front() == *a.begin())); // [sequence.reqmts]/73
                }
        }
};

struct mem_back
{
        template<class X, class R = X::const_reference>
        auto operator()(const X& a [[maybe_unused]]) const noexcept
        {
                if constexpr (requires { a.back(); }) {
                        static_assert(std::same_as<decltype(a.back()), R>);          // [sequence.reqmts]/75
                        BOOST_CHECK(a.empty() or (a.back() == *std::prev(a.end()))); // [sequence.reqmts]/77
                }
        }
};

struct mem_emplace
{
        template<class X, class... Args>
        auto operator()(X& a, Args&&... args) const
        { // [associative.reqmts.general]/47
                static_assert(
                        std::same_as<
                                decltype(a.emplace(std::forward<Args>(args)...)),
                                std::pair<typename X::iterator, bool>>
                );

                static_assert(std::constructible_from<typename X::value_type, Args...>); // [associative.reqmts.general]/48
                // Built once and then used three times.
                auto const value = typename X::value_type(std::forward<Args>(args)...);
                auto const emplaced = not a.contains(value);
                auto const r = a.emplace(value); // [associative.reqmts.general]/49
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
                        std::same_as< // [associative.reqmts.general]/57
                                decltype(a.emplace_hint(p, std::forward<Args>(args)...)), typename X::iterator>
                );
                // Built once, for the reason mem_emplace gives.
                auto const value = typename X::value_type(std::forward<Args>(args)...);
                auto const r = a.emplace_hint(p, value); // [associative.reqmts.general]/58
                BOOST_CHECK(r == a.find(value));         // [associative.reqmts.general]/59
        }
};

struct mem_insert
{
        template<class X>
        auto operator()(X& a, X::value_type const& t) const
        {
                static_assert(std::same_as<decltype(a.insert(t)), std::pair<typename X::iterator, bool>>); // [associative.reqmts.general]/61
                static_assert(std::constructible_from<typename X::value_type, decltype(t)>);               // [associative.reqmts.general]/62
                auto const inserted = not a.contains(t);
                auto const r = a.insert(t);                            // [associative.reqmts.general]/63
                BOOST_CHECK(r == std::make_pair(a.find(t), inserted)); // [associative.reqmts.general]/64
                BOOST_CHECK_EQUAL(a.count(t), 1UZ);                    // [associative.reqmts.general]/4
        }

        template<class X>
        auto operator()(X& a, X::value_type&& t) const
        { // [associative.reqmts.general]/61
                static_assert(std::same_as<decltype(a.insert(std::move(t))), std::pair<typename X::iterator, bool>>);
                // [associative.reqmts.general]/62
                static_assert(std::constructible_from<typename X::value_type, decltype(std::move(t))>);
                auto const key = t;
                auto const inserted = not a.contains(key);
                auto const r = a.insert(std::move(t));                   // [associative.reqmts.general]/63
                BOOST_CHECK(r == std::make_pair(a.find(key), inserted)); // [associative.reqmts.general]/64
                BOOST_CHECK_EQUAL(a.count(key), 1UZ);                    // [associative.reqmts.general]/4
        }

        template<class X>
        auto operator()(X& a, X::iterator p, X::value_type const& t) const
        {
                auto const size = a.size();
                auto const inserted = not a.contains(t);
                auto r = a.insert(p, t);                                                  // NOLINT(misc-const-correctness): the next line asserts decltype(r), so const would break the assertion this exists to make
                static_assert(std::same_as<decltype(r), typename X::iterator>);           // [associative.reqmts.general]/70
                static_assert(requires { a.insert(p, t); });                              // [associative.reqmts.general]/71
                BOOST_CHECK(a.contains(t) and a.size() == size + (inserted ? 1UZ : 0UZ)); // [associative.reqmts.general]/72
                BOOST_CHECK(r == a.find(t));                                              // [associative.reqmts.general]/73
        }

        template<class X>
        auto operator()(X& a, X::iterator p, X::value_type&& t) const
        {
                auto const key = t;
                auto const size = a.size();
                auto const inserted = not a.contains(key);
                auto r = a.insert(p, std::move(t));                                         // NOLINT(misc-const-correctness): the next line asserts decltype(r), so const would break the assertion this exists to make
                static_assert(std::same_as<decltype(r), typename X::iterator>);             // [associative.reqmts.general]/70
                static_assert(requires { a.insert(p, std::move(t)); });                     // [associative.reqmts.general]/71
                BOOST_CHECK(a.contains(key) and a.size() == size + (inserted ? 1UZ : 0UZ)); // [associative.reqmts.general]/72
                BOOST_CHECK(r == a.find(key));                                              // [associative.reqmts.general]/73
        }

        template<class X, std::input_iterator I>
        auto operator()(X& a, I i, I j) const
        {
                static_assert(std::same_as<decltype(a.insert(i, j)), void>); // [associative.reqmts.general]/75
                                                                             // [associative.reqmts.general]/76
                static_assert(std::constructible_from<typename X::value_type, decltype(*i)>);
                auto a1 = a; // NOLINT(misc-const-correctness): a view writes through const where an owner does not
                a.insert(i, j);
                for (auto const& t : std::ranges::subrange(i, j)) {
                        a1.insert(t);
                }
                BOOST_CHECK(a == a1); // [associative.reqmts.general]/77
        }

        template<class X, std::ranges::input_range R>
        auto operator()(X& a, R&& rg) const
        { // [associative.reqmts.general]/79
                static_assert(std::same_as<decltype(a.insert_range(rg)), void>);
                // [associative.reqmts.general]/80
                static_assert(std::constructible_from<typename X::value_type, decltype(*rg.begin())>);
                auto a1 = a; // NOLINT(misc-const-correctness): a view writes through const where an owner does not
                a.insert_range(rg);
                for (auto const& t : rg) {
                        a1.insert(t);
                }
                BOOST_CHECK(a == a1); // [associative.reqmts.general]/81
        }

        template<class X>
        auto operator()(X& a, std::initializer_list<typename X::value_type> il) const
        {
                auto a1 = a; // NOLINT(misc-const-correctness): a view writes through const where an owner does not
                a.insert(il);
                a1.insert(il.begin(), il.end());
                BOOST_CHECK(a == a1); // [associative.reqmts.general]/83
        }
};

struct mem_erase
{
        template<class X>
        auto operator()(X& a, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(a.erase(k)), typename X::size_type>); // [associative.reqmts.general]/118
                auto const erased = a.count(k);
                auto const returns = a.erase(k); // [associative.reqmts.general]/119
                BOOST_CHECK(returns == erased);  // [associative.reqmts.general]/120
        }

        template<class X>
        auto operator()(X& a, X::const_iterator q) const
        {
                static_assert(std::same_as<decltype(a.erase(q)), typename X::iterator>); // [associative.reqmts.general]/126
                BOOST_CHECK(q != a.end());
                auto const expected = std::next(q); // assumes erase does not invalidate iterators
                auto const returns = a.erase(q);    // [associative.reqmts.general]/127
                BOOST_CHECK(returns == expected);   // [associative.reqmts.general]/128
        }

        template<class X>
        auto operator()(X& a, X::const_iterator q1, X::const_iterator q2) const
        {
                static_assert(std::same_as<decltype(a.erase(q1, q2)), typename X::iterator>); // [associative.reqmts.general]/134
                auto const expected = q2;                                                     // assumes erase does not invalidate iterators
                auto const returns = a.erase(q1, q2);                                         // [associative.reqmts.general]/135
                BOOST_CHECK(returns == expected);                                             // [associative.reqmts.general]/136
        }
};

struct mem_clear
{
        auto operator()(auto& a) const noexcept
        {
                auto a1 = a; // NOLINT(misc-const-correctness): a view writes through const where an owner does not
                a.clear();
                a1.erase(a1.begin(), a1.end());
                BOOST_CHECK(a == a1);   // [associative.reqmts.general]/138
                BOOST_CHECK(a.empty()); // [associative.reqmts.general]/139
        }
};

// What std::erase_if removes from a std::set of the same keys, counted the same.
struct fn_erase_if
{
        template<class X, class Predicate>
        auto operator()(X const& c, Predicate pred) const
        {
                auto c1 = c;
                auto model = std::set<typename X::key_type>(c.begin(), c.end());
                static_assert(std::same_as<decltype(erase_if(c1, pred)), typename X::size_type>);
                auto const expected = std::erase_if(model, pred);
                auto const erased = erase_if(c1, pred);
                BOOST_CHECK_EQUAL(erased, expected);        // [set.erasure]/1
                BOOST_CHECK(std::ranges::equal(c1, model)); // [set.erasure]/1
        }
};

struct mem_find
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.find(k)), typename X::iterator>); // [associative.reqmts.general]/141
                BOOST_CHECK(b.find(k) == std::ranges::find(b, k));                      // [associative.reqmts.general]/142
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.find(k)), typename X::const_iterator>); // [associative.reqmts.general]/141
                BOOST_CHECK(b.find(k) == std::ranges::find(b, k));                            // [associative.reqmts.general]/142
        }
};

struct mem_count
{
        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.count(k)), typename X::size_type>);             // [associative.reqmts.general]/147
                BOOST_CHECK_EQUAL(static_cast<std::ptrdiff_t>(b.count(k)), std::ranges::count(b, k)); // [associative.reqmts.general]/148
        }
};

struct mem_contains
{
        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.contains(k)), bool>); // [associative.reqmts.general]/153
                BOOST_CHECK_EQUAL(b.contains(k), b.find(k) != b.end());     // [associative.reqmts.general]/154
        }
};

struct mem_lower_bound
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.lower_bound(k)), typename X::iterator>); // [associative.reqmts.general]/157
                BOOST_CHECK(b.lower_bound(k) == std::ranges::lower_bound(b, k));               // [associative.reqmts.general]/158
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.lower_bound(k)), typename X::const_iterator>); // [associative.reqmts.general]/157
                BOOST_CHECK(b.lower_bound(k) == std::ranges::lower_bound(b, k));                     // [associative.reqmts.general]/158
        }
};

struct mem_upper_bound
{
        template<class X>
        auto operator()(X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.upper_bound(k)), typename X::iterator>); // [associative.reqmts.general]/163
                BOOST_CHECK(b.upper_bound(k) == std::ranges::upper_bound(b, k));               // [associative.reqmts.general]/164
        }

        template<class X>
        auto operator()(const X& b, X::key_type const& k) const
        {
                static_assert(std::same_as<decltype(b.upper_bound(k)), typename X::const_iterator>); // [associative.reqmts.general]/163
                BOOST_CHECK(b.upper_bound(k) == std::ranges::upper_bound(b, k));                     // [associative.reqmts.general]/164
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
                static_assert(std::same_as<decltype(b.equal_range(k)), std::pair<const_iterator, const_iterator>>); // [associative.reqmts.general]/169
                BOOST_CHECK(b.equal_range(k) == std::make_pair(b.lower_bound(k), b.upper_bound(k)));                // [associative.reqmts.general]/170
        }
};

// a.erase(r): the erase of a.erase(q) through the iterator that is not necessarily constant.
struct mem_erase_mutable
{
        template<class X>
        auto operator()(X& a, X::iterator r) const
        {
                static_assert(std::same_as<decltype(a.erase(r)), typename X::iterator>); // [associative.reqmts.general]/130
                // What follows r is compared by key, a vector-backed set moving its elements up as it erases.
                auto const key = static_cast<X::key_type>(*r);
                auto const last = std::next(r) == a.end();
                auto const following = last ? key : static_cast<X::key_type>(*std::next(r));
                auto const returns = a.erase(r);
                BOOST_CHECK(not a.contains(key));                                                      // [associative.reqmts.general]/131
                BOOST_CHECK(last ? returns == a.end() : returns != a.end() and *returns == following); // [associative.reqmts.general]/132
        }
};

// Each key before the next under value_comp(), and never equivalent to it.
struct key_order
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto const comp = a.value_comp();
                BOOST_CHECK(std::ranges::adjacent_find(a, [&](auto&& i, auto&& j) -> bool { return comp(j, i); }) == a.end());     // [associative.reqmts.general]/177
                BOOST_CHECK(std::ranges::adjacent_find(a, [&](auto&& i, auto&& j) -> bool { return not comp(i, j); }) == a.end()); // [associative.reqmts.general]/178
        }
};

// A key that orders against std::size_t without converting to it, so only a member template can take it.
struct heterogeneous_key
{
        std::size_t value;

        [[nodiscard]] friend constexpr auto operator<=>(heterogeneous_key const& lhs, std::size_t rhs) noexcept
                -> std::strong_ordering
        {
                return lhs.value <=> rhs;
        }

        [[nodiscard]] friend constexpr auto operator==(heterogeneous_key const& lhs, std::size_t rhs) noexcept
                -> bool
        {
                return lhs.value == rhs;
        }
};

// A comparator without is_transparent leaves the heterogeneous member templates out of overload resolution.
template<class X>
constexpr auto no_heterogeneous_members()
        -> void
{
        using K = heterogeneous_key;
        static_assert(not requires { typename X::key_compare::is_transparent; });
        static_assert(not requires (X const b, K k) { b.find(k); });        // [associative.reqmts.general]/180
        static_assert(not requires (X const b, K k) { b.count(k); });       // [associative.reqmts.general]/180
        static_assert(not requires (X const b, K k) { b.contains(k); });    // [associative.reqmts.general]/180
        static_assert(not requires (X const b, K k) { b.lower_bound(k); }); // [associative.reqmts.general]/180
        static_assert(not requires (X const b, K k) { b.upper_bound(k); }); // [associative.reqmts.general]/180
        static_assert(not requires (X const b, K k) { b.equal_range(k); }); // [associative.reqmts.general]/180
        static_assert(not requires (X a, K k) { a.erase(k); });             // [associative.reqmts.general]/180
        static_assert(not requires (X a, K k) { a.extract(k); });           // [associative.reqmts.general]/180
}

// Neither clear() nor erase(k) throws, std::less having nothing to throw.
struct mem_clear_erase_nothrow
{
        template<class X>
        auto operator()(X const& a, X::key_type k) const
        {
                auto x = a;                                          // NOLINT(misc-const-correctness): a view writes through const where an owner does not
                BOOST_CHECK_NO_THROW(x.clear());                     // [associative.reqmts.except]/1
                auto y = a;                                          // NOLINT(misc-const-correctness): a view writes through const where an owner does not
                BOOST_CHECK_NO_THROW(static_cast<void>(y.erase(k))); // [associative.reqmts.except]/1
        }
};

// Neither swap throws, std::less having nothing to throw.
struct mem_swap_nothrow
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto x = a;
                auto y = b;
                BOOST_CHECK_NO_THROW(x.swap(y));  // [associative.reqmts.except]/3
                BOOST_CHECK_NO_THROW(swap(x, y)); // [associative.reqmts.except]/3
        }
};

// An insertion of one key that throws leaves the set as it was, each allocation refused in turn where there are any.
struct mem_insert_or_nothing
{
        template<class X>
        auto operator()(X const& a, X::key_type k) const
        {
                using test::container::strong_guarantee;
                BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert(k)); }));                // [associative.reqmts.except]/2
                BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.insert(x.end(), k)); }));       // [associative.reqmts.except]/2
                BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.emplace(k)); }));               // [associative.reqmts.except]/2
                BOOST_CHECK(strong_guarantee(a, [&](X& x) -> void { static_cast<void>(x.emplace_hint(x.end(), k)); })); // [associative.reqmts.except]/2
        }
};

// A comparison that counts itself, and projections that count their applications, for a Complexity bound.
struct counted
{
        std::size_t comparisons = 0;
        std::size_t projections1 = 0;
        std::size_t projections2 = 0;

        [[nodiscard]] auto comp()
        {
                return [this](std::size_t x, std::size_t y) -> bool {
                        ++comparisons;
                        return x < y;
                };
        }

        [[nodiscard]] auto proj1()
        {
                return [this](std::size_t x) -> std::size_t {
                        ++projections1;
                        return x;
                };
        }

        [[nodiscard]] auto proj2()
        {
                return [this](std::size_t x) -> std::size_t {
                        ++projections2;
                        return x;
                };
        }

#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL == 2
        // MSVC's debug-mode STL spends comparisons of its own verifying that both inputs are sorted.
        static constexpr auto library_verifies_order = true;
#else
        static constexpr auto library_verifies_order = false;
#endif

        // At most 2 * (N1 + N2) - 1, and nothing at all for two empty ranges, where the formula leaves -1.
        [[nodiscard]] static auto bound(auto const& a, auto const& b)
                -> std::size_t
        {
                if constexpr (library_verifies_order) {
                        return std::numeric_limits<std::size_t>::max();
                } else {
                        return std::max(2UZ * (a.size() + b.size()), 1UZ) - 1UZ;
                }
        }
};

// The keys of a set, in its own order, as the elements an algorithm writes out.
template<class X>
[[nodiscard]] auto keys_of(X const& a)
        -> std::vector<std::size_t>
{
        return std::vector<std::size_t>(a.begin(), a.end());
}

// includes(a, b): whether every key of b is also one of a.
struct fn_includes
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                BOOST_CHECK(std::ranges::is_sorted(a) and std::ranges::is_sorted(b)); // [includes]/2
                auto const expected = std::ranges::all_of(b, [&](auto&& k) -> bool { return a.contains(k); });
                auto n = counted();
                auto const returns = std::includes(a.begin(), a.end(), b.begin(), b.end(), n.comp());
                BOOST_CHECK_EQUAL(std::includes(a.begin(), a.end(), b.begin(), b.end()), std::includes(a.begin(), a.end(), b.begin(), b.end(), std::less())); // [includes]/1
                BOOST_CHECK_EQUAL(returns, expected);                                                                                                         // [includes]/3
                BOOST_CHECK_LE(n.comparisons, counted::bound(a, b));                                                                                          // [includes]/4
        }
};

struct fn_ranges_includes
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const expected = std::ranges::all_of(b, [&](auto&& k) -> bool { return a.contains(k); });
                auto n = counted();
                auto const returns = std::ranges::includes(a, b, n.comp(), n.proj1(), n.proj2());
                BOOST_CHECK_EQUAL(std::ranges::includes(a, b), std::ranges::includes(a, b, std::ranges::less(), std::identity(), std::identity()));       // [includes]/1
                BOOST_CHECK_EQUAL(returns, expected);                                                                                                     // [includes]/3
                BOOST_CHECK(n.comparisons <= counted::bound(a, b) and n.projections1 <= counted::bound(a, b) and n.projections2 <= counted::bound(a, b)); // [includes]/4
        }
};

// The keys of a set operation, each worked out from contains() alone.
struct expected_keys
{
        template<class X>
        [[nodiscard]] static auto set_union(X const& a, X const& b)
                -> std::vector<std::size_t>
        {
                auto result = keys_of(a);
                std::ranges::copy(b | std::views::filter([&](auto&& k) -> bool { return not a.contains(k); }), std::back_inserter(result));
                std::ranges::sort(result);
                return result;
        }

        template<class X>
        [[nodiscard]] static auto set_intersection(X const& a, X const& b)
                -> std::vector<std::size_t>
        {
                return a | std::views::filter([&](auto&& k) -> bool { return b.contains(k); }) | std::ranges::to<std::vector<std::size_t>>();
        }

        template<class X>
        [[nodiscard]] static auto set_difference(X const& a, X const& b)
                -> std::vector<std::size_t>
        {
                return a | std::views::filter([&](auto&& k) -> bool { return not b.contains(k); }) | std::ranges::to<std::vector<std::size_t>>();
        }

        template<class X>
        [[nodiscard]] static auto set_symmetric_difference(X const& a, X const& b)
                -> std::vector<std::size_t>
        {
                auto result = set_difference(a, b);
                std::ranges::copy(set_difference(b, a), std::back_inserter(result));
                std::ranges::sort(result);
                return result;
        }
};

struct fn_set_union
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::set_union(a.begin(), a.end(), b.begin(), b.end(), result.begin(), comp...), result.end());
                        return result;
                };
                BOOST_CHECK(std::ranges::is_sorted(a) and std::ranges::is_sorted(b)); // [set.union]/2
                auto const expected = expected_keys::set_union(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const last = std::set_union(a.begin(), a.end(), b.begin(), b.end(), out.begin(), n.comp());
                BOOST_CHECK(into() == into(std::less()));                                            // [set.union]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected)); // [set.union]/3
                BOOST_CHECK(last == out.begin() + std::ranges::ssize(expected));                     // [set.union]/4
                BOOST_CHECK_LE(n.comparisons, counted::bound(a, b));                                 // [set.union]/5
        }
};

struct fn_ranges_set_union
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::ranges::set_union(a, b, result.begin(), comp...).out, result.end());
                        return result;
                };
                auto const expected = expected_keys::set_union(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const [in1, in2, last] = std::ranges::set_union(a, b, out.begin(), n.comp(), n.proj1(), n.proj2());
                BOOST_CHECK(into() == into(std::ranges::less(), std::identity(), std::identity()));                                                       // [set.union]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected));                                                      // [set.union]/3
                BOOST_CHECK(in1 == a.end() and in2 == b.end() and last == out.begin() + std::ranges::ssize(expected));                                    // [set.union]/4
                BOOST_CHECK(n.comparisons <= counted::bound(a, b) and n.projections1 <= counted::bound(a, b) and n.projections2 <= counted::bound(a, b)); // [set.union]/5
        }
};

struct fn_set_intersection
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), result.begin(), comp...), result.end());
                        return result;
                };
                BOOST_CHECK(std::ranges::is_sorted(a) and std::ranges::is_sorted(b)); // [set.intersection]/2
                auto const expected = expected_keys::set_intersection(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const last = std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), out.begin(), n.comp());
                BOOST_CHECK(into() == into(std::less()));                                            // [set.intersection]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected)); // [set.intersection]/3
                BOOST_CHECK(last == out.begin() + std::ranges::ssize(expected));                     // [set.intersection]/4
                BOOST_CHECK_LE(n.comparisons, counted::bound(a, b));                                 // [set.intersection]/5
        }
};

struct fn_ranges_set_intersection
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::ranges::set_intersection(a, b, result.begin(), comp...).out, result.end());
                        return result;
                };
                auto const expected = expected_keys::set_intersection(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const [in1, in2, last] = std::ranges::set_intersection(a, b, out.begin(), n.comp(), n.proj1(), n.proj2());
                BOOST_CHECK(into() == into(std::ranges::less(), std::identity(), std::identity()));                                                       // [set.intersection]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected));                                                      // [set.intersection]/3
                BOOST_CHECK(in1 == a.end() and in2 == b.end() and last == out.begin() + std::ranges::ssize(expected));                                    // [set.intersection]/4
                BOOST_CHECK(n.comparisons <= counted::bound(a, b) and n.projections1 <= counted::bound(a, b) and n.projections2 <= counted::bound(a, b)); // [set.intersection]/5
        }
};

struct fn_set_difference
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::set_difference(a.begin(), a.end(), b.begin(), b.end(), result.begin(), comp...), result.end());
                        return result;
                };
                BOOST_CHECK(std::ranges::is_sorted(a) and std::ranges::is_sorted(b)); // [set.difference]/2
                auto const expected = expected_keys::set_difference(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const last = std::set_difference(a.begin(), a.end(), b.begin(), b.end(), out.begin(), n.comp());
                BOOST_CHECK(into() == into(std::less()));                                            // [set.difference]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected)); // [set.difference]/3
                BOOST_CHECK(last == out.begin() + std::ranges::ssize(expected));                     // [set.difference]/4
                BOOST_CHECK_LE(n.comparisons, counted::bound(a, b));                                 // [set.difference]/5
        }
};

struct fn_ranges_set_difference
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::ranges::set_difference(a, b, result.begin(), comp...).out, result.end());
                        return result;
                };
                auto const expected = expected_keys::set_difference(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const [in1, last] = std::ranges::set_difference(a, b, out.begin(), n.comp(), n.proj1(), n.proj2());
                BOOST_CHECK(into() == into(std::ranges::less(), std::identity(), std::identity()));                                                       // [set.difference]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected));                                                      // [set.difference]/3
                BOOST_CHECK(in1 == a.end() and last == out.begin() + std::ranges::ssize(expected));                                                       // [set.difference]/4
                BOOST_CHECK(n.comparisons <= counted::bound(a, b) and n.projections1 <= counted::bound(a, b) and n.projections2 <= counted::bound(a, b)); // [set.difference]/5
        }
};

struct fn_set_symmetric_difference
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(), result.begin(), comp...), result.end());
                        return result;
                };
                BOOST_CHECK(std::ranges::is_sorted(a) and std::ranges::is_sorted(b)); // [set.symmetric.difference]/2
                auto const expected = expected_keys::set_symmetric_difference(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const last = std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(), out.begin(), n.comp());
                BOOST_CHECK(into() == into(std::less()));                                            // [set.symmetric.difference]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected)); // [set.symmetric.difference]/3
                BOOST_CHECK(last == out.begin() + std::ranges::ssize(expected));                     // [set.symmetric.difference]/4
                BOOST_CHECK_LE(n.comparisons, counted::bound(a, b));                                 // [set.symmetric.difference]/5
        }
};

struct fn_ranges_set_symmetric_difference
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto const into = [&](auto... comp) -> std::vector<std::size_t> {
                        auto result = std::vector<std::size_t>(a.size() + b.size());
                        result.erase(std::ranges::set_symmetric_difference(a, b, result.begin(), comp...).out, result.end());
                        return result;
                };
                auto const expected = expected_keys::set_symmetric_difference(a, b);
                auto out = std::vector<std::size_t>(a.size() + b.size());
                auto n = counted();
                auto const [in1, in2, last] = std::ranges::set_symmetric_difference(a, b, out.begin(), n.comp(), n.proj1(), n.proj2());
                BOOST_CHECK(into() == into(std::ranges::less(), std::identity(), std::identity()));                                                       // [set.symmetric.difference]/1
                BOOST_CHECK(std::ranges::equal(std::ranges::subrange(out.begin(), last), expected));                                                      // [set.symmetric.difference]/3
                BOOST_CHECK(in1 == a.end() and in2 == b.end() and last == out.begin() + std::ranges::ssize(expected));                                    // [set.symmetric.difference]/4
                BOOST_CHECK(n.comparisons <= counted::bound(a, b) and n.projections1 <= counted::bound(a, b) and n.projections2 <= counted::bound(a, b)); // [set.symmetric.difference]/5
        }
};

struct op_compare_three_way
{
        auto operator()(auto const& a) const noexcept
        {
                // std::strong_ordering does not have output streaming operator<< required for BOOST_CHECK_EQUAL
                BOOST_CHECK((a <=> a) == std::strong_ordering::equal); // reflexive
        }

        auto operator()(auto const& a, auto const& b) const noexcept
        {
                static_assert(std::same_as<decltype(a <=> b), std::strong_ordering>);                                     // [tab:container.req]
                BOOST_CHECK((a <=> b) == std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end())); // [tab:container.opt]
        }
};

struct op_less
{
        auto operator()(auto const& a) const noexcept
        {                                // [tab:container.opt]
                BOOST_CHECK(not(a < a)); // irreflexive
        }

        auto operator()(auto const& a, auto const& b) const noexcept
        { // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a < b), bool>);
                BOOST_CHECK_EQUAL(a < b, std::is_lt(a <=> b));
                BOOST_CHECK_EQUAL(a < b, std::ranges::lexicographical_compare(a, b));
                BOOST_CHECK(not(a < b) or not(b < a)); // asymmetric
        }

        auto operator()(auto const& a, auto const& b, auto const& c) const noexcept
        {                                                   // [tab:container.opt]
                BOOST_CHECK(not(a < b and b < c) or a < c); // transitive
        }
};

struct op_greater
{
        auto operator()(auto const& a, auto const& b) const noexcept
        { // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a > b), bool>);
                BOOST_CHECK_EQUAL(a > b, std::is_gt(a <=> b));
                BOOST_CHECK_EQUAL(a > b, b < a);
        }
};

struct op_less_equal
{
        auto operator()(auto const& a, auto const& b) const noexcept
        { // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a <= b), bool>);
                BOOST_CHECK_EQUAL(a <= b, std::is_lteq(a <=> b));
                BOOST_CHECK_EQUAL(a <= b, not(b < a));
        }
};

struct op_greater_equal
{
        auto operator()(auto const& a, auto const& b) const noexcept
        { // [tab:container.opt]
                static_assert(std::convertible_to<decltype(a >= b), bool>);
                BOOST_CHECK_EQUAL(a >= b, std::is_gteq(a <=> b));
                BOOST_CHECK_EQUAL(a >= b, not(a < b));
        }
};

struct fn_iterator
{
        auto operator()(auto& c) const noexcept
        {
                BOOST_CHECK(std::begin(c) == c.begin()); // [iterator.range]/2
                BOOST_CHECK(std::end(c) == c.end());     // [iterator.range]/3

                BOOST_CHECK(std::rbegin(c) == c.rbegin()); // [iterator.range]/8
                BOOST_CHECK(std::rend(c) == c.rend());     // [iterator.range]/9
        }

        auto operator()(auto const& c) const noexcept
        {
                BOOST_CHECK(std::begin(c) == c.begin()); // [iterator.range]/2
                BOOST_CHECK(std::end(c) == c.end());     // [iterator.range]/3

                BOOST_CHECK(std::cbegin(c) == std::begin(c)); // [iterator.range]/6
                BOOST_CHECK(std::cend(c) == std::end(c));     // [iterator.range]/7

                BOOST_CHECK(std::rbegin(c) == c.rbegin()); // [iterator.range]/8
                BOOST_CHECK(std::rend(c) == c.rend());     // [iterator.range]/9

                BOOST_CHECK(std::crbegin(c) == std::rbegin(c)); // [iterator.range]/14
                BOOST_CHECK(std::crend(c) == std::rend(c));     // [iterator.range]/15
        }
};

struct fn_size
{
        auto operator()(auto const& c) const noexcept
        {
                BOOST_CHECK_EQUAL(std::size(c), c.size()); // [iterator.range]/16
        }
};

struct fn_ssize
{
        auto operator()(auto const& c) const noexcept
        {
                using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
                BOOST_CHECK_EQUAL(std::ssize(c), static_cast<R>(c.size())); // [iterator.range]/18
        }
};

struct fn_empty
{
        auto operator()(auto const& c) const noexcept
        {
                BOOST_CHECK_EQUAL(std::empty(c), c.empty()); // [iterator.range]/20
        }
};

} // namespace test::set

#endif // TEST_SET_PRIMITIVES_HPP
