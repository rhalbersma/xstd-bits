//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BITSET_PRIMITIVES_HPP
#define TEST_BITSET_PRIMITIVES_HPP

#include <test/dynamic.hpp>               // dynamic
#include <xstd/bits/bit_set_view.hpp>     // view
#include <xstd/bits/detail/ownership.hpp> // owned_storage
#include <boost/dynamic_bitset_fwd.hpp>   // dynamic_bitset
#include <boost/test/unit_test.hpp>       // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_NO_THROW, BOOST_CHECK_THROW
#include <algorithm>                      // all_of, any_of, equal, fold_left, min, none_of
#include <bitset>                         // bitset
#include <cstddef>                        // size_t
#include <functional>                     // hash, plus
#include <limits>                         // numeric_limits
#include <memory>                         // addressof
#include <memory_resource>                // pmr::polymorphic_allocator
#include <ranges>                         // iota, transform
#include <sstream>                        // istringstream, stringstream
#include <stdexcept>                      // invalid_argument, out_of_range, overflow_error
#include <string>                         // basic_string, char_traits, string
#include <string_view>                    // string_view
#include <type_traits>                    // remove_cvref_t

namespace test::bitset {

// A primitive holding a BOOST_CHECK_THROW NOLINTs bugprone-exception-escape: the check reads the callee, not the guard.

// The basic_string_view overload, which P2697R1 gave std::bitset for C++26; dynamic_bitset has its own contract.
template<class X>
concept fixed_string_view_constructible = requires { X(std::string_view()); } and not dynamic<X>;

// The wrapper at a run-time width answers as boost does; boost itself asserts where the wrapper throws.
template<class X>
concept dynamic_string_view_constructible = requires { X(std::string_view()); typename xstd::bits::detail::owned_storage<X>::bits_type; } and dynamic<X>;

// One function per tier: a BOOST_CHECK_THROW is three branches, and nesting three under two if constexprs hits 64.

// A width the text must fit: too long throws, whatever the text says.
template<class X>
auto check_string_view_at_a_static_width() -> void // NOLINT(bugprone-exception-escape)
{
        constexpr auto N = X().size();
        auto const zeros = std::string(N, '0');
        BOOST_CHECK_THROW( // [bitset.cons]/7
                (static_cast<void>(X(std::string_view(zeros), N + 1))), std::out_of_range
        );
}

// The two a zero width has no room to state: every position set, and a character that is neither 0 nor 1.
template<class X>
auto check_string_view_at_a_nonzero_width() -> void // NOLINT(bugprone-exception-escape)
{
        constexpr auto N = X().size();
        auto const ones = std::string(N, '1');
        BOOST_CHECK(X(std::string_view(ones)).all()); // [bitset.cons]/3

        auto invalid = std::string(N, '0');
        invalid[N - 1] = '2';
        BOOST_CHECK_THROW( // [bitset.cons]/7
                (static_cast<void>(X(std::string_view(invalid)))), std::invalid_argument
        );
}

// A run-time width is boost's contract: the text read is the width, and the two throws are as at a static width.
template<class X>
auto check_string_view_at_a_run_time_width() -> void // NOLINT(bugprone-exception-escape)
{
        BOOST_CHECK_EQUAL(X(std::string_view("0101")).size(), 4UZ);
        BOOST_CHECK(X(std::string_view("11")).all());
        BOOST_CHECK_THROW((static_cast<void>(X(std::string_view("01"), 3))), std::out_of_range);
        BOOST_CHECK_THROW((static_cast<void>(X(std::string_view("012")))), std::invalid_argument);
}

// One check per sweep rather than per position, which at a sampled width is thousands of them.
[[nodiscard]] inline auto every_position(std::size_t N, auto pred)
        -> bool
{
        return std::ranges::all_of(std::views::iota(0UZ, N), pred);
}

// [bitset.cons]/2: the first M positions are the integer's, M the smaller of the width and its digits, the rest zero.
template<class Unsigned>
auto check_integer_positions(auto const& a, Unsigned val)
        -> void
{
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Unsigned>::digits);
        BOOST_CHECK(every_position(a.size(), [&](std::size_t i) -> bool {
                return a[i] == (i < digits and ((val >> i) & Unsigned{1}) != 0);
        }));
}

