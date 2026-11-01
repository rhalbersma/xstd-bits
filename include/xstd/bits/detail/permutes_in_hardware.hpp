//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_PERMUTES_IN_HARDWARE_HPP
#define XSTD_BITS_DETAIL_PERMUTES_IN_HARDWARE_HPP

namespace xstd::bits::detail {

// Where xstd-ints runs bit_compress and bit_expand as PEXT and PDEP, which permute a full mask as fast as any other.
#if defined(__BMI2__) || (defined(_MSC_VER) && !defined(__clang__) && defined(__AVX2__))
inline constexpr bool permutes_in_hardware = true;
#else
inline constexpr bool permutes_in_hardware = false;
#endif

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_PERMUTES_IN_HARDWARE_HPP
