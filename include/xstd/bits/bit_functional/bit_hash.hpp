//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_BIT_HASH_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_BIT_HASH_HPP

#include <xstd/bits/bit_algorithm/bit_equal.hpp>          // bit_equal
#include <xstd/bits/bit_functional/detail/bit_hasher.hpp> // bit_hasher
#include <xstd/bits/detail/hash.hpp>                      // bit_hashable, bit_hashable_range, hash_append_bit_planes, hash_append_bit_string
#include <xstd/misc/concepts/hash_algorithm.hpp>          // hash_algorithm
#include <boost/hash2/flavor.hpp>                         // default_flavor
#include <boost/hash2/xxhash.hpp>                         // xxhash_64
#include <concepts>                                       // same_as
#include <cstddef>                                        // size_t

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

// bit_hash_append into a copy of H, keyed by T as std::hash is; xxHash and native bytes, as std::hash runs it.
template<class T, xstd::hash_algorithm H = boost::hash2::xxhash_64, class Flavor = boost::hash2::default_flavor>
        requires bits::detail::bit_hashable<T> or bits::detail::bit_hashable_range<T>
class bit_hash : bits::detail::bit_hasher<H, Flavor>
{
public:
        using bits::detail::bit_hasher<H, Flavor>::bit_hasher;
        // Declared rather than using-declared: MSVC then denies access to its members through the private base.
        // NOLINTNEXTLINE(readability-redundant-typename): clang-cl's -Wc++20-compat rejects the bare dependent name.
        using is_avalanching = typename bits::detail::bit_hasher<H, Flavor>::is_avalanching;

        // Equal bits hash equal whatever blocks or storage hold them, so a lookup may cross containers of one reading.
        using is_transparent = void;

        template<class U>
                requires (std::same_as<U, T> or requires (T const& x, U const& y) { xstd::bit_equal(x, y); }) and (bits::detail::bit_hashable<U> or bits::detail::bit_hashable_range<U>)
        [[nodiscard]] constexpr auto operator()(U const& v) const
                -> std::size_t
        {
                return bits::detail::bit_hasher<H, Flavor>::operator()(v);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FUNCTIONAL_BIT_HASH_HPP
