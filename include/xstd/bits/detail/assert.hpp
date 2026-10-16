//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ASSERT_HPP
#define XSTD_BITS_DETAIL_ASSERT_HPP

#include <cassert> // assert
#include <cstdlib> // IWYU pragma: keep; abort, named only in a hardened release build

// A hardened standard library checks these preconditions in a release build; a user's definition wins either way.
#ifndef XSTD_BITS_HARDENED
#ifdef _GLIBCXX_ASSERTIONS
#define XSTD_BITS_HARDENED 1
#elif defined(_LIBCPP_HARDENING_MODE) and defined(_LIBCPP_HARDENING_MODE_NONE)
#if _LIBCPP_HARDENING_MODE != _LIBCPP_HARDENING_MODE_NONE
#define XSTD_BITS_HARDENED 1
#endif
#elif defined(_MSVC_STL_HARDENING)
#if _MSVC_STL_HARDENING
#define XSTD_BITS_HARDENED 1
#endif
#endif
#endif

#ifndef XSTD_BITS_HARDENED
#define XSTD_BITS_HARDENED 0
#endif

// assert where it is on, and where only hardening is, the abort libstdc++'s assertions end in, without the message.
#if XSTD_BITS_HARDENED and defined(NDEBUG)
#define XSTD_BITS_ASSERT(cond) (static_cast<bool>(cond) ? void(0) : std::abort())
#else
#define XSTD_BITS_ASSERT(cond) assert(cond)
#endif

#endif // XSTD_BITS_DETAIL_ASSERT_HPP
