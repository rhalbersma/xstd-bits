//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/bit_set_adaptor.hpp>                 // bit_set_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // storage
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <concepts>                                      // constructible_from
#include <cstddef>                                       // size_t
#include <functional>                                    // hash, less
#include <initializer_list>                              // initializer_list
#include <iterator>                                      // input_iterator, iter_reference_t, sentinel_for
#include <memory>                                        // allocator, allocator_traits
#include <ranges>                                        // from_range, from_range_t, input_range, range_reference_t
#include <type_traits>                                   // false_type, type_identity_t
#include <utility>                                       // forward, move
#include <vector>                                        // vector

namespace xstd {

// The set reading over a heap of blocks: the flagship, and the one name without a qualifier.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_bit_set : public bits::detail::set_adaptor<bits::detail::contiguous_bit_container<std::vector<Block, Allocator>>, bits::detail::storage::owned, basic_bit_set<Block, Allocator>>
{
        using base_type = bits::detail::set_adaptor<bits::detail::contiguous_bit_container<std::vector<Block, Allocator>>, bits::detail::storage::owned, basic_bit_set<Block, Allocator>>;

public:
        using typename base_type::allocator_type;
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons], line for line; key_compare is std::less and holds no state, so a comparator is taken and dropped.
        [[nodiscard]] basic_bit_set() noexcept(noexcept(Allocator())) = default;

        [[nodiscard]] constexpr explicit basic_bit_set(key_compare const& /* comp */, Allocator const& alloc = Allocator()) noexcept
                : base_type(alloc)
        {}

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr basic_bit_set(I first, S last, key_compare const& /* comp */ = key_compare(), Allocator const& alloc = Allocator())
                : base_type(first, last, alloc)
        {}

        template<std::ranges::input_range R>
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare(), Allocator const& alloc = Allocator())
                : base_type(std::from_range, std::forward<R>(rg), alloc)
        {}

        [[nodiscard]] constexpr explicit basic_bit_set(Allocator const& alloc) noexcept
                : base_type(alloc)
        {}

        [[nodiscard]] constexpr basic_bit_set(basic_bit_set const& other, std::type_identity_t<Allocator> const& alloc)
                : base_type(other, alloc)
        {}

        [[nodiscard]] constexpr basic_bit_set(basic_bit_set&& other, std::type_identity_t<Allocator> const& alloc)
                : base_type(std::move(other), alloc)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare(), Allocator const& alloc = Allocator())
                : base_type(il, alloc)
        {}

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr basic_bit_set(I first, S last, Allocator const& alloc)
                : base_type(first, last, alloc)
        {}

        template<std::ranges::input_range R>
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, Allocator const& alloc)
                : base_type(std::from_range, std::forward<R>(rg), alloc)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, Allocator const& alloc)
                : base_type(il, alloc)
        {}

        // flat_set's container constructor under the bit-storage tag: the blocks move in, every bit a position.
        [[nodiscard]] constexpr basic_bit_set(from_bit_storage_t, std::vector<Block, Allocator> blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_bit_set(from_bit_storage_t, std::vector<Block, Allocator> blocks, Allocator const& alloc)
                : base_type(from_bit_storage, std::move(blocks), alloc)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_set& x, basic_bit_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using bit_set = basic_bit_set<std::size_t>;

// std::set's guides, with Block taken from the allocator where one is given and the machine word where none is.
template<std::input_iterator I, std::sentinel_for<I> S>
basic_bit_set(I, S, std::less<std::size_t> = std::less<std::size_t>()) -> basic_bit_set<std::size_t>;

template<std::input_iterator I, std::sentinel_for<I> S, class Allocator>
        requires xstd::unsigned_integer<typename std::allocator_traits<Allocator>::value_type>
basic_bit_set(I, S, std::less<std::size_t>, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::input_iterator I, std::sentinel_for<I> S, class Allocator>
        requires xstd::unsigned_integer<typename std::allocator_traits<Allocator>::value_type>
basic_bit_set(I, S, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::ranges::input_range R>
basic_bit_set(std::from_range_t, R&&, std::less<std::size_t> = std::less<std::size_t>()) -> basic_bit_set<std::size_t>;

template<std::ranges::input_range R, class Allocator>
        requires xstd::unsigned_integer<typename std::allocator_traits<Allocator>::value_type>
basic_bit_set(std::from_range_t, R&&, std::less<std::size_t>, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::ranges::input_range R, class Allocator>
        requires xstd::unsigned_integer<typename std::allocator_traits<Allocator>::value_type>
basic_bit_set(std::from_range_t, R&&, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

basic_bit_set(std::initializer_list<std::size_t>, std::less<std::size_t> = std::less<std::size_t>()) -> basic_bit_set<std::size_t>;

template<class Allocator>
        requires xstd::unsigned_integer<typename std::allocator_traits<Allocator>::value_type>
basic_bit_set(std::initializer_list<std::size_t>, std::less<std::size_t>, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<class Allocator>
        requires xstd::unsigned_integer<typename std::allocator_traits<Allocator>::value_type>
basic_bit_set(std::initializer_list<std::size_t>, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

// The adaptor named by its storage stays the door for a std::vector of blocks passed to it directly.
template<xstd::unsigned_integer Block, class Allocator>
bit_set_adaptor(from_bit_storage_t, std::vector<Block, Allocator>) -> bit_set_adaptor<std::vector<Block, Allocator>>;

template<xstd::unsigned_integer Block, class Allocator>
bit_set_adaptor(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> bit_set_adaptor<std::vector<Block, Allocator>>;

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, class Allocator>
struct is_range<xstd::basic_bit_set<Block, Allocator>> : std::false_type
{};

template<class Block, class Allocator>
struct is_tuple_like<xstd::basic_bit_set<Block, Allocator>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, class Allocator>
struct hash<xstd::basic_bit_set<Block, Allocator>> : hash<typename xstd::basic_bit_set<Block, Allocator>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BIT_SET_HPP
