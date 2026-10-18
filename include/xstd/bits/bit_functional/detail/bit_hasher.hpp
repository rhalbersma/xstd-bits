//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_DETAIL_BIT_HASHER_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_DETAIL_BIT_HASHER_HPP

#include <xstd/bits/detail/hash.hpp>                               // bit_hashable, bit_hashable_range, hash_append_bit_planes, hash_append_bit_string
#include <xstd/misc/concepts/hash_algorithm.hpp>                   // hash_algorithm
#include <xstd/misc/type_traits/hash_algorithm_is_avalanching.hpp> // hash_algorithm_is_avalanching_v
#include <boost/hash2/get_integral_result.hpp>                     // get_integral_result
#include <cstddef>                                                 // size_t
#include <cstdint>                                                 // uint64_t
#include <type_traits>                                             // bool_constant

namespace xstd::bits::detail {

// A seeded prototype of H, copied on each call so that every key's bits are appended from the same state.
template<xstd::hash_algorithm H, class Flavor>
class bit_hasher
{
        H m_prototype{};

        template<class T>
        [[nodiscard]] constexpr auto appended(T const& v) const
                -> H
        {
                auto h = m_prototype;
                if constexpr (bit_hashable<T>) {
                        hash_append_bit_string(h, Flavor(), v);
                } else {
                        hash_append_bit_planes(h, Flavor(), v);
                }
                return h;
        }

public:
        // NOLINTNEXTLINE(readability-redundant-typename): clang-cl's -Wc++20-compat rejects the bare dependent name.
        using digest_type = typename H::result_type;

        // Read by boost::hash_is_avalanching: where true, Boost.Unordered's open-addressing tables skip their post-mix.
        using is_avalanching = std::bool_constant<xstd::hash_algorithm_is_avalanching_v<H>>;

        [[nodiscard]] bit_hasher() = default;

        // MSVC checks C4459 as it instantiates the template, so any namespace-scope seed a consumer declares trips it.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4459)
#endif
        [[nodiscard]] constexpr explicit bit_hasher(std::uint64_t seed)
                : m_prototype(seed)
        {}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

        [[nodiscard]] constexpr bit_hasher(unsigned char const* p, std::size_t n)
                : m_prototype(p, n)
        {}

        template<class T>
                requires bit_hashable<T> or bit_hashable_range<T>
        [[nodiscard]] constexpr auto operator()(T const& v) const
                -> std::size_t
        {
                auto h = appended(v);
                return boost::hash2::get_integral_result<std::size_t>(h);
        }

        // The algorithm's whole result, as wide on every target, where the call operator folds it to std::size_t.
        template<class T>
                requires bit_hashable<T> or bit_hashable_range<T>
        [[nodiscard]] constexpr auto digest(T const& v) const
                -> digest_type
        {
                return appended(v).result();
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_FUNCTIONAL_DETAIL_BIT_HASHER_HPP