// A run-time width asserts on a position out of range, as boost::dynamic_bitset's contract has it; a static one throws.
template<class X>
concept throws_out_of_range = not dynamic<std::remove_cvref_t<X>>;

template<class X>
struct constructor
{
        auto operator()() const noexcept // NOLINT(bugprone-exception-escape)
        {
                X a;
                BOOST_CHECK(a.none()); // [bitset.cons]/1

                if constexpr (fixed_string_view_constructible<X>) {
                        check_string_view_at_a_static_width<X>();
                        if constexpr (X().size() > 0) {
                                check_string_view_at_a_nonzero_width<X>();
                        }
                } else if constexpr (dynamic_string_view_constructible<X>) {
                        // The texts checked are up to four characters long, which a smaller capacity cannot hold.
                        if (X().max_size() >= 4) {
                                check_string_view_at_a_run_time_width<X>();
                        }
                }
        }

        auto operator()(unsigned long long val) const noexcept // NOLINT(bugprone-exception-escape)
        {
                check_integer_positions(X(val), val);
        }

        // boost::dynamic_bitset's count constructor, whose value is an unsigned long and so as wide as the platform's.
        auto operator()(std::size_t num_bits, unsigned long val) const noexcept // NOLINT(bugprone-exception-escape)
        {
                auto const a = X(num_bits, val);
                BOOST_CHECK_EQUAL(a.size(), num_bits);
                check_integer_positions(a, val);
        }
};

// The bit string, most significant position first: the member where the type has one, boost's free function otherwise.
template<class X>
[[nodiscard]] auto bit_string(const X& x)
{
        if constexpr (requires { x.to_string(); }) {
                return x.to_string();
        } else {
                auto s = std::string();
                to_string(x, s);
                return s;
        }
}

// The bit string spelled in other characters: position N - 1 - i is one exactly where bit i is set.
template<class charT>
[[nodiscard]] auto spelled(const auto& x, charT zero, charT one)
        -> std::basic_string<charT>
{
        auto const N = x.size();
        auto str = std::basic_string<charT>(N, zero);
        for (auto const i : std::views::iota(0UZ, N)) {
                if (x[i]) {
                        str[N - 1UZ - i] = one;
                }
        }
        return str;
}

// The charT pointer form and the zero and one arguments: std::bitset has both, boost::dynamic_bitset neither.
template<class X>
concept character_pointer_constructible = requires (char const* str) { X(str, std::string_view::npos, '0', '1'); };

template<class X>
inline constexpr auto is_std_bitset_v = false;

template<std::size_t N>
inline constexpr auto is_std_bitset_v<std::bitset<N>> = true;

#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE < 16

// libstdc++ before 16 checks only the N characters its std::bitset stores, where [bitset.cons]/7 checks all rlen.
inline constexpr auto std_bitset_checks_every_character = false;

#else

inline constexpr auto std_bitset_checks_every_character = true;

#endif

template<class X>
concept checks_every_character = std_bitset_checks_every_character or not is_std_bitset_v<X>;

// [bitset.cons]/3-7 read back: the bit string of a bitset, in every spelling the type takes, constructs that bitset.
template<class X>
struct string_constructor
{
        // As both models take it: the whole text, and the text as a slice of a longer one.
        static auto check_strings(const X& x, const std::string& str, const std::string& padded) noexcept
                -> void
        {
                auto const n = str.size();
                BOOST_CHECK(X(str) == x);
                BOOST_CHECK(X(padded, 2, n) == x);
                if constexpr (not dynamic<X>) {
                        // M is the smaller of N and rlen, so a character past the first N is checked but not stored.
                        BOOST_CHECK(X(str + "0") == x);
                        BOOST_CHECK(X(str + "1") == x);
                }
                if constexpr (requires { X(std::string_view(str)); }) {
                        BOOST_CHECK(X(std::string_view(str)) == x);
                        BOOST_CHECK(X(std::string_view(padded), 2, n) == x);
                }
        }

