//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_AT_HPP
#define XSTD_BITS_DETAIL_BIT_AT_HPP

#include <xstd/bits/detail/block_width.hpp> // block_width
#include <xstd/bits/detail/shift.hpp>       // shr
#include <cstddef>                          // size_t
#include <type_traits>                      // remove_cvref_t

namespace xstd::bits::detail {

// The bit at pos as a block holding it in bit zero: shifted and masked, never a bool converted back.
template<class Src>
[[nodiscard]] constexpr auto bit_at(Src const& src, std::size_t pos) noexcept
{
        using block_type = std::remove_cvref_t<decltype(src[0UZ])>;
        return static_cast<block_type>(shr(src[pos / block_width<block_type>], pos % block_width<block_type>) & block_type{1});
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_AT_HPP
