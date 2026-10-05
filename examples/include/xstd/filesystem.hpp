//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_FILESYSTEM_HPP
#define XSTD_FILESYSTEM_HPP

#include <xstd/bits/bit_enum_traits.hpp> // enum_traits
#include <xstd/bits/bit_flag_set.hpp>    // bit_flag_set
#include <xstd/bits/bit_key_traits.hpp>  // bit_key_traits
#include <array>                         // array
#include <cassert>                       // assert
#include <cstddef>                       // size_t
#include <cstdint>                       // uint16_t, uint8_t
#include <filesystem>                    // perms
#include <format>                        // format_to, formatter
#include <string_view>                   // string_view
#include <utility>                       // to_underlying

// std::filesystem::perms as a flag type: the same spelling, the set vocabulary on top, and nothing else re-exported.
namespace xstd::filesystem {

// Each rank is the POSIX permission bit's position, so the block is the mode word.
enum class perm : std::uint8_t
{
        others_exec,
        others_write,
        others_read,
        group_exec,
        group_write,
        group_read,
        owner_exec,
        owner_write,
        owner_read,
        sticky_bit,
        set_gid,
        set_uid,
};

} // namespace xstd::filesystem

template<>
struct xstd::enum_traits<xstd::filesystem::perm>
{
        using enum xstd::filesystem::perm;

        static constexpr std::array values = {others_exec, others_write, others_read, group_exec, group_write, group_read, owner_exec, owner_write, owner_read, sticky_bit, set_gid, set_uid};
};

namespace xstd::filesystem {

// Sixteen bits rather than twelve, so that std::filesystem::perms::unknown survives the round trip.
class perms : public bit_flag_set<perms, perm, std::uint16_t, 16, bit_key_traits<perm>, std::filesystem::perms>
{
public:
        using bit_flag_set::bit_flag_set;

        // A class is incomplete inside its own definition, so its constants are defined constexpr after it.
        static perms const none, owner_read, owner_write, owner_exec, owner_all;
        static perms const group_read, group_write, group_exec, group_all;
        static perms const others_read, others_write, others_exec, others_all;
        static perms const all, set_uid, set_gid, sticky_bit, mask, unknown;
};

// One per name in [fs.enum.perms], with the value it specifies.
inline constexpr perms perms::none{};
inline constexpr perms perms::owner_read{perm::owner_read};
inline constexpr perms perms::owner_write{perm::owner_write};
inline constexpr perms perms::owner_exec{perm::owner_exec};
inline constexpr perms perms::owner_all = owner_read | owner_write | owner_exec;
inline constexpr perms perms::group_read{perm::group_read};
inline constexpr perms perms::group_write{perm::group_write};
inline constexpr perms perms::group_exec{perm::group_exec};
inline constexpr perms perms::group_all = group_read | group_write | group_exec;
inline constexpr perms perms::others_read{perm::others_read};
inline constexpr perms perms::others_write{perm::others_write};
inline constexpr perms perms::others_exec{perm::others_exec};
inline constexpr perms perms::others_all = others_read | others_write | others_exec;
inline constexpr perms perms::all        = owner_all | group_all | others_all;
inline constexpr perms perms::set_uid{perm::set_uid};
inline constexpr perms perms::set_gid{perm::set_gid};
inline constexpr perms perms::sticky_bit{perm::sticky_bit};
inline constexpr perms perms::mask    = all | set_uid | set_gid | sticky_bit;
inline constexpr perms perms::unknown = from_bits(0xFFFF);

} // namespace xstd::filesystem

// A perm prints its bare name, written out rather than through the base, whose debug form a range would quote.
template<>
struct std::formatter<xstd::filesystem::perm> : std::formatter<std::string_view>
{
        static constexpr std::array<std::string_view, 12> names = {"others_exec", "others_write", "others_read", "group_exec", "group_write", "group_read", "owner_exec", "owner_write", "owner_read", "sticky_bit", "set_gid", "set_uid"};

        template<class Context>
        [[nodiscard]] auto format(xstd::filesystem::perm key, Context& ctx) const
        {
                auto const rank = static_cast<std::size_t>(std::to_underlying(key));
                assert(rank < names.size());
                return std::format_to(ctx.out(), "{}", names[rank]);
        }
};

#endif // XSTD_FILESYSTEM_HPP
