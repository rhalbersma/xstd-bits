//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SANITIZER_HPP
#define TEST_SANITIZER_HPP

// AddressSanitizer answers an allocation it will not serve by ABORTING, where the C++ allocator answers with
// std::bad_alloc. So an assertion that a width past what the blocks can hold reaches the allocator cannot be
// made on a sanitized build: the process is gone before the catch. Measured on this tree -- the report is
// "allocation-size-too-big", and allocator_may_return_null=1 only renames it to "out-of-memory", because the
// THROWING operator new calls ReportOutOfMemory on a null return rather than throwing.
//
// Every other leg of this ladder answers those rows, which is measured too: the nine failures that led here
// were all sanitized builds or a discarded temporary an optimizer elided; the msvc and mingw legs, which are
// neither, never failed on them at all. So the rows are guarded on this and on nothing else.
#if defined(__SANITIZE_ADDRESS__)

#define TEST_HAS_ADDRESS_SANITIZER

#elif defined(__has_feature)
#if __has_feature(address_sanitizer)

#define TEST_HAS_ADDRESS_SANITIZER

#endif
#endif

namespace test {

#ifdef TEST_HAS_ADDRESS_SANITIZER

inline constexpr bool has_address_sanitizer = true;

#else

inline constexpr bool has_address_sanitizer = false;

#endif

}       // namespace test

#endif  // TEST_SANITIZER_HPP
