//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>        // for_each_type
#include <test/spec/input.hpp>           // context, edge
#include <xstd/bits/bit/bit_convert.hpp> // bit_convert
#include <xstd/bits/bit_blocks.hpp>      // underlying_block_t
#include <xstd/bits/bit_flag_set.hpp>    // bit_flag_set
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                         // array, to_array
#include <bitset>                        // bitset
#include <charconv>                      // chars_format
#include <concepts>                      // convertible_to, integral, same_as
#include <cstddef>                       // size_t
#include <cstdint>                       // uint64_t
#include <filesystem>                    // copy_options, directory_options, perm_options, perms
#include <future>                        // launch
#include <ios>                           // ios_base
#include <limits>                        // numeric_limits
#include <memory>                        // addressof
#include <ranges>                        // iota
#include <regex>                         // regex_constants
#include <tuple>                         // tuple
#include <type_traits>                   // is_enum_v, is_lvalue_reference_v, remove_cvref_t
#include <vector>                        // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Library)
BOOST_AUTO_TEST_SUITE(Description)
BOOST_AUTO_TEST_SUITE(Conventions)
BOOST_AUTO_TEST_SUITE(TypeDescriptions)
BOOST_AUTO_TEST_SUITE(BitmaskTypes)

namespace {

namespace fs    = std::filesystem;
namespace regex = std::regex_constants;
using ios       = std::ios_base;

// Each bitmask type the standard names, as its bitmask elements, its multi-bit constants and its empty ones.

// [ios.fmtflags]: fifteen elements, and three constants that each combine several.
struct fmtflags
{
        using type                      = ios::fmtflags;
        static constexpr auto elements  = std::to_array<type>({ios::boolalpha, ios::dec, ios::fixed, ios::hex, ios::internal, ios::left, ios::oct, ios::right, ios::scientific, ios::showbase, ios::showpoint, ios::showpos, ios::skipws, ios::unitbuf, ios::uppercase});
        static constexpr auto constants = std::to_array<type>({ios::adjustfield, ios::basefield, ios::floatfield});
        static constexpr auto empties   = std::array<type, 0>();
};

// [ios.iostate]: three elements, and goodbit, the value zero.
struct iostate
{
        using type                      = ios::iostate;
        static constexpr auto elements  = std::to_array<type>({ios::badbit, ios::eofbit, ios::failbit});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({ios::goodbit});
};

// [ios.openmode]: seven elements.
struct openmode
{
        using type                      = ios::openmode;
        static constexpr auto elements  = std::to_array<type>({ios::app, ios::ate, ios::binary, ios::in, ios::noreplace, ios::out, ios::trunc});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::array<type, 0>();
};

// [fs.enum.perms] calls no constant an element: the twelve one-bit ones are, and the rest are unions of them.
struct perms
{
        using type                      = fs::perms;
        static constexpr auto elements  = std::to_array<type>({type::owner_read, type::owner_write, type::owner_exec, type::group_read, type::group_write, type::group_exec, type::others_read, type::others_write, type::others_exec, type::set_uid, type::set_gid, type::sticky_bit});
        static constexpr auto constants = std::to_array<type>({type::owner_all, type::group_all, type::others_all, type::all, type::mask, type::unknown});
        static constexpr auto empties   = std::to_array<type>({type::none});
};

// [fs.enum.perm.opts]: every constant an element.
struct perm_options
{
        using type                      = fs::perm_options;
        static constexpr auto elements  = std::to_array<type>({type::replace, type::add, type::remove, type::nofollow});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::array<type, 0>();
};

// [fs.enum.copy.opts]: none the empty bitmask, and every other constant an element.
struct copy_options
{
        using type                      = fs::copy_options;
        static constexpr auto elements  = std::to_array<type>({type::skip_existing, type::overwrite_existing, type::update_existing, type::recursive, type::copy_symlinks, type::skip_symlinks, type::directories_only, type::create_symlinks, type::create_hard_links});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({type::none});
};

// [fs.enum.dir.opts]: none the empty bitmask, and every other constant an element.
struct directory_options
{
        using type                      = fs::directory_options;
        static constexpr auto elements  = std::to_array<type>({type::follow_directory_symlink, type::skip_permission_denied});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({type::none});
};

// [re.synopt]'s elements, two of them held as constants: libc++ makes ECMAScript zero, and its ~ drops multiline.
struct syntax_option_type
{
        using type                      = regex::syntax_option_type;
        static constexpr auto elements  = std::to_array<type>({regex::icase, regex::nosubs, regex::optimize, regex::collate, regex::basic, regex::extended, regex::awk, regex::grep, regex::egrep});
        static constexpr auto constants = std::to_array<type>({regex::ECMAScript, regex::multiline});
        static constexpr auto empties   = std::array<type, 0>();
};

// [re.matchflag]: every constant an element, but match_default and format_default, which are empty bitmasks.
struct match_flag_type
{
        using type                      = regex::match_flag_type;
        static constexpr auto elements  = std::to_array<type>({regex::match_not_bol, regex::match_not_eol, regex::match_not_bow, regex::match_not_eow, regex::match_any, regex::match_not_null, regex::match_continuous, regex::match_prev_avail, regex::format_sed, regex::format_no_copy, regex::format_first_only});
        static constexpr auto constants = std::array<type, 0>();
        static constexpr auto empties   = std::to_array<type>({regex::match_default, regex::format_default});
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

using candidates = std::tuple<
        standard<fmtflags>, standard<iostate>, standard<openmode>, standard<perms>, standard<perm_options>, standard<copy_options>, standard<directory_options>, standard<syntax_option_type>, standard<match_flag_type>, standard<launch>, standard<chars_format>,
        flag_set<fmtflags>, flag_set<iostate>, flag_set<openmode>, flag_set<perms>, flag_set<perm_options>, flag_set<copy_options>, flag_set<directory_options>, flag_set<syntax_option_type>, flag_set<match_flag_type>, flag_set<launch>, flag_set<chars_format>>;

template<class X>
inline constexpr bool is_bitset = false;

template<std::size_t N>
inline constexpr bool is_bitset<std::bitset<N>> = true;

// The lvalue a compound assignment returns: [bitmask.types]/2 shows bitmask&, which libstdc++'s streams make const.
template<class R, class X>
concept lvalue_of = std::is_lvalue_reference_v<R> and std::same_as<std::remove_cvref_t<R>, X>;

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

// [bitmask.types]/4: the value y is set in the object x if x & y is nonzero.
template<class X>
[[nodiscard]] auto is_set(X const& x, X const& y)
        -> bool
{
        return (x & y) != zero<X>();
}

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

// A flag type's operators, on two flag types or on one and a mask, against the mask's own.
template<class C>
auto operators_agree(typename C::type const& a, typename C::type const& b)
        -> void
{
        using T = C::type;
        using F = flag_set<typename C::names>;
        using S = F::type;
        BOOST_CHECK(static_cast<T>(S(a) & S(b)) == (a & b) and static_cast<T>(S(a) & b) == (a & b) and static_cast<T>(a & S(b)) == (a & b));
        BOOST_CHECK(static_cast<T>(S(a) | S(b)) == (a | b) and static_cast<T>(S(a) | b) == (a | b) and static_cast<T>(a | S(b)) == (a | b));
        BOOST_CHECK(static_cast<T>(S(a) ^ S(b)) == (a ^ b) and static_cast<T>(S(a) ^ b) == (a ^ b) and static_cast<T>(a ^ S(b)) == (a ^ b));

        auto x = S(a);
        auto y = S(a);
        BOOST_CHECK(static_cast<T>(x &= S(b)) == (a & b) and static_cast<T>(y &= b) == (a & b));
        x = S(a);
        y = S(a);
        BOOST_CHECK(static_cast<T>(x |= S(b)) == (a | b) and static_cast<T>(y |= b) == (a | b));
        x = S(a);
        y = S(a);
        BOOST_CHECK(static_cast<T>(x ^= S(b)) == (a ^ b) and static_cast<T>(y ^= b) == (a ^ b));

        // The complements agree on every bit that both of them complement.
        auto const both = static_cast<word_t<C>>(word<C>(~zero<T>()) & word<F>(~zero<S>()));
        BOOST_CHECK(static_cast<word_t<C>>(word<F>(~S(a)) & both) == static_cast<word_t<C>>(word<C>(~a) & both));
}

} // namespace

// [bitmask.types]/1: an enumerated type that overloads certain operators, an integer type, or a bitset
BOOST_AUTO_TEST_CASE(Bitmask)
{
        // How the library may implement its own types; a flag type is a class, held to the rest by its operators.
        test::for_each_type<masks>([]<class C> -> void {
                using X = C::type;
                static_assert(std::is_enum_v<X> or std::integral<X> or is_bitset<X>); // [bitmask.types]/1
        });
        BOOST_CHECK(true);
}

// [bitmask.types]/2: constexpr bitmask operator&(bitmask X, bitmask Y);
BOOST_AUTO_TEST_CASE(BitAnd)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x, X const y) { { x & y } -> std::same_as<X>; }); // [bitmask.types]/2
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        BOOST_CHECK(word<C>(x & y) == static_cast<word_t<C>>(word<C>(x) & word<C>(y))); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: constexpr bitmask operator|(bitmask X, bitmask Y);
BOOST_AUTO_TEST_CASE(BitOr)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x, X const y) { { x | y } -> std::same_as<X>; }); // [bitmask.types]/2
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        BOOST_CHECK(word<C>(x | y) == static_cast<word_t<C>>(word<C>(x) | word<C>(y))); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: constexpr bitmask operator^(bitmask X, bitmask Y);
BOOST_AUTO_TEST_CASE(BitXor)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x, X const y) { { x ^ y } -> std::same_as<X>; }); // [bitmask.types]/2
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        BOOST_CHECK(word<C>(x ^ y) == static_cast<word_t<C>>(word<C>(x) ^ word<C>(y))); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: constexpr bitmask operator~(bitmask X);
BOOST_AUTO_TEST_CASE(Complement)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x) { { ~x } -> std::same_as<X>; }); // [bitmask.types]/2
                // The empty value's complement holds the bits of int_type: every element, and what else a type keeps.
                auto const universe = word<C>(~zero<X>());
                for (auto const& e : elements<C>()) {
                        BOOST_CHECK(static_cast<word_t<C>>(universe & word<C>(e)) == word<C>(e)); // [bitmask.types]/2
                }
                for (auto const& x : values<C>()) {
                        BOOST_CHECK(word<C>(~x) == static_cast<word_t<C>>(static_cast<word_t<C>>(~word<C>(x)) & universe)); // [bitmask.types]/2
                }
        });
}

