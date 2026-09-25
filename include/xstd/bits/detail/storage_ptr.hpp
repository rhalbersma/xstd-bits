//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_STORAGE_PTR_HPP
#define XSTD_BITS_DETAIL_STORAGE_PTR_HPP

#include <xstd/bits/detail/contiguous_block_range.hpp>       // borrowed_block_span
#include <xstd/misc/type_traits/conditional_data_member.hpp> // XSTD_NO_UNIQUE_ADDRESS, conditional_data_member_t
#include <concepts>                                          // same_as
#include <cstddef>                                           // nullptr_t, size_t
#include <memory>                                            // addressof
#include <span>                                              // dynamic_extent
#include <tuple>                                             // tie
#include <type_traits>                                       // conditional_t, is_const_v, remove_const_t

// How a view and its iterators reach storage, as std::views::all would: a pointer, or a copy of what is itself a view.
namespace xstd::bits::detail {

// Storage over words someone else owns: a span and nothing else, so copying it copies no bits.
template<class Bits>
concept view_storage = borrowed_block_span<typename std::remove_const_t<Bits>::block_container_type>;

// A view's copy of storage that is itself a view; its words are not its own, so a const view still writes them.
template<class Bits>
class storage_copy
{
        mutable std::remove_const_t<Bits> m_bits;

public:
        [[nodiscard]] constexpr explicit storage_copy(std::remove_const_t<Bits> const& bits) noexcept
                : m_bits(bits)
        {}

        [[nodiscard]] constexpr explicit(false) storage_copy(Bits* ptr) noexcept // NOLINT(misc-explicit-constructor)
                : m_bits(*ptr)
        {}

        [[nodiscard]] constexpr auto operator*() const noexcept
                -> Bits&
        {
                return m_bits;
        }
};

// An iterator's hold on the words themselves, rebuilt into storage at each use, so it outlives the view it came from.
template<class Bits>
class words_ptr
{
        using bits_type = std::remove_const_t<Bits>;
        using span_type = bits_type::block_container_type;
        using block_type = bits_type::block_type;

        // A span of static extent is a pointer alone, as the storage it rebuilds is.
        static constexpr bool has_count = span_type::extent == std::dynamic_extent;

        block_type* m_data{};
        [[XSTD_NO_UNIQUE_ADDRESS]] conditional_data_member_t<has_count, std::size_t, struct count_tag> m_count{};

        // The const twin, whose conversion below reads these members.
        template<class>
        friend class words_ptr;

        // What -> hands back, holding the rebuilt storage until the end of the full-expression.
        struct arrow
        {
                Bits bits;

                [[nodiscard]] constexpr auto operator->() noexcept
                        -> Bits*
                {
                        return std::addressof(bits);
                }
        };

        [[nodiscard]] constexpr auto rebuild() const noexcept
                -> bits_type
        {
                if constexpr (has_count) {
                        return bits_type(span_type(m_data, m_count));
                } else {
                        return bits_type(span_type(m_data, span_type::extent));
                }
        }

public:
        [[nodiscard]] words_ptr() noexcept = default;

        [[nodiscard]] constexpr explicit(false) words_ptr(Bits* ptr) noexcept // NOLINT(misc-explicit-constructor)
                : m_data(ptr->borrowed_blocks().data())
        {
                if constexpr (has_count) {
                        m_count = ptr->borrowed_blocks().size();
                }
        }

        // A mutable hold converts to its const twin, as the iterators holding it do.
        template<class Mutable>
                requires std::is_const_v<Bits> and std::same_as<Mutable const, Bits>
        [[nodiscard]] constexpr explicit(false) words_ptr(words_ptr<Mutable> other) noexcept // NOLINT(misc-explicit-constructor)
                : m_data(other.m_data)
                , m_count(other.m_count)
        {}

        [[nodiscard]] constexpr auto operator->() const noexcept
                -> arrow
        {
                return {rebuild()};
        }

        // The same words, which is what two iterators into one view share.
        [[nodiscard]] friend constexpr auto operator==(words_ptr lhs, words_ptr rhs) noexcept
                -> bool
        {
                if constexpr (has_count) {
                        return std::tie(lhs.m_data, lhs.m_count) == std::tie(rhs.m_data, rhs.m_count);
                } else {
                        return lhs.m_data == rhs.m_data;
                }
        }

        // Never null as a pointer is: an empty span may have no address and is still a width of zero.
        [[nodiscard]] friend constexpr auto operator==(words_ptr /* ptr */, std::nullptr_t) noexcept
                -> bool
        {
                return false;
        }
};

// What a view holds: a pointer to storage, or a copy of storage that is itself a view.
template<class Bits>
using storage_ref_t = std::conditional_t<view_storage<Bits>, storage_copy<Bits>, Bits*>;

// What an iterator or proxy holds: a pointer to storage, or a hold on the words that storage is a view of.
template<class Bits>
using storage_ptr_t = std::conditional_t<view_storage<Bits>, words_ptr<Bits>, Bits*>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_STORAGE_PTR_HPP
