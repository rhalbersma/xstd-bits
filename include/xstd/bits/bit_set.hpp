//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/bit_set_adaptor.hpp>                     // bit_set_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp>     // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/set_adaptor.hpp>                  // set_adaptor
#include <xstd/bits/from_bit_storage.hpp>                    // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <xstd/misc/concepts/simple_allocator.hpp>           // simple_allocator
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <cstddef>                                           // size_t
#include <functional>                                        // hash, less
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <memory>                                            // allocator, allocator_traits
#include <ranges>                                            // from_range, from_range_t, input_range
#include <type_traits>                                       // false_type, type_identity_t
#include <utility>                                           // forward, move
#include <vector>                                            // vector

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

        // [set.cons], in [set.overview]'s order; key_compare is std::less and holds no state, so a comparator is dropped.
        [[nodiscard]] constexpr basic_bit_set()
                : basic_bit_set(key_compare())
        {}

        [[nodiscard]] constexpr explicit basic_bit_set(key_compare const& /* comp */, Allocator const& a = Allocator())
                : base_type(a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare(), Allocator const& a = Allocator())
                : base_type(first, last, a)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare(), Allocator const& a = Allocator())
                : base_type(std::from_range, std::forward<R>(rg), a)
        {}

        [[nodiscard]] basic_bit_set(basic_bit_set const& x) = default;
        [[nodiscard]] basic_bit_set(basic_bit_set&& x) = default;

        [[nodiscard]] constexpr explicit basic_bit_set(Allocator const& a)
                : base_type(a)
        {}

        [[nodiscard]] constexpr basic_bit_set(basic_bit_set const& x, std::type_identity_t<Allocator> const& a)
                : base_type(x, a)
        {}

        [[nodiscard]] constexpr basic_bit_set(basic_bit_set&& x, std::type_identity_t<Allocator> const& a)
                : base_type(std::move(x), a)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare(), Allocator const& a = Allocator())
                : base_type(il, a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_set(InputIterator first, InputIterator last, Allocator const& a)
                : basic_bit_set(first, last, key_compare(), a)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, Allocator const& a)
                : basic_bit_set(std::from_range, std::forward<R>(rg), key_compare(), a)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, Allocator const& a)
                : basic_bit_set(il, key_compare(), a)
        {}

        ~basic_bit_set() = default;

        auto operator=(basic_bit_set const& x) -> basic_bit_set& = default;
        auto operator=(basic_bit_set&& x) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) -> basic_bit_set& = default;

        // Not in [set.cons]: flat_set's container constructor under the bit-storage tag, every bit of the blocks a position.
        [[nodiscard]] constexpr basic_bit_set(from_bit_storage_t, std::vector<Block, Allocator> blocks) noexcept
                : base_type(from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_bit_set(from_bit_storage_t, std::vector<Block, Allocator> blocks, Allocator const& a)
                : base_type(from_bit_storage, std::move(blocks), a)
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

// [set.overview]'s guides, in its order: Block from the allocator, std::size_t by default, the key being std::size_t.
template<std::input_iterator InputIterator, class Compare = std::less<std::size_t>, class Allocator = std::allocator<std::size_t>>
        requires (not xstd::simple_allocator<Compare>) and xstd::simple_allocator<Allocator>
basic_bit_set(InputIterator, InputIterator, Compare = Compare(), Allocator = Allocator()) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::ranges::input_range R, class Compare = std::less<std::size_t>, class Allocator = std::allocator<std::size_t>>
        requires (not xstd::simple_allocator<Compare>) and xstd::simple_allocator<Allocator>
basic_bit_set(std::from_range_t, R&&, Compare = Compare(), Allocator = Allocator()) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<class Key, class Compare = std::less<std::size_t>, class Allocator = std::allocator<std::size_t>>
        requires (not xstd::simple_allocator<Compare>) and xstd::simple_allocator<Allocator>
basic_bit_set(std::initializer_list<Key>, Compare = Compare(), Allocator = Allocator()) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::input_iterator InputIterator, class Allocator>
        requires xstd::simple_allocator<Allocator>
basic_bit_set(InputIterator, InputIterator, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<std::ranges::input_range R, class Allocator>
        requires xstd::simple_allocator<Allocator>
basic_bit_set(std::from_range_t, R&&, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

template<class Key, class Allocator>
        requires xstd::simple_allocator<Allocator>
basic_bit_set(std::initializer_list<Key>, Allocator) -> basic_bit_set<typename std::allocator_traits<Allocator>::value_type, Allocator>;

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
