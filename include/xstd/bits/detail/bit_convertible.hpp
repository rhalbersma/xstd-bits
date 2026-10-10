//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_CONVERTIBLE_HPP
#define XSTD_BITS_DETAIL_BIT_CONVERTIBLE_HPP

#include <xstd/bits/bit_concepts/bit_constructible_from.hpp> // bit_constructible_from
#include <xstd/bits/detail/bit_blocks_capacity.hpp>          // bit_blocks_capacity_v
#include <xstd/bits/detail/bit_layout.hpp>                   // bit_bytes, bit_layout, byte_count, bytes_bits, copy_bits
#include <xstd/bits/detail/bit_width.hpp>                    // bit_width_v
#include <xstd/bits/detail/ownership.hpp>                    // owned_bits_t, owner, reads, set_reading_tag, storage_access, view
#include <xstd/bits/detail/shift.hpp>                        // partial_block_mask, shl
#include <xstd/bits/from_blocks.hpp>                         // from_blocks
#include <xstd/ints/limits.hpp>                              // numeric_limits
#include <algorithm>                                         // min
#include <array>                                             // array
#include <bit>                                               // popcount
#include <concepts>                                          // same_as
#include <cstddef>                                           // size_t
#include <new>                                               // bad_alloc
#include <span>                                              // dynamic_extent, span
#include <stdexcept>                                         // overflow_error
#include <type_traits>                                       // is_array_v, is_const_v, is_integral_v, is_rvalue_reference_v, remove_cvref_t, remove_reference_t
#include <utility>                                           // declval, forward

// What xstd::bit_convert asks of its two ends, and the copy between them: position i to position i, at any widths.
namespace xstd::bits::detail {

// A width fixed by the type: one of our owners or full-width views, a block or an array of them, or a std::bitset.
template<class T>
concept fixed_width =
        bit_width_v<T> != std::dynamic_extent and
        (owner<T> or view<T> or bit_layout<T, bit_width_v<T>>);

// What a fixed width is written into: not a view, which writes bits it does not own, nor an array no function returns.
template<class T>
concept fixed_target = fixed_width<T> and (not view<T>) and (not std::is_array_v<T>);

// The bytes of a fixed width, byte j holding the positions [8j, 8j + 8): ours through their storage.
template<fixed_width T>
[[nodiscard]] constexpr auto fixed_bytes(T const& from) noexcept
        -> std::array<unsigned char, byte_count<bit_width_v<T>>>
{
        if constexpr (owner<T> or view<T>) {
                return storage_access::bits(from).template to_bytes<byte_count<bit_width_v<T>>>();
        } else {
                return bit_bytes<bit_width_v<T>>(from);
        }
}

// A fixed width out of its bytes, through the tag where it is ours.
template<fixed_target To>
[[nodiscard]] constexpr auto from_fixed_bytes(std::array<unsigned char, byte_count<bit_width_v<To>>> const& bytes) noexcept
        -> To
{
        constexpr auto N = bit_width_v<To>;
        if constexpr (N == 0UZ) {
                return To();
        } else if constexpr (owner<To>) {
                return To(xstd::from_blocks, bytes);
        } else {
                return bytes_bits<To, N>(bytes);
        }
}

// The positions a run of bytes holds.
[[nodiscard]] constexpr auto count_bytes(std::span<unsigned char const> bytes) noexcept
        -> std::size_t
{
        auto count = 0UZ;
        for (auto const byte : bytes) {
                count += static_cast<std::size_t>(std::popcount(byte));
        }
        return count;
}

// What a source holds, as its width, its count and a copy of its positions; declared for an extension to specialize.
template<class T>
struct bit_source;

// A view that starts inside its storage, at an offset and a size of its own.
template<class T>
concept windowed_view = view<T> and requires { requires T::is_windowed; };

// A whole view of a run-time width: its storage from position zero, which is its owner's, read as that owner reads it.
template<class T>
concept run_time_view = view<T> and (not fixed_width<T>) and (not windowed_view<T>);

// Owners and whole views, of either reading: a sequence's width is its size, a set's its whole blocks, capped.
template<class T>
        requires owner<T> or run_time_view<T>
struct bit_source<T>
{
        [[nodiscard]] static constexpr auto width(T const& from) noexcept
                -> std::size_t
        {
                auto const& bits = storage_access::bits(from);
                if constexpr (reads<T, set_reading_tag>) {
                        return std::ranges::min(bits.num_blocks() * bits.bits_per_block, bits.max_size());
                } else {
                        return bits.size();
                }
        }

