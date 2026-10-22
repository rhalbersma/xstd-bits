//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_BIT_VECTOR_HPP

#include <xstd/bits/bit_concepts/bit_storage.hpp>             // bit_storage
#include <xstd/bits/bit_ranges/from_blocks.hpp>              // from_blocks, from_blocks_t
#include <xstd/bits/detail/allocator_base_type.hpp>          // allocator_aware, allocator_param_t
#include <xstd/bits/detail/bit_block_container.hpp>          // bit_block_container
#include <xstd/bits/detail/ownership.hpp>                    // storage, window
#include <xstd/bits/detail/rebind.hpp>                       // rebind
#include <xstd/bits/detail/sequence_adaptor.hpp>             // sequence_adaptor
#include <xstd/bits/detail/storage_rebind.hpp>               // fits_storage, storage_owner, storage_rebind, storage_width_v
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <xstd/misc/concepts/simple_allocator.hpp>           // simple_allocator
#include <cstddef>                                           // size_t
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <memory>                                            // allocator
#include <ranges>                                            // from_range, from_range_t, input_range
#include <span>                                              // dynamic_extent
#include <type_traits>                                       // is_nothrow_constructible_v, is_nothrow_move_constructible_v
#include <utility>                                           // forward, move
#include <vector>                                            // vector

namespace xstd {

// The sequence reading over a storage of blocks, at most N bits of them: std::vector<bool> under Hinnant's name for it.
template<bit_storage Blocks = std::vector<std::size_t>, std::size_t N = std::dynamic_extent>
        requires bits::detail::fits_storage<Blocks, N>
class basic_bit_vector : public bits::detail::sequence_adaptor<bits::detail::bit_block_container<Blocks, bits::detail::storage_width_v<Blocks, N>>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_vector<Blocks, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::bit_block_container<Blocks, bits::detail::storage_width_v<Blocks, N>>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_vector<Blocks, N>>;

        // An allocator argument where the storage has an allocator, and no overload taking one where it has none.
        using allocator_param = bits::detail::allocator_param_t<Blocks>;

public:
        using typename base_type::block_container_type;
        using typename base_type::size_type;

        // [vector.bool.pspc]'s constructors, in its order, each beside its allocator-extended form.
        [[nodiscard]] basic_bit_vector() = default;

        // [allocator.requirements.general] forbids an allocator copy to throw, and empty blocks allocate nothing.
        [[nodiscard]] constexpr explicit basic_bit_vector(allocator_param const& a) noexcept
                requires bits::detail::allocator_aware<Blocks>
                : base_type(a)
        {}

        [[nodiscard]] constexpr explicit basic_bit_vector(size_type n)
                : base_type(n)
        {}

        [[nodiscard]] constexpr explicit basic_bit_vector(size_type n, allocator_param const& a)
                requires bits::detail::allocator_aware<Blocks>
                : base_type(n, a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(size_type n, bool const& value)
                : base_type(n, value)
        {}

        [[nodiscard]] constexpr basic_bit_vector(size_type n, bool const& value, allocator_param const& a)
                requires bits::detail::allocator_aware<Blocks>
                : base_type(n, value, a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_vector(InputIterator first, InputIterator last)
                : base_type(first, last)
        {}

        template<std::input_iterator InputIterator>
                requires bits::detail::allocator_aware<Blocks>
        [[nodiscard]] constexpr basic_bit_vector(InputIterator first, InputIterator last, allocator_param const& a)
                : base_type(first, last, a)
        {}

        template<xstd::container_compatible_range<bool> R>
        [[nodiscard]] constexpr basic_bit_vector(std::from_range_t, R&& rg)
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        template<xstd::container_compatible_range<bool> R>
                requires bits::detail::allocator_aware<Blocks>
        [[nodiscard]] constexpr basic_bit_vector(std::from_range_t, R&& rg, allocator_param const& a)
                : base_type(std::from_range, std::forward<R>(rg), a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(basic_bit_vector const& x, allocator_param const& a)
                requires bits::detail::allocator_aware<Blocks>
                : base_type(x, a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(basic_bit_vector&& x, allocator_param const& a)
                requires bits::detail::allocator_aware<Blocks>
                : base_type(std::move(x), a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(std::initializer_list<bool> il)
                : base_type(il)
        {}

        [[nodiscard]] constexpr basic_bit_vector(std::initializer_list<bool> il, allocator_param const& a)
                requires bits::detail::allocator_aware<Blocks>
                : base_type(il, a)
        {}

        // Not in [vector.bool.pspc]: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_bit_vector(from_blocks_t, block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<block_container_type>)
                : base_type(from_blocks, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_bit_vector(from_blocks_t, block_container_type blocks, allocator_param const& a)
                requires bits::detail::allocator_aware<Blocks>
                : base_type(from_blocks, std::move(blocks), a)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_vector& x, basic_bit_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using bit_vector = basic_bit_vector<>;

// [vector.overview]'s guides: the blocks from the allocator, std::size_t by default, the element being bool.
template<std::input_iterator InputIterator, class Allocator = std::allocator<std::size_t>>
        requires xstd::simple_allocator<Allocator>
basic_bit_vector(InputIterator, InputIterator, Allocator = Allocator()) -> basic_bit_vector<std::vector<typename Allocator::value_type, Allocator>>;

template<std::ranges::input_range R, class Allocator = std::allocator<std::size_t>>
        requires xstd::simple_allocator<Allocator>
basic_bit_vector(std::from_range_t, R&&, Allocator = Allocator()) -> basic_bit_vector<std::vector<typename Allocator::value_type, Allocator>>;

template<class Allocator = std::allocator<std::size_t>>
        requires xstd::simple_allocator<Allocator>
basic_bit_vector(std::initializer_list<bool>, Allocator = Allocator()) -> basic_bit_vector<std::vector<typename Allocator::value_type, Allocator>>;

// The blocks adopted are the storage, every bit of them a position: a constant capacity becomes the cap.
template<bit_storage Blocks>
basic_bit_vector(from_blocks_t, Blocks) -> basic_bit_vector<Blocks, bits::detail::storage_width_v<Blocks, std::dynamic_extent>>;

template<bit_storage Blocks, class Allocator>
basic_bit_vector(from_blocks_t, Blocks, Allocator) -> basic_bit_vector<Blocks, bits::detail::storage_width_v<Blocks, std::dynamic_extent>>;

// Another block or width goes through the storage's bit_storage_rebind, and a counted storage recounts its blocks.
template<class Blocks, std::size_t N>
struct bits::detail::storage_owner<basic_bit_vector<Blocks, N>>
{
        template<class OtherBlocks, std::size_t M>
        using type = basic_bit_vector<OtherBlocks, M>;
};

template<class Blocks, std::size_t N>
struct bits::detail::rebind<basic_bit_vector<Blocks, N>> : bits::detail::storage_rebind<basic_bit_vector<Blocks, N>, Blocks, N>
{};

} // namespace xstd

#endif // XSTD_BITS_BIT_VECTOR_BIT_VECTOR_HPP
