//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_COMPRESS_HPP
#define XSTD_BITS_BIT_BIT_COMPRESS_HPP

#include <xstd/bits/bit/detail/bit_permute.hpp>    // check_widths, compress_into, compress_low, permutable, permutes_fixed_width, zeros_over
#include <xstd/bits/detail/ownership.hpp>          // storage_access
#include <xstd/ints/bit/bit_compress.hpp>          // IWYU pragma: export; bit_compress
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <cassert>                                 // assert

// [bit.permute]'s bit_compress over two owners of one type, of any width, beside xstd-ints' over one integer.
namespace xstd {

// The positions of x that m holds, packed in order from position zero, the rest false: T in and T out, as P3104R5's.
template<class T>
        requires bits::detail::permutable<T>
[[nodiscard]] constexpr auto bit_compress(T const& x, T const& m) noexcept(bits::detail::permutes_fixed_width<T>)
        -> T
{
        bits::detail::check_widths(x, m);
        auto result = bits::detail::zeros_over(m);
        bits::detail::compress_into(bits::detail::storage_access::bits(x), bits::detail::storage_access::bits(m), bits::detail::storage_access::bits(result));
        return result;
}

// The same, as an unsigned integer: a table's index, which holds the positions m holds wherever U has the digits.
template<xstd::unsigned_integer U, class T>
        requires bits::detail::permutable<T>
[[nodiscard]] constexpr auto bit_compress(T const& x, T const& m) noexcept
        -> U
{
        assert(bits::detail::storage_access::bits(m).count() <= static_cast<std::size_t>(xstd::numeric_limits<U>::digits));
        bits::detail::check_widths(x, m);
        return bits::detail::compress_low<U>(bits::detail::storage_access::bits(x), bits::detail::storage_access::bits(m));
}

} // namespace xstd

#endif // XSTD_BITS_BIT_BIT_COMPRESS_HPP
