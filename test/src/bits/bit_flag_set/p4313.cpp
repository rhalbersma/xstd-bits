//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flag/p4313.hpp>                     // bitmask_type, operator&, operator^, operator|, operator~
#include <test/set/order_isomorphism.hpp>          // is_order_isomorphism
#include <xstd/bits/bit_fixed_set.hpp>             // basic_bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>          // bit_flag_mapping
#include <xstd/bits/bit_flag_set.hpp>              // bit_flag_set
#include <xstd/bits/bit_type_traits/bit_least.hpp> // bit_least
#include <xstd/ints/concepts/bit_mask.hpp>         // bit_mask
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                               // ranges::equal
#include <array>                                   // array
#include <bit>                                     // popcount
#include <concepts>                                // same_as
#include <cstddef>                                 // size_t
#include <functional>                              // greater
#include <ranges>                                  // iota
#include <type_traits>                             // is_enum_v
#include <utility>                                 // to_underlying

namespace app {

// P4313R1's opening example, in a named namespace, its operators those declared before the xstd headers.
enum class permission // NOLINT(performance-enum-size): the paper's declaration, on int
{
        none    = 0,
        read    = 1 << 0, // NOLINT(bugprone-signed-bitwise): the paper's enumerator, on int
        write   = 1 << 1, // NOLINT(bugprone-signed-bitwise): the paper's enumerator, on int
        execute = 1 << 2, // NOLINT(bugprone-signed-bitwise): the paper's enumerator, on int
};

// The same enumeration with its operators written beside it, as a bitmask type is today.
enum class permission_by_hand // NOLINT(performance-enum-size): the paper's declaration, on int
{
        none    = 0,
        read    = 1 << 0, // NOLINT(bugprone-signed-bitwise): the paper's enumerator, on int
        write   = 1 << 1, // NOLINT(bugprone-signed-bitwise): the paper's enumerator, on int
        execute = 1 << 2, // NOLINT(bugprone-signed-bitwise): the paper's enumerator, on int
};

[[nodiscard]] constexpr auto operator|(permission_by_hand a, permission_by_hand b) noexcept
        -> permission_by_hand
{
        return static_cast<permission_by_hand>(std::to_underlying(a) | std::to_underlying(b)); // NOLINT(bugprone-signed-bitwise): the paper's operator, on int
}

[[nodiscard]] constexpr auto operator&(permission_by_hand a, permission_by_hand b) noexcept
        -> permission_by_hand
{
        return static_cast<permission_by_hand>(std::to_underlying(a) & std::to_underlying(b)); // NOLINT(bugprone-signed-bitwise): the paper's operator, on int
}

[[nodiscard]] constexpr auto operator^(permission_by_hand a, permission_by_hand b) noexcept
        -> permission_by_hand
{
        return static_cast<permission_by_hand>(std::to_underlying(a) ^ std::to_underlying(b)); // NOLINT(bugprone-signed-bitwise): the paper's operator, on int
}

[[nodiscard]] constexpr auto operator~(permission_by_hand a) noexcept
        -> permission_by_hand
{
        return static_cast<permission_by_hand>(~std::to_underlying(a)); // NOLINT(bugprone-signed-bitwise): the paper's operator, on int
}

constexpr auto operator|=(permission_by_hand& a, permission_by_hand b) noexcept
        -> permission_by_hand&
{
        return a = a | b;
}

constexpr auto operator&=(permission_by_hand& a, permission_by_hand b) noexcept
        -> permission_by_hand&
{
        return a = a & b;
}

constexpr auto operator^=(permission_by_hand& a, permission_by_hand b) noexcept
        -> permission_by_hand&
{
        return a = a ^ b;
}

// Marked for the operators that are declared only after the xstd headers.
enum class late_permission : unsigned char
{
        read  = 1U << 0U,
        write = 1U << 1U,
};

} // namespace app

// The global namespace, which an enumeration declared there brings into argument-dependent lookup.
enum class global_permission : unsigned char
{
        read  = 1U << 0U,
        write = 1U << 1U,
};

template<>
inline constexpr bool test::p4313::bitmask_type<app::permission> = true;

// The emulation as a user including it after the xstd headers would have it: the same operators, declared late.
template<class T>
inline constexpr bool late_bitmask_type = false;

template<>
inline constexpr bool late_bitmask_type<global_permission> = true;

template<>
inline constexpr bool late_bitmask_type<app::late_permission> = true;

template<class T>
concept late_bitmask_like = std::is_enum_v<T> and late_bitmask_type<T>;

template<late_bitmask_like T>
[[nodiscard]] constexpr auto operator|(T lhs, T rhs) noexcept
        -> T
{
        return static_cast<T>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

template<late_bitmask_like T>
[[nodiscard]] constexpr auto operator&(T lhs, T rhs) noexcept
        -> T
{
        return static_cast<T>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

template<late_bitmask_like T>
[[nodiscard]] constexpr auto operator^(T lhs, T rhs) noexcept
        -> T
{
        return static_cast<T>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}

template<late_bitmask_like T>
[[nodiscard]] constexpr auto operator~(T lhs) noexcept
        -> T
{
        return static_cast<T>(~std::to_underlying(lhs));
}

template<late_bitmask_like T>
constexpr auto operator|=(T& lhs, T rhs) noexcept
        -> T&
{
        return lhs = lhs | rhs;
}

template<late_bitmask_like T>
constexpr auto operator&=(T& lhs, T rhs) noexcept
        -> T&
{
        return lhs = lhs & rhs;
}

template<late_bitmask_like T>
constexpr auto operator^=(T& lhs, T rhs) noexcept
        -> T&
{
        return lhs = lhs ^ rhs;
}

BOOST_AUTO_TEST_SUITE(BitFlagSet)
BOOST_AUTO_TEST_SUITE(P4313)

namespace {

using app::permission;
using flags = xstd::bit_flag_set<permission>;

// Whether Mask keys a flag type at its default width.
template<class Mask>
constexpr bool names_a_flag_type = requires { typename xstd::bit_flag_set<Mask>; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with it

// The alias as its definition spells it, which a P4313 enumeration and a hand-written one both are.
template<class Mask, std::size_t N>
constexpr bool is_the_flag_type = std::same_as<xstd::bit_flag_set<Mask>, xstd::bit_least<xstd::basic_bit_fixed_set<Mask, std::size_t, N, xstd::bit_flag_mapping<Mask, N>, std::greater<Mask>>>>; // NOLINT(modernize-use-transparent-functors): the comparator the alias names

// Every value of the three flags: none, each alone, and each union.
constexpr auto values = [] noexcept -> std::array<permission, 8> {
        auto nrv = std::array<permission, 8>();
        for (auto const i : std::views::iota(0UZ, nrv.size())) {
                nrv[i] = static_cast<permission>(i);
        }
        return nrv;
}();

} // namespace

// The emulated operators reach xstd::bit_mask by lookup at the concept's definition, or by ADL in the global namespace.
BOOST_AUTO_TEST_CASE(AP4313EnumerationIsABitMaskWhereLookupFindsItsOperators)
{
        static_assert(xstd::bit_mask<permission> and names_a_flag_type<permission>);
        static_assert(xstd::bit_mask<global_permission> and names_a_flag_type<global_permission>);
        static_assert(xstd::bit_mask<app::permission_by_hand>);

        // Declared after the xstd headers, outside the enumeration's namespace: no lookup finds them, a caller does.
        static_assert(not xstd::bit_mask<app::late_permission> and not names_a_flag_type<app::late_permission>);
        BOOST_CHECK((app::late_permission::read | app::late_permission::read) == app::late_permission::read);
}

// The flag type over a P4313 enumeration is the one over a hand-written bitmask enumeration of the same declaration.
BOOST_AUTO_TEST_CASE(TheFlagTypeIsTheOneOverAHandWrittenEnumeration)
{
        static_assert(is_the_flag_type<permission, 31UZ> and is_the_flag_type<app::permission_by_hand, 31UZ>);
        static_assert(flags::max_size() == xstd::bit_flag_set<app::permission_by_hand>::max_size());
        static_assert(sizeof(flags) == sizeof(xstd::bit_flag_set<app::permission_by_hand>));
        static_assert(xstd::bit_mask<flags>);
        static_assert(test::set::is_order_isomorphism<xstd::bit_flag_mapping<permission>, permission>());
        BOOST_CHECK(true);
}

// Every value converts in and back, and the flags run from the highest down.
BOOST_AUTO_TEST_CASE(EveryValueRoundTripsAndIteratesHighestFirst)
{
        for (auto const v : values) {
                BOOST_CHECK(permission(flags(v)) == v);
                BOOST_CHECK_EQUAL(flags(v).size(), static_cast<std::size_t>(std::popcount(static_cast<unsigned>(std::to_underlying(v)))));
        }
        BOOST_CHECK(std::ranges::equal(flags{permission::read, permission::execute}, std::array{permission::execute, permission::read}));
        BOOST_CHECK(flags(permission::none).empty());
}

// Each mixed operator, in both orders, answers as the emulated operator on the enumeration does.
BOOST_AUTO_TEST_CASE(EachOperatorAgreesWithTheEmulatedOnes)
{
        auto mismatches = 0UZ;
        for (auto const a : values) {
                for (auto const b : values) {
                        auto const s = flags(a);
                        mismatches += static_cast<std::size_t>(permission(s | b) != (a | b) or permission(b | s) != (b | a));
                        mismatches += static_cast<std::size_t>(permission(s & b) != (a & b) or permission(b & s) != (b & a));
                        mismatches += static_cast<std::size_t>(permission(s ^ b) != (a ^ b) or permission(b ^ s) != (b ^ a));
                        mismatches += static_cast<std::size_t>(permission(s - b) != (a & ~b) or permission(b - s) != (b & ~a));
                        auto x = s;
                        x |= b;
                        auto y = a;
                        y |= b;
                        mismatches += static_cast<std::size_t>(permission(x) != y);
                }
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
}

// The flag type's complement stays within its N flags, where the enumeration's leaves every enumerator behind.
BOOST_AUTO_TEST_CASE(TheComplementStaysWithinTheWidth)
{
        using three = xstd::bit_flag_set<permission, 3>;
        static_assert(permission(~three()) == (permission::read | permission::write | permission::execute));
        static_assert(std::to_underlying(~permission::none) == -1);
        static_assert(std::to_underlying(permission(~flags())) == 0x7FFF'FFFF);
        BOOST_CHECK(std::ranges::equal(~three(permission::write), std::array{permission::execute, permission::read}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
