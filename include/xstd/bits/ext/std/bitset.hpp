//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_STD_BITSET_HPP
#define XSTD_BITS_EXT_STD_BITSET_HPP

// IWYU pragma: always_keep

#include <xstd/bits/bit_traits.hpp>                // bit_traits
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <bitset>                                  // IWYU pragma: export; bitset
#include <cassert>                                 // assert
#include <concepts>                                // convertible_to
#include <cstddef>                                 // size_t
#include <limits>                                  // numeric_limits

// The one adaptation of std::bitset; [namespace.std] forbids ADL hooks here, and a specialization needs none. [design.md#the-trait]
namespace xstd {

template<std::size_t N>
struct bit_traits<std::bitset<N>>
{
        using bits_type = std::bitset<N>;

        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size (bits_type const&)                  noexcept -> std::size_t { return N;         }
        [[nodiscard]] static constexpr auto at   (bits_type const& c, std::size_t n) noexcept -> bool        { return c[n];      }
        [[nodiscard]] static constexpr auto count(bits_type const& c)                noexcept -> std::size_t { return c.count(); }

        static constexpr void unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept { c[n] = value; }

        // A static width cannot grow, so inserting is assigning with the position as a precondition. [design.md#what-the-trait-reconciles]
        static constexpr void insert(bits_type& c, std::size_t n) noexcept
        {
                assert(n < N);
                c[n] = true;
        }

        static constexpr void fill(bits_type& c, bool value) noexcept
        {
                if (value) {
                        c.set();
                } else {
                        c.reset();
                }
        }

        // The portable block read: a width that fits one unsigned long long is the word to_ullong() returns, constexpr and on every library, the guard putting its overflow_error out of reach. [design.md#the-two-reserved-names]
        static constexpr auto ullong_digits = static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits);

        [[nodiscard]] static constexpr auto num_blocks(bits_type const&) noexcept
                -> std::size_t
                requires (N <= ullong_digits)
        {
                return 1UZ;
        }

        // NOLINTNEXTLINE(bugprone-exception-escape): to_ullong throws only above ullong_digits, which the constraint rules out.
        [[nodiscard]] static constexpr auto block(bits_type const& c, std::size_t i [[maybe_unused]]) noexcept
                -> unsigned long long
                requires (N <= ullong_digits)
        {
                assert(i == 0UZ);
                return c.to_ullong();
        }

        // Above that, the reserved word read, constrained and not guarded on the platform: without _Getword block_readable goes unsatisfied and the walks stay element-wise. [design.md#detection-by-absence]
        [[nodiscard]] static constexpr auto num_blocks(bits_type const& c) noexcept
                -> std::size_t
                requires (N > ullong_digits) and requires { { c._Getword(0UZ) } -> xstd::unsigned_integer; }
        {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<decltype(c._Getword(0UZ))>::digits);
                return (N + digits - 1UZ) / digits;
        }

        [[nodiscard]] static constexpr auto block(bits_type const& c, std::size_t i) noexcept
                requires (N > ullong_digits) and requires { { c._Getword(0UZ) } -> xstd::unsigned_integer; }
        {
                return c._Getword(i);
        }

        // The two reserved names are COMPLEMENTARY, not paired: libstdc++ has these and no reachable _Getword, MSVC the reverse. [design.md#the-two-reserved-names]
        [[nodiscard]] static constexpr auto find_first(bits_type const& c) noexcept
                -> std::size_t
                requires requires { { c._Find_first() } -> std::convertible_to<std::size_t>; }
        {
                return c._Find_first();
        }

        [[nodiscard]] static constexpr auto find_next(bits_type const& c, std::size_t n) noexcept
                -> std::size_t
                requires requires { { c._Find_next(n) } -> std::convertible_to<std::size_t>; }
        {
                return c._Find_next(n);
        }

        // No find_last or find_prev: the width answers one and neither library scans backwards. [design.md#the-two-reserved-names]

        // The checked family, native and throwing here alone; bitset_adaptor forwards it rather than guarding a second time. [design.md#checked-and-unchecked]
        static constexpr void checked_set  (bits_type& c, std::size_t n, bool value) { c.set(n, value); }
        static constexpr void checked_reset(bits_type& c, std::size_t n)             { c.reset(n);      }
        static constexpr void checked_flip (bits_type& c, std::size_t n)             { c.flip(n);       }
        [[nodiscard]] static constexpr auto checked_test(bits_type const& c, std::size_t n) -> bool { return c.test(n); }

        // The shifts are total here, saturating to none: forwarded as they are, the guard being the counterpart's own. [design.md#checked-and-unchecked]
        static constexpr void checked_shift_left (bits_type& c, std::size_t n) noexcept { c <<= n; }
        static constexpr void checked_shift_right(bits_type& c, std::size_t n) noexcept { c >>= n; }
};

}       // namespace xstd

// is_subset_of, is_proper_subset_of, intersects and <=> come from bit_set_view; operator-= and operator- stay, reachable only from std.
namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<std::size_t N>
auto operator-=(bitset<N>& lhs, const bitset<N>& rhs) noexcept
        -> bitset<N>&
{
        return lhs &= ~rhs;
}

template<std::size_t N>
auto operator-(const bitset<N>& lhs, const bitset<N>& rhs) noexcept
        -> bitset<N>
{
        auto nrv = lhs; nrv -= rhs; return nrv;
}

// NOLINTEND(bugprone-std-namespace-modification)

}       // namespace std

#endif // XSTD_BITS_EXT_STD_BITSET_HPP