// [bitmask.types]/2: bitmask& operator&=(bitmask& X, bitmask Y);
BOOST_AUTO_TEST_CASE(AndAssign)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X x, X const y) { { x &= y } -> lvalue_of<X>; }); // [bitmask.types]/2
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        auto z        = x;
                        auto const& r = (z &= y);
                        BOOST_CHECK(std::addressof(r) == std::addressof(z) and z == (x & y)); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: bitmask& operator|=(bitmask& X, bitmask Y);
BOOST_AUTO_TEST_CASE(OrAssign)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X x, X const y) { { x |= y } -> lvalue_of<X>; }); // [bitmask.types]/2
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        auto z        = x;
                        auto const& r = (z |= y);
                        BOOST_CHECK(std::addressof(r) == std::addressof(z) and z == (x | y)); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: bitmask& operator^=(bitmask& X, bitmask Y);
BOOST_AUTO_TEST_CASE(XorAssign)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X x, X const y) { { x ^= y } -> lvalue_of<X>; }); // [bitmask.types]/2
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        auto z        = x;
                        auto const& r = (z ^= y);
                        BOOST_CHECK(std::addressof(r) == std::addressof(z) and z == (x ^ y)); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/3: inline constexpr bitmask C0(V0); and the other bitmask elements
BOOST_AUTO_TEST_CASE(Elements)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X      = C::type;
                auto const e = elements<C>();
                for (auto const i : std::views::iota(0UZ, e.size())) {
                        BOOST_CHECK(e[i] != zero<X>() and (e[i] & e[i]) != zero<X>()); // [bitmask.types]/3
                        BOOST_CHECK((zero<X>() & e[i]) == zero<X>());                  // [bitmask.types]/3
                        for (auto const j : std::views::iota(0UZ, e.size())) {
                                BOOST_CHECK(i == j or (e[i] != e[j] and (e[i] & e[j]) == zero<X>())); // [bitmask.types]/3
                        }
                }
                for (auto const z : C::names::empties) {
                        BOOST_CHECK(X(z) == zero<X>()); // [bitmask.types]/3
                }
        });
}

