//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP
#define XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp>     // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/set_adaptor.hpp>                  // set_adaptor
#include <xstd/bits/from_bit_storage.hpp>                    // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <boost/container/new_allocator.hpp>                 // new_allocator
#include <boost/container/small_vector.hpp>                  // small_vector
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <cstddef>                                           // size_t
#include <functional>                                        // hash
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <memory>                                            // allocator_traits
#include <ranges>                                            // from_range, from_range_t
#include <type_traits>                                       // false_type, is_nothrow_move_constructible_v, type_identity_t
#include <utility>                                           // forward, move

namespace xstd {

// The set reading over the small-vector column; the allocator is Boost's own, as that container defaults to it.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
class basic_bit_small_set : public bits::detail::set_adaptor<bits::detail::contiguous_bit_container<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>, bits::detail::storage::owned, basic_bit_small_set<Block, N, Alloc>>
{
        using base_type = bits::detail::set_adaptor<bits::detail::contiguous_bit_container<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>, bits::detail::storage::owned, basic_bit_small_set<Block, N, Alloc>>;

public:
        using typename base_type::allocator_type;
        using typename base_type::block_container_type;
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons], in [set.overview]'s order, over the small vector's own allocator_type, which wraps Alloc.
        [[nodiscard]] constexpr basic_bit_small_set()
                : basic_bit_small_set(key_compare())
        {}

        [[nodiscard]] constexpr explicit basic_bit_small_set(key_compare const& /* comp */, allocator_type const& a = allocator_type())
                : base_type(a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_small_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare(), allocator_type const& a = allocator_type())
                : base_type(first, last, a)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_small_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare(), allocator_type const& a = allocator_type())
                : base_type(std::from_range, std::forward<R>(rg), a)
        {}

        [[nodiscard]] basic_bit_small_set(basic_bit_small_set const& x) = default;
        [[nodiscard]] basic_bit_small_set(basic_bit_small_set&& x) = default;

        [[nodiscard]] constexpr explicit basic_bit_small_set(allocator_type const& a)
                : base_type(a)
        {}

        [[nodiscard]] constexpr basic_bit_small_set(basic_bit_small_set const& x, std::type_identity_t<allocator_type> const& a)
                : base_type(x, a)
        {}

        [[nodiscard]] constexpr basic_bit_small_set(basic_bit_small_set&& x, std::type_identity_t<allocator_type> const& a)
                : base_type(std::move(x), a)
        {}

        [[nodiscard]] constexpr basic_bit_small_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare(), allocator_type const& a = allocator_type())
                : base_type(il, a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_small_set(InputIterator first, InputIterator last, allocator_type const& a)
                : basic_bit_small_set(first, last, key_compare(), a)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_small_set(std::from_range_t, R&& rg, allocator_type const& a)
                : basic_bit_small_set(std::from_range, std::forward<R>(rg), key_compare(), a)
        {}

        [[nodiscard]] constexpr basic_bit_small_set(std::initializer_list<value_type> il, allocator_type const& a)
                : basic_bit_small_set(il, key_compare(), a)
        {}

        ~basic_bit_small_set() = default;

        auto operator=(basic_bit_small_set const& x) -> basic_bit_small_set& = default;
        auto operator=(basic_bit_small_set&& x) noexcept(std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value or std::allocator_traits<allocator_type>::is_always_equal::value) -> basic_bit_small_set& = default;

        // Not in [set.cons]: flat_set's container constructor under the bit-storage tag, every bit a position.
        [[nodiscard]] constexpr basic_bit_small_set(from_bit_storage_t, block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<block_container_type>)
                : base_type(from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_bit_small_set(from_bit_storage_t, block_container_type blocks, allocator_type const& a)
                : base_type(from_bit_storage, std::move(blocks), a)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_small_set& x, basic_bit_small_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_small_set = basic_bit_small_set<std::size_t, N>;

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, std::size_t N, class Alloc>
struct is_range<xstd::basic_bit_small_set<Block, N, Alloc>> : std::false_type
{};

template<class Block, std::size_t N, class Alloc>
struct is_tuple_like<xstd::basic_bit_small_set<Block, N, Alloc>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N, class Alloc>
struct hash<xstd::basic_bit_small_set<Block, N, Alloc>> : hash<typename xstd::basic_bit_small_set<Block, N, Alloc>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP
