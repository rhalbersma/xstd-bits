//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_CONTAINER_PRIMITIVES_HPP
#define TEST_CONTAINER_PRIMITIVES_HPP

#include <test/reference.hpp>        // proxy_reference
#include <test/sequence/factory.hpp> // static_width
#include <boost/test/unit_test.hpp>  // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_LE, BOOST_CHECK_NO_THROW
#include <algorithm>                 // equal, lexicographical_compare_three_way, min_element
#include <compare>                   // is_eq, strong_ordering
#include <concepts>                  // convertible_to, destructible, same_as, signed_integral, unsigned_integral
#include <cstddef>                   // ptrdiff_t, size_t
#include <iterator>                  // bidirectional_iterator, distance, forward_iterator, indirectly_writable, iter_difference_t, iter_value_t, next, random_access_iterator, reverse_iterator
#include <limits>                    // numeric_limits
#include <type_traits>               // make_unsigned_t
#include <utility>                   // as_const, declval, move

// The [container.requirements] primitives, written once for the set and the sequence reading alike.
namespace test::container {

// A set names the key it holds, where a sequence holds bools by position.
template<class X>
concept keyed = requires { typename X::key_type; };

// A sequence whose width is its type's, which [array.overview]/3 lets be non-empty when default constructed.
template<class X>
concept fixed_width = not keyed<X> and test::sequence::static_width<X>;

// The element an iteration reaches first: a set's least key under its key_compare, a sequence's position zero.
template<class X>
[[nodiscard]] auto first_element(X const& a)
        -> X::value_type
{
        if constexpr (keyed<X>) {
                return *std::ranges::min_element(a, a.key_comp());
        } else {
                return a[0];
        }
}

// The types [container.reqmts] and [container.rev.reqmts] ask of every container, whatever it holds.
template<class X>
constexpr auto nested_types()
        -> void
{
        using T  = X::value_type;
        using I  = X::iterator;
        using CI = X::const_iterator;
        using D  = X::difference_type;
        using S  = X::size_type;

        static_assert(std::same_as<std::iter_value_t<I>, T>);                                                                // [container.reqmts]/2
        static_assert(std::same_as<typename X::reference, T&> or test::proxy_reference<X>);                                  // [container.reqmts]/4
        static_assert(std::same_as<typename X::const_reference, T const&> or test::proxy_reference<X>);                      // [container.reqmts]/5
        static_assert(std::forward_iterator<I> and std::convertible_to<I, CI>);                                              // [container.reqmts]/6
        static_assert(std::forward_iterator<CI> and std::same_as<std::iter_value_t<CI>, T>);                                 // [container.reqmts]/7
        static_assert(not std::indirectly_writable<CI, T>);                                                                  // [container.reqmts]/7
        static_assert(std::signed_integral<D> and std::same_as<D, std::iter_difference_t<I>>);                               // [container.reqmts]/8
        static_assert(std::same_as<D, std::iter_difference_t<CI>>);                                                          // [container.reqmts]/8
        static_assert(std::unsigned_integral<S>);                                                                            // [container.reqmts]/9
        static_assert(std::numeric_limits<S>::max() >= static_cast<std::make_unsigned_t<D>>(std::numeric_limits<D>::max())); // [container.reqmts]/9

        static_assert(std::bidirectional_iterator<I> and std::bidirectional_iterator<CI>);          // [container.rev.reqmts]/1
        static_assert(std::same_as<typename X::reverse_iterator, std::reverse_iterator<I>>);        // [container.rev.reqmts]/2
        static_assert(std::same_as<std::iter_value_t<typename X::reverse_iterator>, T>);            // [container.rev.reqmts]/2
        static_assert(std::same_as<typename X::const_reverse_iterator, std::reverse_iterator<CI>>); // [container.rev.reqmts]/3
        static_assert(not std::indirectly_writable<typename X::const_reverse_iterator, T>);         // [container.rev.reqmts]/3
}

template<class X>
struct constructor_default
{
        auto operator()() const
        {
                X u;
                auto const u1 = X();
                if constexpr (fixed_width<X>) {
                        // Default-initialized elements are indeterminate, so only the width is read.
                        BOOST_CHECK_EQUAL(u.size(), u1.max_size());  // [array.overview]/3
                        BOOST_CHECK_EQUAL(u1.size(), u1.max_size()); // [array.overview]/3
                } else {
                        BOOST_CHECK(u.empty() and u1.empty()); // [container.reqmts]/10
                }
        }
};

struct constructor_copy
{
        template<class X>
        auto operator()(X const& v) const
        {
                X u(v);                          // NOLINT(performance-unnecessary-copy-initialization): the copy is what is under test
                X u1 = v;                        // NOLINT(performance-unnecessary-copy-initialization): the copy is what is under test
                BOOST_CHECK(u == v and u1 == v); // [container.reqmts]/13
        }
};

struct constructor_move
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto rv  = a;
                auto rv1 = a;
                X u(std::move(rv));
                X u1 = std::move(rv1);
                BOOST_CHECK(u == a and u1 == a); // [container.reqmts]/15
        }
};

