//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_BIT_HASH_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_BIT_HASH_HPP

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
class bit_hash
{
        H m_prototype{};

public:
        // Equal bits hash equal whatever blocks or storage hold them, so a lookup may cross containers of one reading.
        using is_transparent = void;

        [[nodiscard]] bit_hash() = default;

        // MSVC checks C4459 as it instantiates the template, so any namespace-scope seed a consumer declares trips it.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4459)
#endif
        [[nodiscard]] constexpr explicit bit_hash(std::uint64_t seed)
                : m_prototype(seed)
        {}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

        [[nodiscard]] constexpr bit_hash(unsigned char const* p, std::size_t n)
                : m_prototype(p, n)
        {}

        template<class T>
                requires bits::detail::bit_hashable<T> or bits::detail::bit_hashable_range<T>
        [[nodiscard]] constexpr auto operator()(T const& v) const
                -> std::size_t
        {
                auto h = appended(v);
                return boost::hash2::get_integral_result<std::size_t>(h);
        }

        // The algorithm's whole result, of which the call operator keeps a std::size_t's worth.
        template<class T>
                requires bits::detail::bit_hashable<T> or bits::detail::bit_hashable_range<T>
        [[nodiscard]] constexpr auto digest(T const& v) const
                -> typename H::result_type
        {
                return appended(v).result();
        }

private:
        template<class T>
        [[nodiscard]] constexpr auto appended(T const& v) const
                -> H
        {
                auto h = m_prototype;
                xstd::bit_hash_append(h, boost::hash2::default_flavor(), v);
                return h;
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FUNCTIONAL_BIT_HASH_HPP
