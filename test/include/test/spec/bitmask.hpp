//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_BITMASK_HPP
#define TEST_SPEC_BITMASK_HPP

#include <test/spec/input.hpp>                          // context, edge
#include <xstd/bits/bit/bit_convert.hpp>                // bit_convert
#include <xstd/bits/bit_flag_set.hpp>                   // bit_flag_set
#include <xstd/bits/bit_type_traits/bit_underlying.hpp> // underlying_block_t
#include <array>                                        // array, to_array
#include <bitset>                                       // bitset
#include <charconv>                                     // chars_format
#include <concepts>                                     // same_as
#include <cstddef>                                      // size_t
#include <cstdint>                                      // uint64_t
#include <filesystem>                                   // copy_options, directory_options, perm_options, perms
#include <future>                                       // launch
#include <ios>                                          // ios_base
#include <limits>                                       // numeric_limits
#include <ranges>                                       // iota
#include <regex>                                        // regex_constants
#include <tuple>                                        // tuple, tuple_cat
#include <utility>                                      // declval
#include <vector>                                       // vector

// The candidates for the bitmask reading, the standard library's types first, and the values a clause checks them over.
namespace test::spec::bitmask {

// Each bitmask type the standard names, as its bitmask elements, its multi-bit constants and its empty ones.

// [ios.fmtflags]: fifteen elements, and three constants that each combine several.
struct fmtflags
{
        using type                      = std::ios_base::fmtflags;
        static constexpr auto elements  = std::to_array<type>({std::ios_base::boolalpha, std::ios_base::dec, std::ios_base::fixed, std::ios_base::hex, std::ios_base::internal, std::ios_base::left, std::ios_base::oct, std::ios_base::right, std::ios_base::scientific, std::ios_base::showbase, std::ios_base::showpoint, std::ios_base::showpos, std::ios_base::skipws, std::ios_base::unitbuf, std::ios_base::uppercase});
        static constexpr auto constants = std::to_array<type>({std::ios_base::adjustfield, std::ios_base::basefield, std::ios_base::floatfield});
        static constexpr auto empties   = std::array<type, 0>();
};

// [ios.iostate]: three elements, and goodbit, the value zero.
struct iostate
{
        using type                      = std::ios_base::iostate;
        static constexpr auto elements  = std::to_array<type>({std::ios_base::badbit, std::ios_base::eofbit, std::ios_base::failbit});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({std::ios_base::goodbit});
};

// [ios.openmode]: seven elements.
struct openmode
{
        using type                      = std::ios_base::openmode;
        static constexpr auto elements  = std::to_array<type>({std::ios_base::app, std::ios_base::ate, std::ios_base::binary, std::ios_base::in, std::ios_base::noreplace, std::ios_base::out, std::ios_base::trunc});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::array<type, 0>();
};

// [fs.enum.perms] calls no constant an element: the twelve one-bit ones are, and the rest are unions of them.
struct perms
{
        using type                      = std::filesystem::perms;
        static constexpr auto elements  = std::to_array<type>({type::owner_read, type::owner_write, type::owner_exec, type::group_read, type::group_write, type::group_exec, type::others_read, type::others_write, type::others_exec, type::set_uid, type::set_gid, type::sticky_bit});
        static constexpr auto constants = std::to_array<type>({type::owner_all, type::group_all, type::others_all, type::all, type::mask, type::unknown});
        static constexpr auto empties   = std::to_array<type>({type::none});
};

// [fs.enum.perm.opts]: every constant an element.
struct perm_options
{
        using type                      = std::filesystem::perm_options;
        static constexpr auto elements  = std::to_array<type>({type::replace, type::add, type::remove, type::nofollow});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::array<type, 0>();
};

// [fs.enum.copy.opts]: none the empty bitmask, and every other constant an element.
struct copy_options
{
        using type                      = std::filesystem::copy_options;
        static constexpr auto elements  = std::to_array<type>({type::skip_existing, type::overwrite_existing, type::update_existing, type::recursive, type::copy_symlinks, type::skip_symlinks, type::directories_only, type::create_symlinks, type::create_hard_links});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({type::none});
};

// [fs.enum.dir.opts]: none the empty bitmask, and every other constant an element.
struct directory_options
{
        using type                      = std::filesystem::directory_options;
        static constexpr auto elements  = std::to_array<type>({type::follow_directory_symlink, type::skip_permission_denied});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({type::none});
};

// [re.synopt]'s elements but multiline, absent from MSVC's STL; ECMAScript is a constant, since libc++ makes it zero.
struct syntax_option_type
{
        using type                      = std::regex_constants::syntax_option_type;
        static constexpr auto elements  = std::to_array<type>({std::regex_constants::icase, std::regex_constants::nosubs, std::regex_constants::optimize, std::regex_constants::collate, std::regex_constants::basic, std::regex_constants::extended, std::regex_constants::awk, std::regex_constants::grep, std::regex_constants::egrep});
        static constexpr auto constants = std::to_array<type>({std::regex_constants::ECMAScript});
        static constexpr auto empties   = std::array<type, 0>();
};

// [re.matchflag]: every constant an element, but match_default and format_default, which are empty bitmasks.
struct match_flag_type
{
        using type                      = std::regex_constants::match_flag_type;
        static constexpr auto elements  = std::to_array<type>({std::regex_constants::match_not_bol, std::regex_constants::match_not_eol, std::regex_constants::match_not_bow, std::regex_constants::match_not_eow, std::regex_constants::match_any, std::regex_constants::match_not_null, std::regex_constants::match_continuous, std::regex_constants::match_prev_avail, std::regex_constants::format_sed, std::regex_constants::format_no_copy, std::regex_constants::format_first_only});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({std::regex_constants::match_default, std::regex_constants::format_default});
};

// [future.syn]: the elements async and deferred.
struct launch
{
        using type                      = std::launch;
        static constexpr auto elements  = std::to_array<type>({type::async, type::deferred});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::array<type, 0>();
};

// [charconv.syn]: the elements scientific, fixed and hex, and general, the first two together.
struct chars_format
{
        using type                      = std::chars_format;
        static constexpr auto elements  = std::to_array<type>({type::scientific, type::fixed, type::hex});
        static constexpr auto constants = std::to_array<type>({type::general});
        static constexpr auto empties   = std::array<type, 0>();
};

// A type X checked against the bitmask type that Names describes: the standard's own, or the flag type over it.
template<class X, class Names>
struct candidate
{
        using type  = X;
        using names = Names;
};

template<class Names>
using standard = candidate<typename Names::type, Names>;

template<class Names>
using flag_set = candidate<xstd::bit_flag_set<typename Names::type>, Names>;

using masks = std::tuple<standard<fmtflags>, standard<iostate>, standard<openmode>, standard<perms>, standard<perm_options>, standard<copy_options>, standard<directory_options>, standard<syntax_option_type>, standard<match_flag_type>, standard<launch>, standard<chars_format>>;

using flag_sets = std::tuple<flag_set<fmtflags>, flag_set<iostate>, flag_set<openmode>, flag_set<perms>, flag_set<perm_options>, flag_set<copy_options>, flag_set<directory_options>, flag_set<syntax_option_type>, flag_set<match_flag_type>, flag_set<launch>, flag_set<chars_format>>;

// The standard's types first, then the flag type over each.
using all = decltype(std::tuple_cat(std::declval<masks>(), std::declval<flag_sets>()));

template<class C>
using word_t = xstd::underlying_block_t<typename C::names::type>;

// A value's bits as the mask's word; a flag type's via a bitset, since its complement can leave an enumeration's range.
template<class C>
[[nodiscard]] auto word(typename C::type const& x)
        -> word_t<C>
{
        if constexpr (std::same_as<typename C::type, typename C::names::type>) {
                return static_cast<word_t<C>>(x);
        } else {
                return static_cast<word_t<C>>(xstd::bit_convert<std::bitset<C::type::max_size()>>(x).to_ullong());
        }
}

// [bitmask.types]/3: the value 0, the empty bitmask, which an enumeration need not name as one of its enumerators.
template<class X>
[[nodiscard]] auto zero()
        -> X
{
        return X{}; // NOLINT(bugprone-invalid-enum-default-initialization): the empty bitmask is zero, named or not
}

// The union of the elements but the one at skip, or of every one of them where skip is past the end.
template<class X>
[[nodiscard]] auto union_but(std::vector<X> const& e, std::size_t skip)
        -> X
{
        auto u = zero<X>();
        for (auto const j : std::views::iota(0UZ, e.size())) {
                if (j != skip) {
                        u = u | e[j];
                }
        }
        return u;
}

namespace inputs {

template<class C>
[[nodiscard]] auto elements()
        -> std::vector<typename C::type>
{
        auto v = std::vector<typename C::type>();
        for (auto const e : C::names::elements) {
                v.emplace_back(e);
        }
        return v;
}

// The empty value, every named one and the union of the elements, each a value of the bitmask type itself.
template<class C>
[[nodiscard]] auto named_values()
        -> std::vector<typename C::type>
{
        using X      = C::type;
        auto const e = elements<C>();
        auto v       = std::vector<X>{zero<X>()};
        v.insert(v.end(), e.begin(), e.end());
        for (auto const c : C::names::constants) {
                v.emplace_back(c);
        }
        for (auto const c : C::names::empties) {
                v.emplace_back(c);
        }
        v.push_back(union_but(e, e.size()));
        return v;
}

// The named values and the complement of each, which the operators of [bitmask.types]/2 are taken over.
template<class C>
[[nodiscard]] auto values()
        -> std::vector<typename C::type>
{
        auto v       = named_values<C>();
        auto const n = v.size();
        for (auto const i : std::views::iota(0UZ, n)) {
                v.push_back(~v[i]);
        }
        return v;
}

} // namespace inputs

// fun called with every pair of values, a failure naming the two as words.
template<class C>
auto for_each_pair(std::vector<typename C::type> const& v, auto fun)
        -> void
{
        auto const from = test::spec::edge("named values", static_cast<std::size_t>(std::numeric_limits<word_t<C>>::digits));
        for (auto const& x : v) {
                for (auto const& y : v) {
                        auto const wx         = static_cast<std::uint64_t>(word<C>(x));
                        auto const wy         = static_cast<std::uint64_t>(word<C>(y));
                        auto const on_failure = test::spec::context(from, wx, wy);
                        fun(x, y);
                }
        }
}

} // namespace test::spec::bitmask

#endif // TEST_SPEC_BITMASK_HPP
