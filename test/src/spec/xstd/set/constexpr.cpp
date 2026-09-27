//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/spec/set.hpp>        // fixed_every_width
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <compare>                  // strong_ordering

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Constexpr)

// std::set is not a literal type, so there is no model, and only a width in the type makes a set one.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnEmptySetIsUsableInAConstantExpression, T, test::spec::set::fixed_every_width)
{
        constexpr auto b = T();
        static_assert(b.empty());
        static_assert(b.size() == 0);
        static_assert(b.begin() == b.end());
        // Reflexivity cannot be written without naming the object twice.
        static_assert(b == b);                                   // NOLINT(misc-redundant-expression)
        static_assert((b <=> b) == std::strong_ordering::equal); // NOLINT(misc-redundant-expression)
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AFullSetIsUsableInAConstantExpression, T, test::spec::set::fixed_every_width)
{
        constexpr auto b = ~T();
        static_assert(b.full());
        static_assert(b.size() == b.max_size());
        static_assert(b.empty() or b.front() == *b.cbegin());
        static_assert(b.empty() or b.back() == *b.crbegin());
        // Reflexivity cannot be written without naming the object twice.
        static_assert(b == b);                                   // NOLINT(misc-redundant-expression)
        static_assert((b <=> b) == std::strong_ordering::equal); // NOLINT(misc-redundant-expression)
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
