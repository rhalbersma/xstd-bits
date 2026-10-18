//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_BIT_DIGEST_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_BIT_DIGEST_HPP

#include <xstd/bits/bit_functional/detail/bit_hasher.hpp> // bit_hasher
#include <xstd/bits/detail/hash.hpp>                      // bit_hashable, bit_hashable_range
#include <xstd/misc/concepts/hash_algorithm.hpp>          // hash_algorithm
#include <boost/hash2/flavor.hpp>                         // little_endian_flavor
#include <boost/hash2/xxh3.hpp>                           // xxh3_128

namespace xstd {

// A digest stands in for its bits on every target: XXH3-128 keeps two keys apart, little-endian bytes keep it portable.
template<class T, xstd::hash_algorithm H = boost::hash2::xxh3_128, class Flavor = boost::hash2::little_endian_flavor>
        requires bits::detail::bit_hashable<T> or bits::detail::bit_hashable_range<T>
class bit_digest : bits::detail::bit_hasher<H, Flavor>
{
public:
        using bits::detail::bit_hasher<H, Flavor>::bit_hasher;

        // NOLINTNEXTLINE(readability-redundant-typename): clang-cl's -Wc++20-compat rejects the bare dependent name.
        using result_type = typename bits::detail::bit_hasher<H, Flavor>::digest_type;

        [[nodiscard]] constexpr auto operator()(T const& v) const
                -> result_type
        {
                return bits::detail::bit_hasher<H, Flavor>::digest(v);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FUNCTIONAL_BIT_DIGEST_HPP