        // The charT pointer, the zero and one arguments, and a wider character type.
        static auto check_other_spellings(const X& x, const std::string& str, const std::string& padded) noexcept
                -> void
        {
                auto const n = str.size();
                BOOST_CHECK(X(str.c_str()) == x);
                BOOST_CHECK(X(padded.c_str() + 2, n) == x);
                auto const dots = spelled(x, '.', 'x');
                BOOST_CHECK(X(dots, 0, n, '.', 'x') == x);
                BOOST_CHECK(X(dots.c_str(), n, '.', 'x') == x);
                BOOST_CHECK(X(spelled(x, L'0', L'1')) == x);
        }

        // [bitset.cons]/7, which boost::dynamic_bitset asserts rather than throws.
        static auto check_out_of_range(const std::string& str) noexcept // NOLINT(bugprone-exception-escape)
                -> void
        {
                BOOST_CHECK_THROW(static_cast<void>(X(str, str.size() + 1)), std::out_of_range);
        }

        // The refused character needs room at a run-time width, which a full capacity has not.
        static auto check_invalid_at_a_run_time_width(const X& x, const std::string& str) noexcept // NOLINT(bugprone-exception-escape)
                -> void
        {
                if (str.size() < x.max_size()) {
                        BOOST_CHECK_THROW(static_cast<void>(X("2" + str)), std::invalid_argument);
                }
        }

        // A character among the N stored, which a zero width has none of.
        static auto check_invalid_at_a_static_width(const std::string& str) noexcept // NOLINT(bugprone-exception-escape)
                -> void
        {
                if (not str.empty()) {
                        BOOST_CHECK_THROW(static_cast<void>(X("2" + str)), std::invalid_argument);
                }
        }

        // Each of the rlen characters is checked, the one past the N stored included.
        static auto check_invalid_past_a_static_width(const std::string& str) noexcept // NOLINT(bugprone-exception-escape)
                -> void
        {
                BOOST_CHECK_THROW(static_cast<void>(X(str + "2")), std::invalid_argument);
        }

        auto operator()(const X& x) const noexcept
        {
                auto const str = bit_string(x);
                auto const padded = "01" + str + "10";
                check_strings(x, str, padded);
                if constexpr (character_pointer_constructible<X>) {
                        check_other_spellings(x, str, padded);
                        check_out_of_range(str);
                        if constexpr (dynamic<X>) {
                                check_invalid_at_a_run_time_width(x, str);
                        } else {
                                check_invalid_at_a_static_width(str);
                                if constexpr (checks_every_character<X>) {
                                        check_invalid_past_a_static_width(str);
                                }
                        }
                }
        }
};

struct mem_bit_and_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                auto const src = self;
                auto const& dst = self &= rhs;
                BOOST_CHECK(every_position(self.size(), [&](std::size_t i) -> bool {
                        return dst[i] == (not rhs[i] ? false : src[i]); // [bitset.members]/1
                }));
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/2
        }
};

struct mem_bit_or_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                auto const src = self;
                auto const& dst = self |= rhs;
                BOOST_CHECK(every_position(self.size(), [&](std::size_t i) -> bool {
                        return dst[i] == (rhs[i] ? true : src[i]); // [bitset.members]/3
                }));
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/4
        }
};

struct mem_bit_xor_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                auto const src = self;
                auto const& dst = self ^= rhs;
                BOOST_CHECK(every_position(self.size(), [&](std::size_t i) -> bool {
                        return dst[i] == (rhs[i] ? not src[i] : src[i]); // [bitset.members]/5
                }));
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/6
        }
};

// Set vocabulary, which a bitset at a static width has none of, as std::bitset has none: guarded.
struct mem_bit_minus_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                if constexpr (requires { self -= rhs; }) {
                        auto const src = self;
                        auto const& dst = self -= rhs;
                        BOOST_CHECK(every_position(self.size(), [&](std::size_t i) -> bool {
                                return dst[i] == (rhs[i] ? false : src[i]);
                        }));
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));
                }
        }
};