        [[nodiscard]] static constexpr auto count(T const& from) noexcept
                -> std::size_t
        {
                return storage_access::bits(from).count();
        }

        [[nodiscard]] static constexpr auto blocks(T const& from) noexcept
        {
                return storage_access::bits(from).blocks();
        }

        template<class U>
        static constexpr auto copy(T const& from, std::span<U> dst) noexcept
                -> void
        {
                copy_bits(blocks(from), dst);
        }
};

// Anything else of a fixed width, a view or a std::bitset among them, read through its bytes.
template<class T>
        requires fixed_width<T> and (not owner<T>)
struct bit_source<T>
{
        static constexpr auto N = bit_width_v<T>;

        [[nodiscard]] static constexpr auto width(T const& /* from */) noexcept
                -> std::size_t
        {
                return N;
        }

        [[nodiscard]] static constexpr auto count(T const& from) noexcept
                -> std::size_t
        {
                return count_bytes(blocks(from));
        }

        [[nodiscard]] static constexpr auto blocks(T const& from) noexcept
                -> std::array<unsigned char, byte_count<N>>
        {
                return fixed_bytes(from);
        }

        template<class U>
        static constexpr auto copy(T const& from, std::span<U> dst) noexcept
                -> void
        {
                copy_bits(blocks(from), dst);
        }
};

// A block as another block type, through unsigned where it would promote to int and choose a signed constructor.
template<class U, class Block>
[[nodiscard]] constexpr auto as_block(Block b) noexcept
        -> U
{
        if constexpr (std::is_integral_v<Block> and sizeof(Block) < sizeof(unsigned)) {
                return static_cast<U>(static_cast<unsigned>(b));
        } else {
                return static_cast<U>(b);
        }
}

// A window, read a block at a time from its offset, so that position zero of the copy is its first position.
template<windowed_view T>
struct bit_source<T>
{
        [[nodiscard]] static constexpr auto width(T const& from) noexcept
                -> std::size_t
        {
                return from.size();
        }

        [[nodiscard]] static constexpr auto count(T const& from) noexcept
                -> std::size_t
        {
                return from.count();
        }

