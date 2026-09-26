//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BOUNDED_VECTOR_HPP
#define XSTD_BITS_BIT_BOUNDED_VECTOR_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bit_storage.hpp>                         // bit_storage_extent_v
#include <xstd/bits/detail/contiguous_bit_container.hpp>     // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                    // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>             // sequence_adaptor
#include <xstd/bits/from_bit_storage.hpp>                    // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/ints/memory.hpp>                              // align_up
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <cstddef>                                           // size_t
#include <functional>                                        // hash
#include <initializer_list>                                  // initializer_list
#include <inplace_vector>                                    // inplace_vector
#include <iterator>                                          // input_iterator
#include <limits>                                            // numeric_limits
#include <ranges>                                            // from_range, from_range_t
#include <type_traits>                                       // false_type
#include <utility>                                           // forward, move

namespace xstd {

// The packed std::inplace_vector<bool, N> that P0843 declined to write, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_bounded_vector : public bits::detail::sequence_adaptor<bits::detail::contiguous_bit_container<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_bounded_vector<Block, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::contiguous_bit_container<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_bounded_vector<Block, N>>;

public:
        using typename base_type::block_container_type;
        using typename base_type::size_type;

        // [inplace.vector.cons], in [inplace.vector.overview]'s order.
        [[nodiscard]] basic_bit_bounded_vector() noexcept = default;

        [[nodiscard]] constexpr explicit basic_bit_bounded_vector(size_type n)
                : base_type(n)
        {}

        [[nodiscard]] constexpr basic_bit_bounded_vector(size_type n, bool const& value)
                : base_type(n, value)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_bounded_vector(InputIterator first, InputIterator last)
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<bool> R>
        [[nodiscard]] constexpr basic_bit_bounded_vector(std::from_range_t, R&& rg)
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] constexpr basic_bit_bounded_vector(std::initializer_list<bool> il)
                : base_type(il)
        {}

        // Not in [inplace.vector.cons]: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_bit_bounded_vector(from_bit_storage_t, block_container_type blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_bounded_vector& x, basic_bit_bounded_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_bounded_vector = basic_bit_bounded_vector<std::size_t, N>;

// Every bit of the blocks an element, so the capacity is theirs, rounded to whole blocks.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_bounded_vector(from_bit_storage_t, std::inplace_vector<Block, K>) -> basic_bit_bounded_vector<Block, bit_storage_extent_v<Block> * K>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_bounded_vector = xstd::basic_bit_bounded_vector<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_bounded_vector = basic_bit_bounded_vector<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, std::size_t N>
struct is_range<xstd::basic_bit_bounded_vector<Block, N>> : std::false_type
{};

template<class Block, std::size_t N>
struct is_tuple_like<xstd::basic_bit_bounded_vector<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N>
struct hash<xstd::basic_bit_bounded_vector<Block, N>> : hash<typename xstd::basic_bit_bounded_vector<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BIT_BOUNDED_VECTOR_HPP
