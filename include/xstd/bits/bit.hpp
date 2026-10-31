//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_HPP
#define XSTD_BITS_BIT_HPP

// [bit.cast], function template bit_cast, whose counterpart across bit storages is bit_convert
#include <xstd/bits/bit/bit_convert.hpp> // IWYU pragma: export; bit_convert

// [bit.pow.two], integral powers of 2, over one owner as well as over one unsigned integer
#include <xstd/bits/bit/has_single_bit.hpp> // IWYU pragma: export; has_single_bit

// [bit.rotate], rotating, beside which P3793R2's shl and shr shift a set's keys by any count
#include <xstd/bits/bit/shl.hpp> // IWYU pragma: export; shl
#include <xstd/bits/bit/shr.hpp> // IWYU pragma: export; shr

// [bit.permute], permutation, over two owners of one type as well as over one unsigned integer
#include <xstd/bits/bit/bit_compress.hpp> // IWYU pragma: export; bit_compress
#include <xstd/bits/bit/bit_expand.hpp>   // IWYU pragma: export; bit_expand

#endif // XSTD_BITS_BIT_HPP
