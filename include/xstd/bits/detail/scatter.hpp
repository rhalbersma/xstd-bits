//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SCATTER_HPP
#define XSTD_BITS_DETAIL_SCATTER_HPP

namespace xstd::bits::detail {

// How a scatter combines a destination block d with the bits e deposited under the mask m: only m's positions change.
inline constexpr auto scatter_assign = []<class Block>(Block d, Block e, Block m) noexcept -> Block { return static_cast<Block>(static_cast<Block>(d & static_cast<Block>(~m)) | e); };

inline constexpr auto scatter_and = []<class Block>(Block d, Block e, Block m) noexcept -> Block { return static_cast<Block>(d & static_cast<Block>(e | static_cast<Block>(~m))); };

inline constexpr auto scatter_or = []<class Block>(Block d, Block e, Block /* m */) noexcept -> Block { return static_cast<Block>(d | e); };

inline constexpr auto scatter_xor = []<class Block>(Block d, Block e, Block /* m */) noexcept -> Block { return static_cast<Block>(d ^ e); };

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_SCATTER_HPP
