//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_OWNERSHIP_HPP
#define XSTD_BITS_OWNERSHIP_HPP

namespace xstd {

// The one template parameter owning-versus-viewing collapses to: an enum rather than a bool, so a diagnostic reads it. [design.md#ownership-is-not-an-axis]
enum class ownership : bool { refers, owns };

[[nodiscard]] constexpr auto owns(ownership o) noexcept
        -> bool
{
        return o == ownership::owns;
}

}       // namespace xstd

#endif  // XSTD_BITS_OWNERSHIP_HPP
