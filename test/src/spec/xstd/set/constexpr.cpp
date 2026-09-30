//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/exhaustive.hpp>  // static_width
#include <test/spec/set.hpp>        // owners
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <compare>                  // strong_ordering

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Constexpr)

// xstd set: constexpr X();
BOOST_AUTO_TEST_CASE(DefaultConstructor)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                // std::set is not a literal type, so there is no model, and only a width in the type makes a set one.
                if constexpr (test::set::static_width<T>) {
                        constexpr auto b = T();
                        static_assert(b.empty());
                        static_assert(b.size() == 0);
                        static_assert(b.begin() == b.end());
                        // Reflexivity cannot be written without naming the object twice.
                        static_assert(b == b);                                   // NOLINT(misc-redundant-expression)
                        static_assert((b <=> b) == std::strong_ordering::equal); // NOLINT(misc-redundant-expression)
                }
        });
}

// xstd set: constexpr X operator~(const X& lhs);
BOOST_AUTO_TEST_CASE(Complement)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                if constexpr (test::set::static_width<T>) {
                        constexpr auto b = ~T();
                        static_assert(b.full());
                        static_assert(b.size() == b.max_size());
                        static_assert(b.empty() or b.front() == *b.cbegin());
                        static_assert(b.empty() or b.back() == *b.crbegin());
                        // Reflexivity cannot be written without naming the object twice.
                        static_assert(b == b);                                   // NOLINT(misc-redundant-expression)
                        static_assert((b <=> b) == std::strong_ordering::equal); // NOLINT(misc-redundant-expression)
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