struct mem_shift_left_assign
{
        auto operator()(auto& self, std::size_t pos) const noexcept
        {
                auto const src = self;
                auto const& dst = self <<= pos;
                BOOST_CHECK(every_position(self.size(), [&](std::size_t I) -> bool {
                        return I < pos ? not dst[I] : dst[I] == src[I - pos]; // [bitset.members]/7
                }));
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/8
        }
};

struct mem_shift_right_assign
{
        auto operator()(auto& self, std::size_t pos) const noexcept
        {
                auto const src = self;
                auto const& dst = self >>= pos;
                auto const N = self.size();
                BOOST_CHECK(every_position(N, [&](std::size_t I) -> bool {
                        return pos >= N - I ? not dst[I] : dst[I] == src[I + pos]; // [bitset.members]/9
                }));
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/10
        }
};

struct mem_shift_left
{
        template<class X>
        auto operator()(const X& self, std::size_t pos) const noexcept
        {
                BOOST_CHECK_EQUAL(self << pos, X(self) <<= pos); // [bitset.members]/11
        }
};

struct mem_shift_right
{
        template<class X>
        auto operator()(const X& self, std::size_t pos) const noexcept
        {
                BOOST_CHECK_EQUAL(self >> pos, X(self) >>= pos); // [bitset.members]/12
        }
};

struct mem_set
{
        auto operator()(auto& self) const noexcept
        {
                auto const& dst = self.set();
                BOOST_CHECK(self.all());                                      // [bitset.members]/13
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/14
        }

        auto operator()(auto& self, std::size_t pos, bool val = true) const noexcept // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        auto const src = self;
                        auto const& dst = self.set(pos, val);
                        BOOST_CHECK(every_position(N, [&](std::size_t i) -> bool {
                                return dst[i] == (i == pos ? val : src[i]); // [bitset.members]/15
                        }));
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/16
                } else if constexpr (throws_out_of_range<decltype(self)>) {
                        BOOST_CHECK_THROW(self.set(pos, val), std::out_of_range); // [bitset.members]/17
                }
        }
};

struct mem_reset
{
        auto operator()(auto& self) const noexcept
        {
                auto const& dst = self.reset();
                BOOST_CHECK(self.none());                                     // [bitset.members]/18
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/19
        }

        auto operator()(auto& self, std::size_t pos) const noexcept // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        auto const src = self;
                        auto const& dst = self.reset(pos);
                        BOOST_CHECK(every_position(N, [&](std::size_t i) -> bool {
                                return dst[i] == (i == pos ? false : src[i]); // [bitset.members]/20
                        }));
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/21
                } else if constexpr (throws_out_of_range<decltype(self)>) {
                        BOOST_CHECK_THROW(self.reset(pos), std::out_of_range); // [bitset.members]/22
                }
        }
};

struct mem_bit_not
{
        template<class X>
        auto operator()(const X& self) const noexcept
        {
                auto x = X(self);                   // [bitset.members]/23
                BOOST_CHECK_EQUAL(~self, x.flip()); // [bitset.members]/24
        }
};

struct mem_flip
{
        auto operator()(auto& self) const noexcept
        {
                auto const src = self;
                auto const& dst = self.flip();
                BOOST_CHECK(every_position(self.size(), [&](std::size_t i) -> bool {
                        return dst[i] != src[i]; // [bitset.members]/25
                }));
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/26
        }

        auto operator()(auto& self, std::size_t pos) const noexcept // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        auto const src = self;
                        auto const& dst = self.flip(pos);
                        BOOST_CHECK(every_position(N, [&](std::size_t i) -> bool {
                                return dst[i] == (i == pos ? not src[i] : src[i]); // [bitset.members]/27
                        }));
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self)); // [bitset.members]/28
                } else if constexpr (throws_out_of_range<decltype(self)>) {
                        BOOST_CHECK_THROW(self.flip(pos), std::out_of_range); // [bitset.members]/29
                }
        }
};

struct mem_at
{
        auto operator()(const auto& self, std::size_t pos) const noexcept // NOLINT(bugprone-exception-escape)
        {
                auto const N = self.size();
                BOOST_CHECK(pos < N);                               // [bitset.members]/30
                BOOST_CHECK_EQUAL(self[pos], self.test(pos));       // [bitset.members]/31
                BOOST_CHECK_NO_THROW(static_cast<void>(self[pos])); // [bitset.members]/32
        }
};

