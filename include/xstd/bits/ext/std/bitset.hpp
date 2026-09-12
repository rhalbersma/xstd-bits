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

// The one adaptation of std::bitset, and the view's contract alone: what bit_set_view and bit_span ask, nothing an owner would. [design.md#owning-is-ours] [design.md#the-trait]
namespace xstd {

template<std::size_t N>
struct bit_traits<std::bitset<N>>
{
        using bits_type = std::bitset<N>;

        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size (bits_type const&)                  noexcept -> std::size_t { return N;         }
        [[nodiscard]] static constexpr auto at   (bits_type const& c, std::size_t n) noexcept -> bool        { return c[n];      }
        [[nodiscard]] static constexpr auto count(bits_type const& c)                noexcept -> std::size_t { return c.count(); }

        // The three the sequence reading asks, which std::bitset spells itself: entries, so none is synthesized. [design.md#the-sequence-aggregates]
        [[nodiscard]] static constexpr auto all  (bits_type const& c)                noexcept -> bool        { return c.all();   }
        [[nodiscard]] static constexpr auto any  (bits_type const& c)                noexcept -> bool        { return c.any();   }
        [[nodiscard]] static constexpr auto none (bits_type const& c)                noexcept -> bool        { return c.none();  }

        static constexpr auto unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept -> void { c[n] = value; }

        // A static width cannot grow, so inserting is assigning with the position as a precondition. [design.md#what-the-trait-reconciles]
        static constexpr auto insert(bits_type& c, std::size_t n) noexcept
                -> void
        {
                assert(n < N);
                c[n] = true;
        }

        static constexpr auto fill(bits_type& c, bool value) noexcept
                -> void
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
                requires (N > ullong_digits) and requires (std::size_t i) { { c._Getword(i) } -> xstd::unsigned_integer; }
        {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<decltype(c._Getword(0UZ))>::digits);
                return (N + digits - 1UZ) / digits;
        }

        [[nodiscard]] static constexpr auto block(bits_type const& c, std::size_t i) noexcept
                requires (N > ullong_digits) and requires { { c._Getword(i) } -> xstd::unsigned_integer; }
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
};

}       // namespace xstd

#endif // XSTD_BITS_EXT_STD_BITSET_HPP
