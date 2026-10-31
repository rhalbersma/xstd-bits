//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_EXPAND_HPP
#define XSTD_BITS_BIT_BIT_EXPAND_HPP

#include <xstd/bits/bit/detail/bit_permute.hpp>    // assign_low, check_widths, expand_into, permutable, permutes_fixed_width, zeros_over
#include <xstd/bits/detail/ownership.hpp>          // reads, set_reading_tag, storage_access
#include <xstd/ints/bit/bit_expand.hpp>            // IWYU pragma: export; bit_expand
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

// [bit.permute]'s bit_expand over two owners of one type, of any width, beside xstd-ints' over one integer.
namespace xstd {

// The low positions of x, one per position m holds, laid at those positions in order, the rest false.
template<class T>
        requires bits::detail::permutable<T>
[[nodiscard]] constexpr auto bit_expand(T const& x, T const& m) noexcept(bits::detail::permutes_fixed_width<T>)
        -> T
{
        bits::detail::check_widths(x, m);
        auto const& mask = bits::detail::storage_access::bits(m);
        auto result      = bits::detail::zeros_over(m);
        if constexpr (bits::detail::reads<T, bits::detail::set_reading_tag> and not bits::detail::permutes_fixed_width<T>) {
                // A narrower set's absent keys are false, read from a copy as wide as the mask.
                if (auto const& src = bits::detail::storage_access::bits(x); src.size() < mask.size()) {
                        auto wide = x;
                        bits::detail::storage_access::bits(wide).resize(mask.size());
                        bits::detail::expand_into(bits::detail::storage_access::bits(wide), mask, bits::detail::storage_access::bits(result));
                        return result;
                }
        }
        bits::detail::expand_into(bits::detail::storage_access::bits(x), mask, bits::detail::storage_access::bits(result));
        return result;
}

// An unsigned integer's low bits laid at the positions m holds: a table's index back to the subset of m it names.
template<xstd::unsigned_integer U, class T>
        requires bits::detail::permutable<T>
[[nodiscard]] constexpr auto bit_expand(U x, T const& m) noexcept(bits::detail::permutes_fixed_width<T>)
        -> T
{
        auto src = bits::detail::zeros_over(m);
        bits::detail::assign_low(x, bits::detail::storage_access::bits(src));
        return xstd::bit_expand(src, m);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_BIT_EXPAND_HPP