// [bitset.members]/37-40: the value the low positions spell, or overflow_error if one past the digits is set.
template<class Unsigned>
auto check_to_unsigned(const auto& self, auto convert)
        -> void
{
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Unsigned>::digits);
        auto const N = self.size();
        auto const M = std::ranges::min(N, digits);
        auto const fits = std::ranges::none_of(std::views::iota(M, N), [&](auto i) -> bool {
                return self[i];
        });
        if (fits) {
                auto value = Unsigned{0};
                for (auto const i : std::views::iota(0UZ, M)) {
                        if (self[i]) {
                                value |= static_cast<Unsigned>(Unsigned{1} << i);
                        }
                }
                BOOST_CHECK_EQUAL(convert(self), value);
        } else {
                BOOST_CHECK_THROW(static_cast<void>(convert(self)), std::overflow_error);
        }
}

template<class X>
inline constexpr auto is_boost_dynamic_bitset_v = false;

template<class Block, class Allocator>
inline constexpr auto is_boost_dynamic_bitset_v<boost::dynamic_bitset<Block, Allocator>> = true;

#ifdef __clang_analyzer__

// The static analyzer loses boost::dynamic_bitset::to_ulong's overflow guard before the shift it protects.
template<class X>
concept analyzable_to_ulong = not is_boost_dynamic_bitset_v<X>;

#else

template<class X>
concept analyzable_to_ulong = true;

#endif

struct mem_to_ulong
{
        template<class X>
        auto operator()(const X& self) const noexcept // NOLINT(bugprone-exception-escape)
        {
                if constexpr (analyzable_to_ulong<X>) {
                        check_to_unsigned<unsigned long>(self, [](const auto& x) -> unsigned long {
                                return x.to_ulong();
                        });
                }
        }
};

// boost::dynamic_bitset converts to unsigned long only.
struct mem_to_ullong
{
        auto operator()(const auto& self) const noexcept // NOLINT(bugprone-exception-escape)
        {
                if constexpr (requires { self.to_ullong(); }) {
                        check_to_unsigned<unsigned long long>(self, [](const auto& x) -> unsigned long long {
                                return x.to_ullong();
                        });
                }
        }
};

// [bitset.members]/41-42, in the characters, traits and allocator asked for; boost's free function spells char only.
struct mem_to_string
{
        auto operator()(const auto& self) const noexcept // NOLINT(bugprone-exception-escape)
        {
                BOOST_CHECK_EQUAL(bit_string(self), spelled(self, '0', '1'));
                if constexpr (requires { self.to_string(); }) {
                        BOOST_CHECK_EQUAL(self.to_string('.', 'x'), spelled(self, '.', 'x'));
                        BOOST_CHECK(self.template to_string<wchar_t>() == spelled(self, L'0', L'1'));
                        BOOST_CHECK(self.template to_string<wchar_t>(L'-', L'+') == spelled(self, L'-', L'+'));
                        auto const pmr = self.template to_string<char, std::char_traits<char>, std::pmr::polymorphic_allocator<char>>();
                        BOOST_CHECK_EQUAL(std::string_view(pmr), spelled(self, '0', '1'));
                }
        }
};

struct mem_count
{
        auto operator()(const auto& self) const noexcept
        {
                auto const N = self.size();
                BOOST_CHECK_EQUAL(
                        self.count(),
                        std::ranges::fold_left(
                                std::views::iota(0UZ, N) | std::views::transform([&](auto i) {
                                        return self[i];
                                }),
                                0UZ, std::plus<>()
                        )
                ); // [bitset.members]/43
        }
};

struct mem_size
{
        template<class X>
        auto operator()(const X& self) const noexcept
        {
                if constexpr (not dynamic<X>) {
                        BOOST_CHECK_EQUAL(self.size(), X().size()); // [bitset.members]/44
                }
        }
};

