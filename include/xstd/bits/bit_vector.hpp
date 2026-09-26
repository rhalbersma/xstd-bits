//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_HPP

#include <xstd/bits/bit_sequence_adaptor.hpp>                // bit_sequence_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp>     // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                    // storage, window
#include <xstd/bits/detail/sequence_adaptor.hpp>             // sequence_adaptor
#include <xstd/bits/from_bit_storage.hpp>                    // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <xstd/misc/concepts/simple_allocator.hpp>           // simple_allocator
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <cstddef>                                           // size_t
#include <functional>                                        // hash
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <memory>                                            // allocator, allocator_traits
#include <ranges>                                            // from_range, from_range_t, input_range
#include <type_traits>                                       // false_type, type_identity_t
#include <utility>                                           // forward, move
#include <vector>                                            // vector

namespace xstd {

// The sequence reading over a heap of blocks: std::vector<bool> under the name Hinnant proposed for it.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_bit_vector : public bits::detail::sequence_adaptor<bits::detail::contiguous_bit_container<std::vector<Block, Allocator>>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_vector<Block, Allocator>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::contiguous_bit_container<std::vector<Block, Allocator>>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_vector<Block, Allocator>>;

public:
        using typename base_type::allocator_type;
        using typename base_type::size_type;

        // [vector.bool.pspc]'s constructors, in its order.
        [[nodiscard]] constexpr basic_bit_vector() noexcept(noexcept(Allocator()))
                : basic_bit_vector(Allocator())
        {}

        [[nodiscard]] constexpr explicit basic_bit_vector(Allocator const& a) noexcept
                : base_type(a)
        {}

        [[nodiscard]] constexpr explicit basic_bit_vector(size_type n, Allocator const& a = Allocator())
                : base_type(n, a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(size_type n, bool const& value, Allocator const& a = Allocator())
                : base_type(n, value, a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_vector(InputIterator first, InputIterator last, Allocator const& a = Allocator())
                : base_type(first, last, a)
        {}

        template<xstd::container_compatible_range<bool> R>
        [[nodiscard]] constexpr basic_bit_vector(std::from_range_t, R&& rg, Allocator const& a = Allocator())
                : base_type(std::from_range, std::forward<R>(rg), a)
        {}

        [[nodiscard]] basic_bit_vector(basic_bit_vector const& x) = default;
        [[nodiscard]] basic_bit_vector(basic_bit_vector&& x) noexcept = default;

        [[nodiscard]] constexpr basic_bit_vector(basic_bit_vector const& x, std::type_identity_t<Allocator> const& a)
                : base_type(x, a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(basic_bit_vector&& x, std::type_identity_t<Allocator> const& a)
                : base_type(std::move(x), a)
        {}

        [[nodiscard]] constexpr basic_bit_vector(std::initializer_list<bool> il, Allocator const& a = Allocator())
                : base_type(il, a)
        {}

        ~basic_bit_vector() = default;

        auto operator=(basic_bit_vector const& x) -> basic_bit_vector& = default;
        auto operator=(basic_bit_vector&& x) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value or std::allocator_traits<Allocator>::is_always_equal::value) -> basic_bit_vector& = default;

        // Not in [vector.bool.pspc]: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_bit_vector(from_bit_storage_t, std::vector<Block, Allocator> blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_bit_vector(from_bit_storage_t, std::vector<Block, Allocator> blocks, Allocator const& a)
                : base_type(from_bit_storage, std::move(blocks), a)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_vector& x, basic_bit_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using bit_vector = basic_bit_vector<std::size_t>;

// [vector.overview]'s guides: Block from the allocator, std::size_t by default, the element being bool.
template<std::input_iterator InputIterator, class Allocator = std::allocator<std::size_t>>
        requires xstd::simple_allocator<Allocator>
basic_bit_vector(InputIterator, InputIterator, Allocator = Allocator()) -> basic_bit_vector<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::ranges::input_range R, class Allocator = std::allocator<std::size_t>>
        requires xstd::simple_allocator<Allocator>
basic_bit_vector(std::from_range_t, R&&, Allocator = Allocator()) -> basic_bit_vector<typename std::allocator_traits<Allocator>::value_type, Allocator>;

// The blocks adopted name the block and the allocator both.
template<xstd::unsigned_integer Block, class Allocator>
basic_bit_vector(from_bit_storage_t, std::vector<Block, Allocator>) -> basic_bit_vector<Block, Allocator>;

template<xstd::unsigned_integer Block, class Allocator>
basic_bit_vector(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> basic_bit_vector<Block, Allocator>;

// The adaptor named by its storage stays the door for a std::vector of blocks passed to it directly.
template<xstd::unsigned_integer Block, class Allocator>
bit_sequence_adaptor(from_bit_storage_t, std::vector<Block, Allocator>) -> bit_sequence_adaptor<std::vector<Block, Allocator>>;

template<xstd::unsigned_integer Block, class Allocator>
bit_sequence_adaptor(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> bit_sequence_adaptor<std::vector<Block, Allocator>>;

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, class Allocator>
struct is_range<xstd::basic_bit_vector<Block, Allocator>> : std::false_type
{};

template<class Block, class Allocator>
struct is_tuple_like<xstd::basic_bit_vector<Block, Allocator>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, class Allocator>
struct hash<xstd::basic_bit_vector<Block, Allocator>> : hash<typename xstd::basic_bit_vector<Block, Allocator>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BIT_VECTOR_HPP
