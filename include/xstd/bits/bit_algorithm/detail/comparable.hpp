//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_COMPARABLE_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_COMPARABLE_HPP

#include <xstd/bits/detail/key_order.hpp> // same_keys
#include <xstd/bits/detail/ownership.hpp> // reads_bools, storage_access
#include <cstddef>                        // size_t
#include <type_traits>                    // remove_cvref_t
#include <utility>                        // declval

namespace xstd::bits::detail {

// The width a type fixes, or dynamic_extent where it is a run-time value: a window's own, else its storage's.
template<class R>
[[nodiscard]] consteval auto static_width() noexcept
        -> std::size_t
{
        if constexpr (requires { std::remove_cvref_t<R>::static_extent; }) {
                return std::remove_cvref_t<R>::static_extent;
        } else {
                return std::remove_cvref_t<decltype(storage_access::bits(std::declval<R const&>()))>::extent;
        }
}

// Two of ours of one reading that bit_hash keeps apart only where their values differ: one width kind, one width.
template<class R1, class R2>
concept comparable_readings = ((reads_bools<R1> and reads_bools<R2>) or same_keys<R1, R2>) and static_width<R1>() == static_width<R2>();

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_COMPARABLE_HPP