struct mem_equal_to
{
        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                auto const N = self.size();
                BOOST_CHECK_EQUAL(
                        self == rhs,
                        std::ranges::all_of(std::views::iota(0UZ, N), [&](auto i) {
                                return self[i] == rhs[i];
                        })
                ); // [bitset.members]/45
                // The set reading cross-check is ours to make: a foreign bitset has no view.
                if constexpr (requires { xstd::bit_set_view(self); }) {
                        auto const lhs_view = xstd::bit_set_view(self);
                        auto const rhs_view = xstd::bit_set_view(rhs);
#ifdef _MSC_VER

                        BOOST_CHECK_EQUAL(
                                self == rhs,
                                std::ranges::equal(
                                        lhs_view.begin(), lhs_view.end(),
                                        rhs_view.begin(), rhs_view.end()
                                )
                        );

#else

                        // range version not working with Visual C++
                        BOOST_CHECK_EQUAL(self == rhs, std::ranges::equal(lhs_view, rhs_view));

#endif
                }
        }
};

// Two orderings: the set view's is std::set's over ascending positions, the type's own is the bit string's.
struct mem_compare_three_way
{
        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                // The set ordering is ours to check: a foreign bitset has no view.
                if constexpr (requires { xstd::bit_set_view(self); }) {
                        auto const lhs_view = xstd::bit_set_view(self);
                        auto const rhs_view = xstd::bit_set_view(rhs);
                        BOOST_CHECK(
                                (lhs_view <=> rhs_view) ==
                                std::lexicographical_compare_three_way(
                                        lhs_view.begin(), lhs_view.end(),
                                        rhs_view.begin(), rhs_view.end()
                                )
                        );
                }
                if constexpr (requires { self <=> rhs; }) {
                        BOOST_CHECK((self <=> rhs) == (bit_string(self) <=> bit_string(rhs)));
                } else if constexpr (requires { self < rhs; }) {
                        BOOST_CHECK_EQUAL(self < rhs, bit_string(self) < bit_string(rhs));
                }
        }
};

struct mem_test
{
        auto operator()(const auto& self, std::size_t pos) const noexcept // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        BOOST_CHECK_EQUAL(self.test(pos), self[pos]); // [bitset.members]/46
                } else if constexpr (throws_out_of_range<decltype(self)>) {
                        BOOST_CHECK_THROW(static_cast<void>(self.test(pos)), std::out_of_range); // [bitset.members]/47
                }
        }
};

struct mem_all
{
        auto operator()(const auto& self) const noexcept
        {
                BOOST_CHECK_EQUAL(self.all(), self.count() == self.size()); // [bitset.members]/48
        }
};

struct mem_any
{
        auto operator()(const auto& self) const noexcept
        {
                BOOST_CHECK_EQUAL(self.any(), self.count() != 0); // [bitset.members]/49
        }
};

struct mem_none
{
        auto operator()(const auto& self) const noexcept
        {
                BOOST_CHECK_EQUAL(self.none(), self.count() == 0); // [bitset.members]/50
        }
};

struct mem_is_subset_of
{
        template<class X>
        [[nodiscard]] static auto fn_is_subset_of(const X& lhs, const X& rhs) noexcept
        {
                if constexpr (requires { lhs.is_subset_of(rhs); }) {
                        return lhs.is_subset_of(rhs);
                } else {
                        return std::ranges::all_of(std::views::iota(0UZ, lhs.size()), [&](auto i) { return not lhs[i] or rhs[i]; });
                }
        }

        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(fn_is_subset_of(self, rhs), (self & ~rhs).none());
        }
};

struct mem_is_proper_subset_of
{
        template<class X>
        [[nodiscard]] static auto fn_is_proper_subset_of(const X& lhs, const X& rhs) noexcept
        {
                if constexpr (requires { lhs.is_proper_subset_of(rhs); }) {
                        return lhs.is_proper_subset_of(rhs);
                } else {
                        return std::ranges::all_of(std::views::iota(0UZ, lhs.size()), [&](auto i) { return not lhs[i] or rhs[i]; }) and lhs != rhs;
                }
        }

        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(fn_is_proper_subset_of(self, rhs), mem_is_subset_of::fn_is_subset_of(self, rhs) and self != rhs);
        }
};