struct op_copy_assign
{
        template<class X>
        auto operator()(X const& t0, X const& v) const
        {
                auto t = t0;
                static_assert(std::same_as<decltype(t = v), X&>); // [container.reqmts]/17
                t = v;
                BOOST_CHECK(t == v); // [container.reqmts]/18
        }
};

struct op_move_assign
{
        template<class X>
        auto operator()(X const& t0, X const& v) const
        {
                auto t  = t0;
                auto rv = v;
                static_assert(std::same_as<decltype(t = std::move(rv)), X&>); // [container.reqmts]/20
                t = std::move(rv);
                BOOST_CHECK_EQUAL(t.size(), v.size()); // [container.reqmts]/21
                BOOST_CHECK(t == v);                   // [container.reqmts]/22
        }
};

template<class X>
constexpr auto destructor()
        -> void
{
#if defined(_MSC_VER) && !defined(__clang__)
        // MSVC mis-types an explicit destructor call named through a template parameter, so only destructibility is asked.
        static_assert(std::destructible<X>); // [container.reqmts]/24
#else
        static_assert(std::same_as<decltype(std::declval<X&>().~X()), void>); // [container.reqmts]/24
#endif
}

// begin() and end() on a non-const object, which hand out the mutable iterator.
struct mem_begin_end
{
        template<class X>
        auto operator()(X& a) const
        {
                static_assert(std::same_as<decltype(a.begin()), typename X::iterator>); // [container.reqmts]/27
                static_assert(std::same_as<decltype(a.end()), typename X::iterator>);   // [container.reqmts]/30
                if (not a.empty()) {
                        BOOST_CHECK(*a.begin() == first_element(a)); // [container.reqmts]/28
                }
                BOOST_CHECK(std::next(a.begin(), static_cast<std::ptrdiff_t>(a.size())) == a.end()); // [container.reqmts]/31
        }

        template<class X>
        auto operator()(X const& a) const
        {
                static_assert(std::same_as<decltype(a.begin()), typename X::const_iterator>); // [container.reqmts]/27
                static_assert(std::same_as<decltype(a.end()), typename X::const_iterator>);   // [container.reqmts]/30
                if (not a.empty()) {
                        BOOST_CHECK(*a.begin() == first_element(a)); // [container.reqmts]/28
                }
                BOOST_CHECK(std::next(a.begin(), static_cast<std::ptrdiff_t>(a.size())) == a.end()); // [container.reqmts]/31
        }
};

struct mem_cbegin_cend
{
        template<class X>
        auto operator()(X const& b) const
        {
                auto x = b;
                static_assert(std::same_as<decltype(x.cbegin()), typename X::const_iterator>); // [container.reqmts]/33
                BOOST_CHECK(x.cbegin() == std::as_const(x).begin());                           // [container.reqmts]/34
                static_assert(std::same_as<decltype(x.cend()), typename X::const_iterator>);   // [container.reqmts]/36
                BOOST_CHECK(x.cend() == std::as_const(x).end());                               // [container.reqmts]/37
        }
};

