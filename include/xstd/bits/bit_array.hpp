//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_HPP

#include <xstd/bits/bit_storage.hpp>                     // bit_storage_extent_v
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/memory.hpp>                          // align_up
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <array>                                         // array
#include <concepts>                                      // constructible_from
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <initializer_list>                              // initializer_list
#include <limits>                                        // numeric_limits
#include <tuple>                                         // tuple_element, tuple_size
#include <type_traits>                                   // false_type

namespace xstd {

// The packed std::array<bool, N>, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_array : public bits::detail::sequence_adaptor<bits::detail::contiguous_bit_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_array<Block, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::contiguous_bit_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_array<Block, N>>;

public:
        using typename base_type::value_type;

        // std::array is an aggregate and declares none: these are what its initialization does, as constructors.
        [[nodiscard]] basic_bit_array() noexcept = default;

        // What is listed leads, the rest stays false.
        constexpr basic_bit_array(std::initializer_list<value_type> il)
                : base_type(il)
        {}

        // Not in [array]: words that are bit storage, read as this sequence's bools.
        template<class B>
                requires std::constructible_from<base_type, from_bit_storage_t, B const&>
        [[nodiscard]] constexpr basic_bit_array(from_bit_storage_t, B const& b) noexcept
                : base_type(from_bit_storage, b)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_array& x, basic_bit_array& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

// The width of one word or of an array of them; K = 1 keeps MSVC 17 from dropping the one-word guide.
template<xstd::unsigned_integer Block, std::size_t K = 1>
basic_bit_array(from_bit_storage_t, Block) -> basic_bit_array<Block, bit_storage_extent_v<Block> * K>;

// No guide from zero blocks: an empty array names no width worth deducing.
template<xstd::unsigned_integer Block, std::size_t K>
        requires (K != 0)
basic_bit_array(from_bit_storage_t, std::array<Block, K>) -> basic_bit_array<Block, bit_storage_extent_v<std::array<Block, K>>>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_array = xstd::basic_bit_array<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, std::size_t N>
struct is_range<xstd::basic_bit_array<Block, N>> : std::false_type
{};

template<class Block, std::size_t N>
struct is_tuple_like<xstd::basic_bit_array<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N>
struct hash<xstd::basic_bit_array<Block, N>> : hash<typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

// [array.tuple]'s three, as the adaptor answers them: a partial specialization never matches a derived class.
template<std::size_t I, class Block, std::size_t N>
struct tuple_element<I, xstd::basic_bit_array<Block, N>> : tuple_element<I, typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

template<std::size_t I, class Block, std::size_t N>
struct tuple_element<I, const xstd::basic_bit_array<Block, N>> : tuple_element<I, const typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

template<class Block, std::size_t N>
struct tuple_size<xstd::basic_bit_array<Block, N>> : tuple_size<typename xstd::basic_bit_array<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BIT_ARRAY_HPP