// [bitmask.types]/4: to set a value Y in an object X is to evaluate the expression X |= Y
BOOST_AUTO_TEST_CASE(Set)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        auto z = x;
                        z |= y;
                        BOOST_CHECK(y == zero<X>() or is_set(z, y)); // [bitmask.types]/4
                        for (auto const& f : elements<C>()) {
                                BOOST_CHECK(is_set(z, f) == (is_set(x, f) or is_set(y, f))); // [bitmask.types]/4
                        }
                });
        });
}

// [bitmask.types]/4: to clear a value Y in an object X is to evaluate the expression X &= ~Y
BOOST_AUTO_TEST_CASE(Clear)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X = C::type;
                for_each_pair<C>(values<C>(), [](X const& x, X const& y) -> void {
                        auto z = x;
                        z &= ~y;
                        BOOST_CHECK(not is_set(z, y)); // [bitmask.types]/4
                        for (auto const& f : elements<C>()) {
                                BOOST_CHECK(is_set(z, f) == (is_set(x, f) and not is_set(y, f))); // [bitmask.types]/4
                        }
                });
        });
}

// [bitmask.types]/4: the value Y is set in the object X if the expression X & Y is nonzero
BOOST_AUTO_TEST_CASE(IsSet)
{
        test::for_each_type<candidates>([]<class C> -> void {
                using X           = C::type;
                auto const e      = elements<C>();
                auto const in_all = union_but(e, e.size());
                for (auto const i : std::views::iota(0UZ, e.size())) {
                        auto const but_one = union_but(e, i);
                        for (auto const j : std::views::iota(0UZ, e.size())) {
                                BOOST_CHECK(is_set(e[i], e[j]) == (i == j));    // [bitmask.types]/4
                                BOOST_CHECK(is_set(but_one, e[j]) == (i != j)); // [bitmask.types]/4
                        }
                        BOOST_CHECK(is_set(in_all, e[i]) and not is_set(zero<X>(), e[i])); // [bitmask.types]/4
                }
        });
}

// xstd flag set: constexpr X(const Mask& mask) noexcept; constexpr operator Mask() const noexcept;
BOOST_AUTO_TEST_CASE(MaskConversion)
{
        test::for_each_type<masks>([]<class C> -> void {
                using T = C::type;
                using S = xstd::bit_flag_set<T>;
                static_assert(std::convertible_to<T, S> and std::convertible_to<S, T>);
                for (auto const& a : named_values<C>()) {
                        S const s = a;
                        BOOST_CHECK(static_cast<T>(s) == a and s == a);
                        BOOST_CHECK(word<flag_set<typename C::names>>(s) == word<C>(a));
                }
        });
}

// xstd flag set: constexpr X operator&(const X& lhs, const Mask& rhs) noexcept; and fifteen more
BOOST_AUTO_TEST_CASE(MaskOperators)
{
        test::for_each_type<masks>([]<class C> -> void {
                using T = C::type;
                for_each_pair<C>(named_values<C>(), [](T const& a, T const& b) -> void { operators_agree<C>(a, b); });
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
