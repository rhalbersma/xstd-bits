//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_SET_HPP
#define XSTD_BITS_BIT_INPLACE_SET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bit_set_adaptor.hpp>                     // bit_set_adaptor
#include <xstd/bits/bit_storage.hpp>                         // bit_storage_extent_v
#include <xstd/bits/detail/contiguous_bit_container.hpp>     // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/set_adaptor.hpp>                  // set_adaptor
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

// The set reading over a run-time width under a compile-time capacity: inplace names where the storage lives.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bit_inplace_set : public bits::detail::set_adaptor<bits::detail::contiguous_bit_container<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_inplace_set<Block, N>>
{
        using base_type = bits::detail::set_adaptor<bits::detail::contiguous_bit_container<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_inplace_set<Block, N>>;

public:
        using typename base_type::block_container_type;
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons] less its allocator forms, in [set.overview]'s order; key_compare is std::less, so comp is dropped.
        [[nodiscard]] constexpr basic_bit_inplace_set() noexcept
                : basic_bit_inplace_set(key_compare())
        {}

        [[nodiscard]] constexpr explicit basic_bit_inplace_set(key_compare const& /* comp */) noexcept
                : base_type()
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_inplace_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare())
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_inplace_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare())
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] basic_bit_inplace_set(basic_bit_inplace_set const& x) = default;
        [[nodiscard]] basic_bit_inplace_set(basic_bit_inplace_set&& x) = default;

        [[nodiscard]] constexpr basic_bit_inplace_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare())
                : base_type(il)
        {}

        ~basic_bit_inplace_set() = default;

        auto operator=(basic_bit_inplace_set const& x) -> basic_bit_inplace_set& = default;
        auto operator=(basic_bit_inplace_set&& x) noexcept -> basic_bit_inplace_set& = default;

        // Not in [set.cons]: flat_set's container constructor under the bit-storage tag, every bit a position.
        [[nodiscard]] constexpr basic_bit_inplace_set(from_bit_storage_t, block_container_type blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_inplace_set& x, basic_bit_inplace_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, N>;

// Every bit of the blocks a position, so the capacity is theirs, rounded to whole blocks.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_inplace_set(from_bit_storage_t, std::inplace_vector<Block, K>) -> basic_bit_inplace_set<Block, bit_storage_extent_v<Block> * K>;

// The adaptor named by its storage stays the door for an inplace_vector of blocks passed to it directly.
template<xstd::unsigned_integer Block, std::size_t K>
bit_set_adaptor(from_bit_storage_t, std::inplace_vector<Block, K>) -> bit_set_adaptor<std::inplace_vector<Block, K>>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_inplace_set = xstd::basic_bit_inplace_set<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, std::size_t N>
struct is_range<xstd::basic_bit_inplace_set<Block, N>> : std::false_type
{};

template<class Block, std::size_t N>
struct is_tuple_like<xstd::basic_bit_inplace_set<Block, N>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N>
struct hash<xstd::basic_bit_inplace_set<Block, N>> : hash<typename xstd::basic_bit_inplace_set<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BIT_INPLACE_SET_HPP
