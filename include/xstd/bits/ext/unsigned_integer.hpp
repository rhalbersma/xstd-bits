//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_UNSIGNED_INTEGER_HPP
#define XSTD_BITS_EXT_UNSIGNED_INTEGER_HPP

// IWYU pragma: always_keep

#include <xstd/bits/bit_traits.hpp>                // bit_traits
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <cassert>                                 // assert
#include <cstddef>                                 // size_t

// The degenerate bit container: a built-in unsigned integer is exactly the half of std::bitset that std::bitset
// generalized FROM -- the bitwise operators over a fixed width -- without the half it added, the members. So it
// answers nothing itself and everything through the trait. [design.md#the-degenerate-bit-container]
//
// In ext/ and not in bit_traits.hpp, because adaptation is opt-in: a consumer who never includes this header
// sees no bit_traits for unsigned integers at all, and one who does gets every width at once. [design.md#opt-in]
//
// [namespace.std] forbids ADL hooks here, and a specialization needs none. [design.md#the-trait]
namespace xstd {

// A constrained partial specialization: more constrained than the primary, which is what makes it a
// specialization at all, and what keeps every other type unadapted.
template<xstd::unsigned_integer Block>
struct bit_traits<Block>
{
        using bits_type = Block;

        static constexpr std::size_t extent = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits);

        [[nodiscard]] static constexpr auto size(bits_type const&) noexcept
                -> std::size_t
        {
                return extent;
        }

        [[nodiscard]] static constexpr auto at(bits_type const& c, std::size_t n) noexcept
                -> bool
        {
                assert(n < extent);
                return ((c >> n) & Block{1}) != Block{};
        }

        // One block, and it is the word: the block tier costs nothing here, so every scan takes it.
        // [design.md#detection-by-absence]
        [[nodiscard]] static constexpr auto num_blocks(bits_type const&) noexcept
                -> std::size_t
        {
                return 1UZ;
        }

        [[nodiscard]] static constexpr auto block(bits_type const& c, std::size_t i [[maybe_unused]]) noexcept
                -> Block
        {
                assert(i == 0UZ);
                return c;
        }

        static constexpr auto unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept
                -> void
        {
                auto const mask = static_cast<Block>(Block{1} << n);
                c = value ? static_cast<Block>(c | mask) : static_cast<Block>(c & static_cast<Block>(~mask));
        }

        // A fixed width cannot grow, so inserting is assigning with the position as a precondition.
        // [design.md#what-the-trait-reconciles]
        static constexpr auto insert(bits_type& c, std::size_t n) noexcept
                -> void
        {
                assert(n < extent);
                unchecked_assign(c, n, true);
        }

        static constexpr auto fill(bits_type& c, bool value) noexcept
                -> void
        {
                c = value ? static_cast<Block>(~Block{}) : Block{};
        }
};

}       // namespace xstd

#endif  // XSTD_BITS_EXT_UNSIGNED_INTEGER_HPP
