//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_HASHER_HPP
#define XSTD_BITS_BIT_HASHER_HPP

#include <xstd/bits/detail/hash.hpp>           // bit_hashable, bit_hashable_range, hash_append_bit_planes, hash_append_bit_string
#include <xstd/misc/ext/boost/hash2.hpp>       // hash_algorithm
#include <boost/hash2/flavor.hpp>              // default_flavor
#include <boost/hash2/get_integral_result.hpp> // get_integral_result
#include <boost/hash2/xxhash.hpp>              // xxhash_64
#include <cstddef>                             // size_t
#include <cstdint>                             // uint64_t

// The hash of the bits rather than of the value as its standard model sees it: their bytes, whatever the blocks.
namespace xstd {

// The bits as bytes, bit i at bit i % 8 of byte i / 8, then the width where it is a run-time value.
template<xstd::hash_algorithm Hash, class Flavor, class T>
        requires bits::detail::bit_hashable<T>
constexpr auto bit_hash_append(Hash& h, Flavor const& f, T const& x)
        -> void
{
        bits::detail::hash_append_bit_string(h, f, x);
}

// Planes of one static width in a contiguous range, nested arrays flattened: their bytes, then a run-time count.
template<xstd::hash_algorithm Hash, class Flavor, class R>
        requires bits::detail::bit_hashable_range<R>
constexpr auto bit_hash_append(Hash& h, Flavor const& f, R const& r)
        -> void
{
        bits::detail::hash_append_bit_planes(h, f, r);
}

// bit_hash_append into a copy of the algorithm H; xxHash by default, FNV-1a not carrying a last byte's high bits down.
template<xstd::hash_algorithm H = boost::hash2::xxhash_64>
class bit_hasher
{
        H m_prototype{};

public:
        // Equal bits hash equal whatever blocks or storage hold them, so a lookup may cross containers of one reading.
        using is_transparent = void;

        [[nodiscard]] bit_hasher() = default;

        // Not seed, which MSVC's C4459 flags here wherever a consumer declares a seed at namespace scope.
        [[nodiscard]] constexpr explicit bit_hasher(std::uint64_t s)
                : m_prototype(s)
        {}

        [[nodiscard]] constexpr bit_hasher(unsigned char const* p, std::size_t n)
                : m_prototype(p, n)
        {}

        template<class T>
                requires bits::detail::bit_hashable<T> or bits::detail::bit_hashable_range<T>
        [[nodiscard]] constexpr auto operator()(T const& v) const
                -> std::size_t
        {
                auto h = m_prototype;
                xstd::bit_hash_append(h, boost::hash2::default_flavor(), v);
                return boost::hash2::get_integral_result<std::size_t>(h);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_HASHER_HPP