// The three-way comparison of two iterators exists for random access alone, and an iterator meets a const one.
struct op_iterator_compare
{
        template<class X>
        auto operator()(X const& b) const
        {
                using I       = X::iterator;
                using CI      = X::const_iterator;
                auto a        = b;
                auto const i  = a.begin();
                auto const ci = std::as_const(a).begin();
                if constexpr (std::random_access_iterator<I>) {
                        static_assert(std::same_as<decltype(i <=> i), std::strong_ordering>); // [container.reqmts]/39
                        // NOLINTNEXTLINE(modernize-use-nullptr): a mixed <=> is the reversed candidate, compared against a literal 0
                        BOOST_CHECK(std::is_eq(i <=> ci) and std::is_eq(ci <=> i));       // [container.reqmts]/63
                        BOOST_CHECK(not(i < ci) and i <= ci and i >= ci and not(i > ci)); // [container.reqmts]/63
                        BOOST_CHECK_EQUAL(i - ci, 0);                                     // [container.reqmts]/63
                        BOOST_CHECK_EQUAL(ci - i, 0);                                     // [container.reqmts]/63
                } else {
                        static_assert(not requires { i <=> i; }); // [container.reqmts]/40
                        static_assert(not std::random_access_iterator<CI>);
                }
                BOOST_CHECK(i == ci and ci == i and not(i != ci)); // [container.reqmts]/63
        }
};

struct op_equal_to
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                static_assert(std::same_as<decltype(a == b), bool>);                           // [container.reqmts]/43
                BOOST_CHECK_EQUAL(a == b, std::equal(a.begin(), a.end(), b.begin(), b.end())); // [container.reqmts]/44
                auto const c = b;                                                              // NOLINT(performance-unnecessary-copy-initialization): an equal object that is not b
                BOOST_CHECK(a == a and b == b);                                                // [container.reqmts]/46
                BOOST_CHECK_EQUAL(a == b, b == a);                                             // [container.reqmts]/46
                BOOST_CHECK(not(a == b and b == c) or a == c);                                 // [container.reqmts]/46
        }
};

struct op_not_equal_to
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                BOOST_CHECK_EQUAL(a != b, not(a == b)); // [container.reqmts]/47
        }
};

struct mem_swap
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto t = a;
                auto s = b;
                static_assert(std::same_as<decltype(t.swap(s)), void>); // [container.reqmts]/48
                t.swap(s);
                BOOST_CHECK(t == b and s == a); // [container.reqmts]/49
        }
};

struct fn_swap
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto t  = a;
                auto s  = b;
                auto t1 = a;
                auto s1 = b;
                swap(t, s);
                t1.swap(s1);
                BOOST_CHECK(t == t1 and s == s1); // [container.reqmts]/51
        }
};

struct mem_size
{
        template<class X>
        auto operator()(X const& c) const
        {
                static_assert(std::same_as<decltype(c.size()), typename X::size_type>);                      // [container.reqmts]/52
                BOOST_CHECK_EQUAL(static_cast<std::ptrdiff_t>(c.size()), std::distance(c.begin(), c.end())); // [container.reqmts]/53
        }
};

struct mem_max_size
{
        template<class X>
        auto operator()(X const& c) const
        {
                static_assert(std::same_as<decltype(c.max_size()), typename X::size_type>); // [container.reqmts]/56
                BOOST_CHECK_LE(c.size(), c.max_size());                                     // [container.reqmts]/57
                if constexpr (fixed_width<X>) {
                        BOOST_CHECK_EQUAL(c.max_size(), c.size()); // [container.reqmts]/57
                }
        }
};

struct mem_empty
{
        template<class X>
        auto operator()(X const& c) const
        {
                static_assert(std::same_as<decltype(c.empty()), bool>); // [container.reqmts]/59
                BOOST_CHECK_EQUAL(c.empty(), c.begin() == c.end());     // [container.reqmts]/60
                BOOST_CHECK_EQUAL(c.empty(), c.size() == 0UZ);          // [container.reqmts]/62
        }
};

