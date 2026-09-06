//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_DYNAMIC_BITSET_HPP
#define XSTD_BITS_EXT_BOOST_DYNAMIC_BITSET_HPP

// IWYU pragma: always_keep

#include <boost/dynamic_bitset.hpp>                // IWYU pragma: export; dynamic_bitset
#include <xstd/bits/bit_traits.hpp>                // bit_traits
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cassert>                                 // assert
#include <cstddef>                                 // size_t
#include <span>                                    // dynamic_extent

// The one door for boost::dynamic_bitset; a specialization cannot be shadowed by a future upstream member, as an ADL hook could. [design.md#the-door]
namespace xstd {

template<xstd::unsigned_integer Block, class Allocator>
struct bit_traits<boost::dynamic_bitset<Block, Allocator>>
{
        using bits_type = boost::dynamic_bitset<Block, Allocator>;

        static constexpr std::size_t extent = std::dynamic_extent;

        [[nodiscard]] static constexpr auto size (bits_type const& c)                noexcept -> std::size_t { return c.size();  }
        [[nodiscard]] static constexpr auto at   (bits_type const& c, std::size_t n) noexcept -> bool        { return c[n];      }
        [[nodiscard]] static constexpr auto count(bits_type const& c)                noexcept -> std::size_t { return c.count(); }

        static constexpr void unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept { c[n] = value; }

        // The one entry a dynamic width answers by growing; n + 1 must be addressable, the ruled-out position being the one whose successor wraps. [design.md#what-the-door-reconciles]
        static constexpr void insert(bits_type& c, std::size_t n)
        {
                if (n >= c.size()) {
                        assert(n < c.max_size());
                        c.resize(n + 1UZ);
                }
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

        // Native and worth keeping, the element-wise fallback being a test per position; npos becomes size(), the door being total. [design.md#total-versus-precondition]
        [[nodiscard]] static auto find_first(bits_type const& c) noexcept
                -> std::size_t
        {
                auto const n = c.find_first();
                return n == bits_type::npos ? c.size() : n;
        }

        [[nodiscard]] static auto find_next(bits_type const& c, std::size_t n) noexcept
                -> std::size_t
        {
                auto const next = c.find_next(n);
                return next == bits_type::npos ? c.size() : next;
        }

        // No find_last, find_prev or block access: boost has none, so the door synthesizes what it can. [design.md#detection-by-absence]
};

}       // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_DYNAMIC_BITSET_HPP
