//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SANITIZER_HPP
#define TEST_SANITIZER_HPP

// AddressSanitizer aborts on an allocation it will not serve, where the allocator throws std::bad_alloc.
#ifdef __SANITIZE_ADDRESS__

#define TEST_HAS_ADDRESS_SANITIZER

#elifdef __has_feature
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

} // namespace test

#endif // TEST_SANITIZER_HPP
