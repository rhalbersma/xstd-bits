//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_DYNAMIC_BITSET_HPP
#define XSTD_BITS_EXT_BOOST_DYNAMIC_BITSET_HPP

// IWYU pragma: always_keep

#include <boost/dynamic_bitset.hpp>                // IWYU pragma: export; dynamic_bitset
#include <xstd/bits/bit_traits.hpp>                // bit_traits
#include <xstd/bits/ranges/sequence_view.hpp>      // sequence_find
#include <xstd/bits/ranges/set_view.hpp>           // set_find, set_view
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <algorithm>                               // find_if, min
#include <cassert>                                 // assert
#include <compare>                                 // strong_ordering
#include <cstddef>                                 // std::size_t
#include <ranges>                                  // find_if, min
#include <span>                                    // dynamic_extent
                                        // iota, reverse

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

        static constexpr void assign(bits_type& c, std::size_t n, bool value) noexcept { c[n] = value; }

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

// Specializing set_find rather than using ADL: dynamic_bitset's own members would silently shadow a same-named free function.
namespace xstd::ranges {

template<xstd::unsigned_integer Block, class Allocator>
struct set_find<boost::dynamic_bitset<Block, Allocator>>
{
        [[nodiscard]] static constexpr auto first(boost::dynamic_bitset<Block, Allocator> const& c) noexcept
                -> std::size_t
        {
                return c.find_first();
        }

        [[nodiscard]] static constexpr auto last(boost::dynamic_bitset<Block, Allocator> const& c) noexcept
                -> std::size_t
        {
                return c.npos;
        }

        [[nodiscard]] static constexpr auto next(boost::dynamic_bitset<Block, Allocator> const& c, std::size_t n) noexcept
                -> std::size_t
        {
                return c.find_next(n);
        }

        [[nodiscard]] static constexpr auto prev(boost::dynamic_bitset<Block, Allocator> const& c, std::size_t n) noexcept
                -> std::size_t
        {
                assert(c.any());
                return *std::ranges::find_if(std::views::iota(0UZ, std::ranges::min(n, c.size())) | std::views::reverse, [&](auto i) {
                        return c[i];
                });
        }
};

// Opt in to the iteration-based ordering rather than risk inheriting whatever a future native <=> decides to mean.
template<xstd::unsigned_integer Block, class Allocator>
struct set_compare<boost::dynamic_bitset<Block, Allocator>>
{
        [[nodiscard]] static constexpr auto lexicographical_three_way(boost::dynamic_bitset<Block, Allocator> const& x, boost::dynamic_bitset<Block, Allocator> const& y) noexcept
                -> std::strong_ordering
        {
                return set_three_way(set_view(x), set_view(y));
        }
};

}       // namespace xstd::ranges

// Same reasoning as set_find above, for the array-of-bool reading: trivial, and unshadowable by a future upstream member.
namespace xstd::ranges {

template<xstd::unsigned_integer Block, class Allocator>
struct sequence_find<boost::dynamic_bitset<Block, Allocator>>
{
        [[nodiscard]] static constexpr auto first(boost::dynamic_bitset<Block, Allocator> const&) noexcept -> std::size_t { return 0UZ; }
        [[nodiscard]] static constexpr auto last (boost::dynamic_bitset<Block, Allocator> const& c) noexcept -> std::size_t { return c.size(); }
        [[nodiscard]] static constexpr auto at   (boost::dynamic_bitset<Block, Allocator> const& c, std::size_t n) noexcept -> bool { return c[n]; }
};


}       // namespace xstd::ranges

#endif // XSTD_BITS_EXT_BOOST_DYNAMIC_BITSET_HPP
