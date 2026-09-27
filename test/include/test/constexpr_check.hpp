//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_CONSTEXPR_CHECK_HPP
#define TEST_CONSTEXPR_CHECK_HPP

#include <boost/test/unit_test.hpp> // BOOST_CHECK, BOOST_CHECK_EQUAL

// No STATIC_REQUIRE in Boost.Test, so one expression becomes both checks and cannot drift.
#define XSTD_CONSTEXPR_CHECK(...) \
        static_assert(__VA_ARGS__); \
        BOOST_CHECK(__VA_ARGS__)

#define XSTD_CONSTEXPR_CHECK_EQUAL(a, b) \
        static_assert((a) == (b)); \
        BOOST_CHECK_EQUAL((a), (b))

#define XSTD_CONSTEXPR_CHECK_EQUAL_IF(condition, a, b) \
        if constexpr (condition) { \
                static_assert((a) == (b)); \
        } \
        BOOST_CHECK_EQUAL((a), (b))

#endif // TEST_CONSTEXPR_CHECK_HPP
