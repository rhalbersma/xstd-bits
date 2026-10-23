//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_THROW_HPP
#define XSTD_BITS_DETAIL_THROW_HPP

#include <cstdio>  // IWYU pragma: keep; fputs, stderr, named only without exceptions
#include <cstdlib> // IWYU pragma: keep; abort, named only without exceptions
#include <utility> // IWYU pragma: keep; forward, named only with exceptions

// Exceptions as the compiler has them: GCC and Clang define __cpp_exceptions, and MSVC _CPPUNWIND under /EHsc.
#ifndef XSTD_BITS_HAS_EXCEPTIONS
#if defined(__cpp_exceptions) or defined(_CPPUNWIND)
#define XSTD_BITS_HAS_EXCEPTIONS 1
#else
#define XSTD_BITS_HAS_EXCEPTIONS 0
#endif
#endif

// Out of line and cold, as libstdc++'s __throw_ helpers are, so a throw site's message costs its caller no inlining.
#if defined(__GNUC__) or defined(__clang__)
#define XSTD_BITS_COLD [[gnu::cold, gnu::noinline]]
#elif defined(_MSC_VER)
#define XSTD_BITS_COLD __declspec(noinline)
#else
#define XSTD_BITS_COLD
#endif

namespace xstd::bits::detail {

// Thrown where exceptions are on; without them, the message and then abort, as libstdc++'s __throw_ helpers do.
template<class Exception>
[[noreturn]] XSTD_BITS_COLD constexpr auto throw_exception(Exception&& e)
        -> void
{
#if XSTD_BITS_HAS_EXCEPTIONS
        throw std::forward<Exception>(e);
#else
        std::fputs(e.what(), stderr);
        std::fputs("\n", stderr);
        std::abort();
#endif
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_THROW_HPP