        // Steps of the narrower block, so that each one lands inside a single target block.
        template<class U>
        static constexpr auto copy(T const& from, std::span<U> dst) noexcept
                -> void
        {
                auto const& bits         = storage_access::bits(from);
                using block_type         = std::remove_cvref_t<decltype(bits)>::block_type;
                constexpr auto to_digits = static_cast<std::size_t>(xstd::numeric_limits<U>::digits);
                constexpr auto step      = std::ranges::min(std::remove_cvref_t<decltype(bits)>::bits_per_block, to_digits);
                auto const n             = std::ranges::min(from.size(), std::size(dst) * to_digits);
                for (auto p = 0UZ; p < n; p += step) {
                        auto const chunk = static_cast<block_type>(bits.block_at(storage_access::offset(from) + p) & partial_block_mask<block_type>(std::ranges::min(step, n - p)));
                        auto& block      = dst[p / to_digits];
                        block            = static_cast<U>(block | shl(as_block<U>(chunk), p % to_digits));
                }
        }
};

template<class T>
concept bit_convert_source = requires (T const& from, std::span<unsigned char> dst) {
        { bit_source<T>::width(from) } -> std::same_as<std::size_t>;
        { bit_source<T>::count(from) } -> std::same_as<std::size_t>;
        bit_source<T>::copy(from, dst);
};

// A set target covers the source's width in whole blocks, up to the capacity it has; a sequence takes the width.
template<class To>
[[nodiscard]] constexpr auto target_width(std::size_t width) noexcept
        -> std::size_t
{
        using bits_type = owned_bits_t<To>;
        if constexpr (reads<To, set_reading_tag>) {
                auto const whole = bits_type::blocks_for(width) * bits_type::bits_per_block;
                if constexpr (bits_type::has_static_capacity) {
                        return std::ranges::min(whole, bits_type::static_capacity());
                } else {
                        return whole;
                }
        } else {
                return width;
        }
}

// A set capped below its source's width that counts fewer keys lost one, which is what its own insert refuses.
constexpr auto refuse_lost_keys(std::size_t capped_width, std::size_t kept, std::size_t width, std::size_t count)
        -> void
{
        if (capped_width < width and kept != count) {
                throw std::bad_alloc();
        }
}

// Sized first, so a target that cannot hold the width throws what its own growth throws before a block is written.
template<class To, class From>
[[nodiscard]] constexpr auto copy_blocks(From const& from)
        -> To
{
        using source     = bit_source<From>;
        auto to          = To();
        auto& bits       = storage_access::bits(to);
        auto const width = source::width(from);
        bits.resize(target_width<To>(width));
        source::copy(from, bits.blocks());
        bits.erase_unused();
        if constexpr (reads<To, set_reading_tag> and owned_bits_t<To>::has_static_capacity) {
                refuse_lost_keys(bits.size(), bits.count(), width, source::count(from));
        }
        return to;
}

// Into a fixed width, of the same width: the bytes cross whole, and nothing can be lost.
template<fixed_target To, fixed_width From>
        requires (bit_width_v<To> == bit_width_v<From>)
[[nodiscard]] constexpr auto convert_fixed(From const& from) noexcept
        -> To
{
        return from_fixed_bytes<To>(fixed_bytes(from));
}

// A position the fixed target cannot hold was set, which is what std::bitset's to_ulong throws for.
constexpr auto refuse_lost_positions(std::size_t kept, std::size_t count)
        -> void
{
        if (kept != count) {
                throw std::overflow_error("xstd::bit_convert: a position at or past the target's width is set");
        }
}

// Into a fixed width, from a run-time one: zero-extended where it is wider, and refused for a position past it.
template<fixed_target To, class From>
[[nodiscard]] constexpr auto narrow_blocks(From const& from)
        -> To
{
        constexpr auto N = bit_width_v<To>;
        using source     = bit_source<From>;
        auto bytes       = std::array<unsigned char, byte_count<N>>();
        source::copy(from, std::span<unsigned char>(bytes));
        if constexpr (N % bits_per_byte != 0UZ) {
                bytes.back() = static_cast<unsigned char>(bytes.back() & ((1U << (N % bits_per_byte)) - 1U));
        }
        refuse_lost_positions(count_bytes(bytes), source::count(from));
        return from_fixed_bytes<To>(bytes);
}

// A foreign target an extension converts into, declared for it to specialize.
template<class To>
struct bit_target;

template<class To, class From>
concept foreign_convertible = requires (From const& from) {
        { bit_target<To>::convert(from) } -> std::same_as<To>;
};

// An rvalue whose blocks To takes as they are, moved rather than copied, into a capacity that holds all of them.
template<class To, class From>
concept adopts_from =
        std::is_rvalue_reference_v<From&&> and
        (not std::is_const_v<std::remove_reference_t<From>>) and
        requires (From&& from) { std::forward<From>(from).extract(); } and
        xstd::bit_constructible_from<To, decltype(std::declval<From&&>().extract())> and
        ((not owned_bits_t<To>::has_static_capacity) or owned_bits_t<To>::static_capacity() == bit_blocks_capacity_v<typename owned_bits_t<To>::block_container_type>);

// The blocks move over and the source is left at width zero; adopted whole, a sequence then takes the source's width.
template<class To, class From>
[[nodiscard]] constexpr auto adopt_blocks(From&& from)
        -> To
{
        auto const width = bit_source<std::remove_cvref_t<From>>::width(from);
        auto to          = To(xstd::from_blocks, std::forward<From>(from).extract());
        if constexpr (not reads<To, set_reading_tag>) {
                storage_access::bits(to).resize(width);
        }
        return to;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_CONVERTIBLE_HPP
