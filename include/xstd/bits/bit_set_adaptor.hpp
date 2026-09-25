//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_ADAPTOR_HPP
#define XSTD_BITS_BIT_SET_ADAPTOR_HPP

#include <xstd/bits/bit_storage.hpp>                     // bit_storage, bit_storage_extent_v
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                // storage
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/bits/detail/words.hpp>                    // owner_storage_t
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/limits.hpp>                          // numeric_limits
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <type_traits>                                   // false_type

namespace xstd {

// A set of indices packed into a storage of blocks it owns: std::set<std::size_t>'s reading over any block container.
template<bit_storage Blocks, std::size_t N = bit_storage_extent_v<Blocks>>
class bit_set_adaptor : public bits::detail::set_adaptor<bits::detail::owner_storage_t<Blocks, N>, bits::detail::storage::owned, bit_set_adaptor<Blocks, N>>
{
        using base_type = bits::detail::set_adaptor<bits::detail::owner_storage_t<Blocks, N>, bits::detail::storage::owned, bit_set_adaptor<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(bit_set_adaptor& x, bit_set_adaptor& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

// Block counts computed as the aliases compute them; the defaulted K keeps MSVC 17 from dropping the one-word guide.
template<xstd::unsigned_integer B, std::size_t K = 1>
bit_set_adaptor(from_bit_storage_t, B) -> bit_set_adaptor<std::array<B, bits::detail::num_blocks_v<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

template<xstd::unsigned_integer B, std::size_t K>
bit_set_adaptor(from_bit_storage_t, std::array<B, K>) -> bit_set_adaptor<std::array<B, bits::detail::num_blocks_v<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Blocks, std::size_t N>
struct is_range<xstd::bit_set_adaptor<Blocks, N>> : std::false_type
{};

template<class Blocks, std::size_t N>
struct is_tuple_like<xstd::bit_set_adaptor<Blocks, N>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Blocks, std::size_t N>
struct hash<xstd::bit_set_adaptor<Blocks, N>> : hash<typename xstd::bit_set_adaptor<Blocks, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BIT_SET_ADAPTOR_HPP