// The four edges is_proper_subset_of's multi-block paths need, which singleton and doubleton pairs never reach.
struct mem_is_proper_subset_of_edges
{
        template<class X>
        auto operator()(X& a, X&) const noexcept // NOLINT(bugprone-exception-escape)
        {
                auto const N = a.size();
                if (N == 0) {
                        return;
                }
                auto const lo = 0UZ;
                auto const hi = N - 1;
                auto const one = [&](std::size_t i) -> X { auto x = a; x.set(i);           return x; };
                auto const two = [&](std::size_t i, std::size_t j) -> X { auto x = a; x.set(i); x.set(j); return x; };

                auto const check = mem_is_proper_subset_of();
                check(one(lo), one(lo));     // equal: every block compares the same
                check(one(lo), two(lo, hi)); // proper subset, differing in the last block
                check(one(hi), one(lo));     // not a subset, differing in the first block
                check(two(lo, hi), one(lo)); // a subset up to the last block, then not
        }
};

struct mem_intersects
{
        template<class X>
        [[nodiscard]] static auto fn_intersects(const X& lhs, const X& rhs) noexcept
        {
                if constexpr (requires { lhs.intersects(rhs); }) {
                        return lhs.intersects(rhs);
                } else {
                        return std::ranges::any_of(std::views::iota(0UZ, lhs.size()), [&](auto i) { return lhs[i] and rhs[i]; });
                }
        }

        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(fn_intersects(self, rhs), (self & rhs).any());
        }
};

// [bitset.hash]/1 stipulates a std::hash<std::bitset<N>>; equal values hash equal wherever one exists.
struct op_hash
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                if constexpr (requires { std::hash<X>()(lhs); }) {
                        BOOST_CHECK_EQUAL(std::hash<X>()(lhs), std::hash<X>()(X(lhs)));
                        BOOST_CHECK(lhs != rhs or std::hash<X>()(lhs) == std::hash<X>()(rhs));
                }
        }
};

struct op_bit_and
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(lhs & rhs, X(lhs) &= rhs); // [bitset.operators]/1
        }
};

struct op_bit_or
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(lhs | rhs, X(lhs) |= rhs); // [bitset.operators]/2
        }
};

struct op_bit_xor
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(lhs ^ rhs, X(lhs) ^= rhs); // [bitset.operators]/3
        }
};

struct op_bit_minus
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                if constexpr (requires { lhs - rhs; }) {
                        auto nrv = lhs;
                        BOOST_CHECK_EQUAL(lhs - rhs, nrv -= rhs);
                }
        }
};

struct op_iostream
{
        template<class X>
        auto operator()(const X& x) const noexcept // NOLINT(bugprone-exception-escape)
        {
                std::stringstream sstr;
                X y;
                sstr << x;
                sstr >> y;
                BOOST_CHECK_EQUAL(x, y); // [bitset.operators]/4-8
        }
};

// A first character that is neither zero nor one stores nothing; an exhausted stream fails the sentry instead.
template<class X>
struct op_istream_failure
{
        auto operator()() const noexcept // NOLINT(bugprone-exception-escape)
        {
                if constexpr (fixed_string_view_constructible<X>) {
                        constexpr auto N = X().size();
                        for (auto const* input : {"", "2"}) {
                                auto const exhausted = *input == '\0';
                                auto is = std::istringstream(input);
                                auto x = X();
                                is >> x;
                                BOOST_CHECK(x.none());
                                BOOST_CHECK_EQUAL(is.fail(), exhausted or N > 0); // [istream.formatted.reqmts], then [bitset.operators]/6
                        }

                        // Fewer digits than N: the loop stops on eof, and x = X(str) puts what was read low.
                        if constexpr (N > 1) {
                                auto is = std::istringstream("1");
                                auto x = X();
                                is >> x;
                                BOOST_CHECK(not is.fail());
                                BOOST_CHECK_EQUAL(x.count(), 1UZ);
                                BOOST_CHECK(x.test(0)); // [bitset.operators]/5
                        }
                }
        }
};

} // namespace test::bitset

#endif // TEST_BITSET_PRIMITIVES_HPP
