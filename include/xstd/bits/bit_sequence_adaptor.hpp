//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SEQUENCE_ADAPTOR_HPP
#define XSTD_BITS_BIT_SEQUENCE_ADAPTOR_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, default_extent_v, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/bits/from_bits.hpp>                       // from_bits_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/limits.hpp>                          // numeric_limits
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <tuple>                                         // tuple_element, tuple_size
#include <type_traits>                                   // false_type

namespace xstd {

// A sequence of bools packed into a storage of blocks it owns: std::vector<bool>'s reading over any block container.
template<class Blocks, std::size_t N = detail::bits::default_extent_v<Blocks>>
class bit_sequence_adaptor : public detail::bits::sequence_adaptor<detail::bits::contiguous_bit_container<Blocks, N>, detail::bits::storage::owned, detail::bits::window::all, bit_sequence_adaptor<Blocks, N>>
{
        using base_type = detail::bits::sequence_adaptor<detail::bits::contiguous_bit_container<Blocks, N>, detail::bits::storage::owned, detail::bits::window::all, bit_sequence_adaptor<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(bit_sequence_adaptor& x, bit_sequence_adaptor& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

// Block counts computed as the aliases compute them; the defaulted K keeps MSVC 17 from dropping the one-word guide.
template<xstd::unsigned_integer B, std::size_t K = 1>
bit_sequence_adaptor(from_bits_t, B) -> bit_sequence_adaptor<std::array<B, detail::bits::num_blocks_v<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

template<xstd::unsigned_integer B, std::size_t K>
bit_sequence_adaptor(from_bits_t, std::array<B, K>) -> bit_sequence_adaptor<std::array<B, detail::bits::num_blocks_v<B, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>>, static_cast<std::size_t>(xstd::numeric_limits<B>::digits) * K>;

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Blocks, std::size_t N>
struct is_range<xstd::bit_sequence_adaptor<Blocks, N>> : std::false_type
{};

template<class Blocks, std::size_t N>
struct is_tuple_like<xstd::bit_sequence_adaptor<Blocks, N>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Blocks, std::size_t N>
struct hash<xstd::bit_sequence_adaptor<Blocks, N>> : hash<typename xstd::bit_sequence_adaptor<Blocks, N>::adaptor_type>
{};

template<std::size_t I, xstd::unsigned_integer Block, std::size_t K, std::size_t N>
struct tuple_element<I, xstd::bit_sequence_adaptor<std::array<Block, K>, N>> : tuple_element<I, typename xstd::bit_sequence_adaptor<std::array<Block, K>, N>::adaptor_type>
{};

template<std::size_t I, xstd::unsigned_integer Block, std::size_t K, std::size_t N>
struct tuple_element<I, const xstd::bit_sequence_adaptor<std::array<Block, K>, N>> : tuple_element<I, const typename xstd::bit_sequence_adaptor<std::array<Block, K>, N>::adaptor_type>
{};

template<xstd::unsigned_integer Block, std::size_t K, std::size_t N>
struct tuple_size<xstd::bit_sequence_adaptor<std::array<Block, K>, N>> : tuple_size<typename xstd::bit_sequence_adaptor<std::array<Block, K>, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BIT_SEQUENCE_ADAPTOR_HPP
