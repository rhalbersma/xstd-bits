//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_FILESYSTEM_HPP
#define XSTD_FILESYSTEM_HPP

#include <xstd/bits/bit_flag_set.hpp> // bit_flag_set
#include <filesystem>                 // perms

// std::filesystem::perms as a flag type: the same constants, the set vocabulary on top, and nothing else re-exported.
namespace xstd::filesystem {

// All sixteen bits rather than the twelve permissions, so that std::filesystem::perms::unknown survives the round trip.
using perms = bit_flag_set<std::filesystem::perms, 16>;

} // namespace xstd::filesystem

#endif // XSTD_FILESYSTEM_HPP