// Calling a member for what it reports moves no iterator and changes no value.
struct mem_observers
{
        template<class X>
        auto operator()(X const& b) const
        {
                auto a           = b; // NOLINT(performance-unnecessary-copy-initialization,misc-const-correctness): the members are called on a mutable object
                auto const first = a.begin();
                auto const last  = a.end();
                static_cast<void>(a.size());
                static_cast<void>(a.max_size());
                static_cast<void>(a.empty());
                static_cast<void>(a == b);
                BOOST_CHECK(first == a.begin() and last == a.end()); // [container.reqmts]/67
                BOOST_CHECK(a == b);                                 // [container.reqmts]/67
        }
};

// Erasing the first element, the last one and then all of them throws nothing.
struct mem_erasers
{
        template<class X>
        auto operator()(X const& a) const
        {
                auto b = a;
                erase_first(b);
                pop_last(b);
                BOOST_CHECK_NO_THROW(b.clear()); // [container.reqmts]/66
                BOOST_CHECK(b.empty());
        }

private:
        template<class X>
        static auto erase_first(X& b)
                -> void
        {
                if (not b.empty()) {
                        BOOST_CHECK_NO_THROW(b.erase(b.begin())); // [container.reqmts]/66
                }
        }

        template<class X>
        static auto pop_last(X& b)
                -> void
        {
                if constexpr (requires { b.pop_back(); }) {
                        if (not b.empty()) {
                                BOOST_CHECK_NO_THROW(b.pop_back()); // [container.reqmts]/66
                        }
                }
        }
};

struct mem_rbegin
{
        template<class X>
        auto operator()(X const& b) const
        {
                auto a = b;
                static_assert(std::same_as<decltype(a.rbegin()), typename X::reverse_iterator>);                      // [container.rev.reqmts]/4
                static_assert(std::same_as<decltype(std::as_const(a).rbegin()), typename X::const_reverse_iterator>); // [container.rev.reqmts]/4
                BOOST_CHECK(a.rbegin() == typename X::reverse_iterator(a.end()));                                     // [container.rev.reqmts]/5
                BOOST_CHECK(std::as_const(a).rbegin() == typename X::const_reverse_iterator(std::as_const(a).end())); // [container.rev.reqmts]/5
        }
};

struct mem_rend
{
        template<class X>
        auto operator()(X const& b) const
        {
                auto a = b;
                static_assert(std::same_as<decltype(a.rend()), typename X::reverse_iterator>);                        // [container.rev.reqmts]/7
                static_assert(std::same_as<decltype(std::as_const(a).rend()), typename X::const_reverse_iterator>);   // [container.rev.reqmts]/7
                BOOST_CHECK(a.rend() == typename X::reverse_iterator(a.begin()));                                     // [container.rev.reqmts]/8
                BOOST_CHECK(std::as_const(a).rend() == typename X::const_reverse_iterator(std::as_const(a).begin())); // [container.rev.reqmts]/8
        }
};

struct mem_crbegin
{
        template<class X>
        auto operator()(X const& b) const
        {
                auto a = b;
                static_assert(std::same_as<decltype(a.crbegin()), typename X::const_reverse_iterator>); // [container.rev.reqmts]/10
                BOOST_CHECK(a.crbegin() == std::as_const(a).rbegin());                                  // [container.rev.reqmts]/11
        }
};

struct mem_crend
{
        template<class X>
        auto operator()(X const& b) const
        {
                auto a = b;
                static_assert(std::same_as<decltype(a.crend()), typename X::const_reverse_iterator>); // [container.rev.reqmts]/13
                BOOST_CHECK(a.crend() == std::as_const(a).rend());                                    // [container.rev.reqmts]/14
        }
};

struct op_three_way
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                static_assert(std::same_as<decltype(a <=> b), std::strong_ordering>);                                     // [container.opt.reqmts]/2
                BOOST_CHECK((a <=> b) == std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end())); // [container.opt.reqmts]/4
        }
};

// The three-way comparison as a constant expression, over any container whose iterators are constexpr ones.
template<class X>
[[nodiscard]] constexpr auto compares_as_a_constant()
        -> bool
{
        return std::is_eq(X() <=> X());
}

} // namespace test::container

#endif // TEST_CONTAINER_PRIMITIVES_HPP
