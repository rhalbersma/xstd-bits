//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BIT_CONTAINER_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BIT_CONTAINER_HPP

#include <xstd/bits/detail/allocator_base_type.hpp>          // allocator_base_type
#include <xstd/bits/detail/contiguous_block_range.hpp>       // contiguous_block_range
#include <xstd/bits/detail/intrin.hpp>                       // countl_zero, countr_zero, popcount
#include <xstd/bits/detail/pred.hpp>                         // intersects, is_subset_of, not_equal_to
#include <xstd/bits/detail/shift.hpp>                        // shl, shr
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/ints/cstdlib/div.hpp>                         // div, div_result
#include <xstd/ints/limits.hpp>                              // numeric_limits
#include <xstd/ints/memory.hpp>                              // align_up
#include <xstd/misc/type_traits/conditional_data_member.hpp> // XSTD_NO_UNIQUE_ADDRESS, conditional_data_member_t
#include <boost/hash2/hash_append_fwd.hpp>                   // hash_append, hash_append_tag
#include <algorithm>                                         // all_of, any_of, equal, fill, fill_n, find_if, fold_left, lexicographical_compare_three_way, max, min, shift_left, shift_right
#include <array>                                             // array
#include <bit>                                               // endian
#include <cassert>                                           // assert
#include <compare>                                           // strong_ordering
#include <concepts>                                          // same_as
#include <cstddef>                                           // byte, ptrdiff_t, size_t, to_integer
#include <cstring>                                           // memcpy
#include <format>                                            // format
#include <functional>                                        // plus
#include <iterator>                                          // distance, forward_iterator, input_iterator, prev
#include <limits>                                            // numeric_limits
#include <ranges>                                            // begin, drop, iota, rbegin, rend, size, swap, transform, zip
                                                             // (views::drop_last when P22014R2 is accepted)
#include <source_location>                                   // source_location
#include <xstd/bits/detail/bit_castable.hpp>                 // bit_bytes, bit_castable, byte_count, bytes_bits, container_source
#include <span>                                              // dynamic_extent, span
#include <stdexcept>                                         // length_error
#include <type_traits>                                       // conditional_t, is_const_v, remove_reference_t
#include <utility>                                           // exchange, move, pair

namespace xstd::detail::bits {

// Floored at one so a zero width still names a block.
template<xstd::unsigned_integer Block, std::size_t N>
inline constexpr auto num_blocks_v = std::ranges::max(
        align_up(N, static_cast<std::size_t>(xstd::numeric_limits<Block>::digits)) /
                static_cast<std::size_t>(xstd::numeric_limits<Block>::digits),
        1UZ
);

// The one vehicle: it owns the unused-tail invariant, and has no iterators.
template<contiguous_block_range Blocks, std::size_t N = std::dynamic_extent>
class contiguous_bit_container : public detail::bits::allocator_base_type<Blocks>
{
public:
        using block_type = std::ranges::range_value_t<Blocks>;

        static constexpr auto bits_per_block = static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits);

        // Derived rather than written as an 8: a block's digits over its bytes is the bits in a byte.
        static constexpr auto bits_per_byte = bits_per_block / sizeof(block_type);
        static constexpr auto has_static_size = N != std::dynamic_extent;

        // A run-time width over a capacity the type carries: the qualified capacity() call is the discriminator.
        static constexpr auto has_static_capacity = requires { Blocks::capacity(); };

        // The capacity in bits; a function and not a variable, so the std::vector column never instantiates it.
        [[nodiscard]] static constexpr auto static_capacity() noexcept
                -> std::size_t
        {
                if constexpr (has_static_capacity) {
                        return Blocks::capacity() * bits_per_block;
                } else {
                        return 0UZ;
                }
        }

        // The width as a type, dynamic_extent where there is none: asked before an object exists.
        static constexpr std::size_t extent = N;

        // The width is this container's, so a dynamic one answers zero here rather than compute byte_count of SIZE_MAX.
        static constexpr auto bit_extent = has_static_size ? N : 0UZ;

        // The two shapes a reading asks for: either family, or the field-of-bits family where integers have a door.
        template<class B>
        static constexpr auto exchanges_bits = has_static_size and bit_castable<B, bit_extent>;

        // A field of bits is anything but the bare scalar, which is left out only because it has its own door.
        template<class B>
        static constexpr auto exchanges_bits_as_field =
                has_static_size and (block_range_source<B, bit_extent> or container_source<B, bit_extent>);

        // How many blocks a run-time width needs, floored at one; total over every size_t, as boost spells it.
        [[nodiscard]] static constexpr auto blocks_for(std::size_t n) noexcept
                -> std::size_t
        {
                return std::ranges::max((n / bits_per_block) + (n % bits_per_block != 0UZ ? 1UZ : 0UZ), 1UZ);
        }

        // The two ceilings the readings choose between, neither enforced here; the blocks' own is narrower.
        static constexpr auto max_num_blocks = std::numeric_limits<std::size_t>::max() / bits_per_block;
        static constexpr auto max_width = max_num_blocks * bits_per_block;

        // A width whose positions a difference_type can all name, which is what std::vector<bool> refuses against.
        static constexpr auto max_addressable_num_blocks = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / bits_per_block;
        static constexpr auto max_addressable_width = max_addressable_num_blocks * bits_per_block;

private:
        static constexpr auto static_num_bits = has_static_size ? align_up(N, bits_per_block) : 0UZ;
        static constexpr auto static_num_blocks = has_static_size ? std::ranges::max(static_num_bits / bits_per_block, 1UZ) : 0UZ;
        static constexpr auto static_last_block = static_num_blocks - 1UZ;

        static constexpr auto left_bit = bits_per_block - 1UZ;
        static constexpr auto unit = static_cast<block_type>(1);
        static constexpr auto zero = static_cast<block_type>(0);
        static constexpr auto ones = static_cast<block_type>(-1);

        // Width zero named, not computed: MSVC folds both ?: arms and answers C4293.
        static constexpr auto static_num_unused_bits = has_static_size ? static_num_bits - N : 0UZ;
        static constexpr auto static_used_bits = has_static_size and N == 0 ? zero : shr(ones, static_num_unused_bits);
        static constexpr auto static_unused_bits = static_cast<block_type>(~static_used_bits);
        static constexpr auto static_has_unused_bits = has_static_size and static_used_bits != ones;

        // An NSDMI, not extent-constrained constructors: vector starts empty.
        [[nodiscard]] static constexpr auto make_blocks(std::size_t n [[maybe_unused]])
                -> Blocks
        {
                if constexpr (has_static_size) {
                        return Blocks{};
                } else {
                        return Blocks(blocks_for(n));
                }
        }

        // The width is a size_t unless the blocks out-align one, when it fills what would be padding.
        using width_type = std::conditional_t<(alignof(std::size_t) >= alignof(Blocks)), std::size_t, block_type>;
        static_assert(sizeof(width_type) >= sizeof(std::size_t) and alignof(width_type) >= alignof(Blocks));

        // Dynamic widths only; the tag keeps the absent member distinct from any other in an enclosing layout.
        [[XSTD_NO_UNIQUE_ADDRESS]]
        conditional_data_member_t<not has_static_size, width_type, struct size_tag> m_size{};

        Blocks m_blocks = make_blocks(0UZ);

public:
        [[nodiscard]] contiguous_bit_container() = default;

        // The width is a constructor argument exactly when it is not a template argument.
        [[nodiscard]] constexpr explicit contiguous_bit_container(std::size_t n)
                requires (not has_static_size)
                : m_size(n)
                , m_blocks(make_blocks(n))
        {}

        // boost's allocator arguments, deduced and matched, so a storage without one has no such constructor.
        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr explicit contiguous_bit_container(Alloc const& alloc)
                : m_blocks(blocks_for(0UZ), alloc)
        {}

        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr contiguous_bit_container(std::size_t n, Alloc const& alloc)
                : m_size(n)
                , m_blocks(blocks_for(n), alloc)
        {}

        // [container.alloc.reqmts]'s allocator-extended copy and move; the moved-from is left empty.
        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr contiguous_bit_container(contiguous_bit_container const& other, Alloc const& alloc)
                : m_size(other.m_size)
                , m_blocks(other.m_blocks, alloc)
        {}

        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr contiguous_bit_container(contiguous_bit_container&& other, Alloc const& alloc)
                : m_size(std::exchange(other.m_size, 0UZ))
                , m_blocks(std::move(other.m_blocks), alloc)
        {
                other.m_blocks.clear();
        }

        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires requires (Blocks const& b) { b.get_allocator(); }
        {
                return m_blocks.get_allocator();
        }

        // Memberwise, width first: the unused bits are kept clear, so the blocks compare as the bits do.
        [[nodiscard]] friend auto operator==(contiguous_bit_container const&, contiguous_bit_container const&) noexcept -> bool = default;

        // The set reading's equality, where width is capacity: a hidden friend, neither value being the subject.
        [[nodiscard]] friend constexpr auto set_equal(contiguous_bit_container const& x, contiguous_bit_container const& y) noexcept
                -> bool
        {
                if constexpr (has_static_size) {
                        // One width, so holding the same positions and being equal are the same statement.
                        return x == y;
                } else {
                        // ranges::equal over the shared prefix as an iterator pair, which lowers to a memcmp.
                        auto const shared = static_cast<std::ptrdiff_t>(std::ranges::min(x.num_blocks(), y.num_blocks()));
                        auto const xf = std::ranges::begin(x.m_blocks);
                        auto const yf = std::ranges::begin(y.m_blocks);
                        return std::ranges::equal(xf, xf + shared, yf, yf + shared) and
                               (x.num_blocks() < y.num_blocks()
                                        ? not y.any_block_set(x.num_blocks(), y.num_blocks())
                                        : not x.any_block_set(y.num_blocks(), x.num_blocks()));
                }
        }

        // No operator<=>: pure storage names all three orderings and picks none, each a hidden friend.
        [[nodiscard]] friend constexpr auto set_lexicographical_compare_three_way(contiguous_bit_container const& x [[maybe_unused]], contiguous_bit_container const& y [[maybe_unused]]) noexcept
                -> std::strong_ordering
        {
                if constexpr (has_static_size and N == 0) {
                        return std::strong_ordering::equal;
                } else if constexpr (has_static_size and N == 1) {
                        // One position, so the loser is empty and any_above is constantly false.
                        return x.test(0UZ) <=> y.test(0UZ);
                } else {
                        if constexpr (not has_static_size) {
                                if (x.size() != y.size()) {
                                        return x.padded_set_three_way(y);
                                }
                        }
                        auto const [index, diff] = x.first_difference(y);
                        if (diff == zero) {
                                return std::strong_ordering::equal;
                        }
                        auto const offset = detail::bits::countr_zero(diff);
                        if (detail::bits::intersects(x.m_blocks[index], shl(unit, offset))) {
                                return y.any_above(index, offset) ? std::strong_ordering::less : std::strong_ordering::greater;
                        }
                        return x.any_above(index, offset) ? std::strong_ordering::greater : std::strong_ordering::less;
                }
        }

        // The sequence reading a word at a time: whoever holds the lowest differing position is greater.
        [[nodiscard]] friend constexpr auto sequence_lexicographical_compare_three_way(contiguous_bit_container const& x [[maybe_unused]], contiguous_bit_container const& y [[maybe_unused]]) noexcept
                -> std::strong_ordering
        {
                if constexpr (has_static_size and N == 0) {
                        return std::strong_ordering::equal;
                } else {
                        if constexpr (not has_static_size) {
                                if (x.size() != y.size()) {
                                        return x.padded_sequence_three_way(y);
                                }
                        }
                        auto const [index, diff] = x.first_difference(y);
                        if (diff == zero) {
                                return std::strong_ordering::equal;
                        }
                        auto const offset = detail::bits::countr_zero(diff);
                        return detail::bits::intersects(x.m_blocks[index], shl(unit, offset))
                                       ? std::strong_ordering::greater
                                       : std::strong_ordering::less;
                }
        }

        // The bitset reading a word at a time: the bit string is the blocks from the top down, tail clear.
        [[nodiscard]] friend constexpr auto string_lexicographical_compare_three_way(contiguous_bit_container const& x, contiguous_bit_container const& y) noexcept
                -> std::strong_ordering
        {
                assert(x.size() == y.size());
                return std::lexicographical_compare_three_way(
                        std::ranges::rbegin(x.m_blocks), std::ranges::rend(x.m_blocks),
                        std::ranges::rbegin(y.m_blocks), std::ranges::rend(y.m_blocks)
                );
        }

        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, contiguous_bit_container const* v) noexcept
                -> void
        {
                boost::hash2::hash_append(h, f, v->m_blocks);
        }

        // Saturated at the top of size_t rather than wrapped: a wrapped sum passes the ceiling it should fail.
        [[nodiscard]] static constexpr auto check_width(std::size_t n)
                -> std::size_t
        {
                if (n > max_width) {
                        throw length_error(n);
                }
                return n;
        }

        // The same refusal against the other ceiling: [container.reqmts] puts a sequence's at max_size().
        [[nodiscard]] static constexpr auto check_addressable_width(std::size_t n)
                -> std::size_t
        {
                if (n > max_addressable_width) {
                        throw addressable_length_error(n);
                }
                return n;
        }

        [[nodiscard]] static constexpr auto width_sum(std::size_t base, std::size_t count) noexcept
                -> std::size_t
        {
                constexpr auto top = std::numeric_limits<std::size_t>::max();
                return count > top - base ? top : base + count;
        }

        // In bits: the width, or the widest whole number of blocks the blocks and the address space allow.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size) {
                        return N;
                } else {
                        return std::ranges::min(m_blocks.max_size(), max_num_blocks) * bits_per_block;
                }
        }

        // boost::dynamic_bitset's answer, which saturates where the one above clamps; said as a sum, not a branch.
        [[nodiscard]] constexpr auto saturating_max_size() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size) {
                        return N;
                } else {
                        auto const saturates = static_cast<std::size_t>(m_blocks.max_size() > max_num_blocks);
                        return max_size() + (saturates * (bits_per_block - 1UZ));
                }
        }

        // std::vector<bool>'s answer, which clamps further: a difference_type counts a random access range.
        [[nodiscard]] constexpr auto addressable_max_size() const noexcept
                -> std::size_t
        {
                return std::ranges::min(max_size(), max_addressable_width);
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size) {
                        return N;
                } else {
                        return static_cast<std::size_t>(m_size);
                }
        }

        [[nodiscard]] constexpr auto num_blocks() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size) {
                        return static_num_blocks;
                } else {
                        return std::ranges::size(m_blocks);
                }
        }

        // The block, and not operator[]: a subscript on a bit container means a bit, which test() answers.
        [[nodiscard]] constexpr auto block(std::size_t i) const noexcept
                -> block_type
        {
                assert(i < num_blocks());
                return m_blocks[i];
        }

        // The write side, a reference rather than a setter: the caller restores the invariant with erase_unused.
        [[nodiscard]] constexpr auto block(std::size_t i) noexcept
                -> block_type&
        {
                assert(i < num_blocks());
                return m_blocks[i];
        }

        // block(i) as a range, so a caller writing every block writes them in one call.
        [[nodiscard]] constexpr auto blocks() noexcept
                -> std::span<block_type>
        {
                return {m_blocks.data(), num_blocks()};
        }

        [[nodiscard]] constexpr auto blocks() const noexcept
                -> std::span<block_type const>
        {
                return {m_blocks.data(), num_blocks()};
        }

        // Public, because restoring the invariant belongs to whoever wrote the blocks that broke it.
        constexpr auto erase_unused() noexcept
                -> void
        {
                if constexpr (has_static_size and static_has_unused_bits) {
                        m_blocks[static_last_block] &= static_used_bits;
                        assert(not detail::bits::intersects(m_blocks[static_last_block], static_unused_bits));
                } else if constexpr (not has_static_size) {
                        // The run-time twin of the arm above: what the other asks the compiler, this asks the value.
                        if (has_unused_bits()) {
                                m_blocks[last_block()] &= used_bits();
                        }
                }
        }

        // The bits as bytes and back, said in shifts: byte j holds [8j, 8j + 8), least significant bit first.
        template<std::size_t E>
        static constexpr auto shared_bytes = std::ranges::min(E, static_num_blocks * sizeof(block_type));

        // On a little-endian target a straight copy of those bytes is the same answer, and not byte at a time.
        static constexpr auto bytes_copy_as_blocks = std::endian::native == std::endian::little;

        // The shifts, in one place: they say where a position goes rather than assume a byte order.
        template<std::size_t E>
        constexpr auto assign_bytes_by_shifts(std::array<std::byte, E> const& bytes) noexcept
                -> void
        {
                for (auto j = 0UZ; j < shared_bytes<E>; ++j) {
                        auto const byte = static_cast<block_type>(std::to_integer<unsigned char>(bytes[j]));
                        auto& block = m_blocks[j / sizeof(block_type)];
                        block = static_cast<block_type>(block | shl(byte, bits_per_byte * (j % sizeof(block_type))));
                }
        }

        template<std::size_t E>
        constexpr auto to_bytes_by_shifts(std::array<std::byte, E>& bytes) const noexcept
                -> void
        {
                for (auto j = 0UZ; j < shared_bytes<E>; ++j) {
                        auto const block = shr(m_blocks[j / sizeof(block_type)], bits_per_byte * (j % sizeof(block_type)));
                        bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(block));
                }
        }

        template<std::size_t E>
                requires has_static_size
        constexpr auto assign_bytes(std::array<std::byte, E> const& bytes) noexcept
                -> void
        {
                std::ranges::fill(m_blocks, zero);

                // if constexpr, not a loop running zero times: an unenterable loop is a line no test reaches.
                if constexpr (shared_bytes<E> > 0UZ) {
                        // The shifts answer the two cases a copy cannot: a constant expression, and big-endian.
                        if consteval {
                                assign_bytes_by_shifts(bytes);
                        } else {
                                if constexpr (bytes_copy_as_blocks) {
                                        std::memcpy(m_blocks.data(), bytes.data(), shared_bytes<E>);
                                } else {
                                        assign_bytes_by_shifts(bytes);
                                }
                        }
                }

                // The source keeps its own tail clear, so this restores nothing where the two widths agree.
                erase_unused();
        }

        // A whole field of bits in and out over the two byte primitives, the width being known here.
        template<class B>
                requires exchanges_bits<B>
        constexpr auto assign_bits(B const& b) noexcept
                -> void
        {
                assign_bytes(bit_bytes<bit_extent>(b));
        }

        template<class B>
                requires exchanges_bits<B>
        [[nodiscard]] constexpr auto to_bits() const noexcept
                -> B
        {
                return bytes_bits<B, bit_extent>(to_bytes<byte_count<bit_extent>>());
        }

        template<std::size_t E>
                requires has_static_size
        [[nodiscard]] constexpr auto to_bytes() const noexcept
                -> std::array<std::byte, E>
        {
                auto bytes = std::array<std::byte, E>();
                if constexpr (shared_bytes<E> > 0UZ) {
                        if consteval {
                                to_bytes_by_shifts(bytes);
                        } else {
                                if constexpr (bytes_copy_as_blocks) {
                                        std::memcpy(bytes.data(), m_blocks.data(), shared_bytes<E>);
                                } else {
                                        to_bytes_by_shifts(bytes);
                                }
                        }
                }
                return bytes;
        }

        // A word at any position: the bits [n, n + bits_per_block), the clear tail and nothing beyond.
        [[nodiscard]] constexpr auto block_at(std::size_t n) const noexcept
                -> block_type
        {
                auto const [index, offset] = index_offset(n);
                assert(index < num_blocks());
                if (offset == 0UZ or index == last_block()) {
                        return shr(m_blocks[index], offset);
                }
                return straddled_block(index, bits_per_block - offset, offset);
        }

        // The write side of block_at, masked: the mask stays inside size(), so the padding above it is untouched.
        constexpr auto block_at(std::size_t n, block_type value, block_type mask) noexcept
                -> void
        {
                auto const [index, offset] = index_offset(n);
                assert(index < num_blocks());
                assert(n + bits_per_block <= size() or shr(mask, size() - n) == zero);
                auto const bits = static_cast<block_type>(value & mask);

                // Each step lands back in block_type: a promoted operand is what bugprone-signed-bitwise reads.
                auto const low_kept = static_cast<block_type>(m_blocks[index] & static_cast<block_type>(~shl(mask, offset)));
                m_blocks[index] = static_cast<block_type>(low_kept | shl(bits, offset));
                if (offset != 0UZ and index != last_block()) {
                        auto const shift = bits_per_block - offset;
                        auto const high_kept = static_cast<block_type>(m_blocks[index + 1UZ] & static_cast<block_type>(~shr(mask, shift)));
                        m_blocks[index + 1UZ] = static_cast<block_type>(high_kept | shr(bits, shift));
                }
        }

        // boost's ranged forms through block_at; the precondition is a subtraction, n + len being what wraps.
        constexpr auto set(std::size_t n, std::size_t len, bool value) noexcept
                -> contiguous_bit_container&
        {
                assert(n <= size() and len <= size() - n);
                for_each_block(n, len, [&](std::size_t pos, block_type mask) -> void { block_at(pos, value ? ones : zero, mask); });
                return *this;
        }

        constexpr auto flip(std::size_t n, std::size_t len) noexcept
                -> contiguous_bit_container&
        {
                assert(n <= size() and len <= size() - n);
                for_each_block(n, len, [&](std::size_t pos, block_type mask) -> void { block_at(pos, static_cast<block_type>(~block_at(pos)), mask); });
                return *this;
        }

        [[nodiscard]] constexpr auto find_front() const noexcept
                -> std::size_t
        {
                assert(any());
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return detail::bits::countr_zero(m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return m_blocks[0] != zero ? detail::bits::countr_zero(m_blocks[0]) : detail::bits::countr_zero(m_blocks[1]) + bits_per_block;
                } else {
                        // A while, not a for: any() makes a for's exit untestable.
                        auto i = 0UZ;
                        while (m_blocks[i] == zero) {
                                assert(i != last_block());
                                ++i;
                        }
                        return (bits_per_block * i) + detail::bits::countr_zero(m_blocks[i]);
                }
        }

        [[nodiscard]] constexpr auto find_back() const noexcept
                -> std::size_t
        {
                assert(any());
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return last_bit() - detail::bits::countl_zero(m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return m_blocks[1] != zero ? last_bit() - detail::bits::countl_zero(m_blocks[1]) : left_bit - detail::bits::countl_zero(m_blocks[0]);
                } else {
                        // The mirror of find_front, counting up from block i's base to drop the reversed range's term.
                        auto i = last_block();
                        while (m_blocks[i] == zero) {
                                assert(i != 0);
                                --i;
                        }
                        return (bits_per_block * i) + left_bit - detail::bits::countl_zero(m_blocks[i]);
                }
        }

        // Its own 0, and the same instructions the hand-written version emitted.
        [[nodiscard]] constexpr auto find_first() const noexcept
                -> std::size_t
        {
                return inclusive_find_next(0UZ);
        }

        [[nodiscard]] constexpr auto find_last() const noexcept
                -> std::size_t
        {
                return size();
        }

        // The primitive: inclusive, so both derivations are + 1 and nothing wraps.
        [[nodiscard]] constexpr auto inclusive_find_next(std::size_t n) const noexcept
                -> std::size_t
        {
                assert(n <= size());
                if (n == size()) {
                        return size();
                }
                if constexpr (has_static_size and static_num_blocks == 1) {
                        if (auto const block = shr(m_blocks[0], n); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        // Indexed, not branched: an if cost 10 instructions at -O3.
                        auto const [index, offset] = index_offset(n);
                        if (auto const block = shr(m_blocks[index], offset); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                        if (index == 0 and m_blocks[1] != zero) {
                                return bits_per_block + detail::bits::countr_zero(m_blocks[1]);
                        }
                } else {
                        // No offset != 0 guard: >> 0 is the identity.
                        auto [index, offset] = index_offset(n);
                        if (auto const block = shr(m_blocks[index], offset); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                        ++index;
                        n += bits_per_block - offset;
                        // A plain index walk: drop + find_if made distance() recover the index.
                        for (auto i = index; i < num_blocks(); ++i) {
                                if (auto const block = m_blocks[i]; block != zero) {
                                        return n + detail::bits::countr_zero(block) + (bits_per_block * (i - index));
                                }
                        }
                }
                return size();
        }

        [[nodiscard]] constexpr auto exclusive_find_next(std::size_t n) const noexcept
                -> std::size_t
        {
                assert(is_valid(n));
                return inclusive_find_next(n + 1);
        }

        // Deliberately NOT total: three instructions cheaper, and rend() supplies the guard.
        [[nodiscard]] constexpr auto exclusive_find_prev(std::size_t n) const noexcept
                -> std::size_t
        {
                // States 1 <= n <= size() in one predicate, 0 - 1 being SIZE_MAX.
                assert(is_valid(n - 1));
                assert(any());
                --n;
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return n - detail::bits::countl_zero(shl(m_blocks[0], left_bit - n));
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        // Naming the fallback block removes the general path's start-index guard.
                        auto const [index, offset] = index_offset(n);
                        if (auto const block = shl(m_blocks[index], left_bit - offset); block != zero) {
                                return n - detail::bits::countl_zero(block);
                        }
                        // Reaching here at index 0 would break the precondition the general path asserts instead.
                        assert(index == 1);
                        assert(m_blocks[0] != zero);
                        return left_bit - detail::bits::countl_zero(m_blocks[0]);
                } else {
                        auto [index, offset] = index_offset(n);
                        if (auto const reverse_offset = left_bit - offset; reverse_offset != 0) {
                                if (auto const block = shl(m_blocks[index], reverse_offset); block != zero) {
                                        return n - detail::bits::countl_zero(block);
                                }
                                --index;
                                n -= bits_per_block - reverse_offset;
                        }
                        // A while, not a for: the precondition makes a for's exit untestable.
                        auto i = index;
                        while (m_blocks[i] == zero) {
                                assert(i != 0);
                                --i;
                        }
                        return n - detail::bits::countl_zero(m_blocks[i]) - (bits_per_block * (index - i));
                }
        }

        // Total across two widths and reading-neutral: the blocks the other lacks read as zero. Growing is not here.
        constexpr auto operator&=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] &= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] &= other.m_blocks[0];
                        this->m_blocks[1] &= other.m_blocks[1];
                } else if constexpr (not(has_static_size and N == 0)) {
                        if constexpr (not has_static_size) {
                                if (this->size() != other.size()) {
                                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                                this->m_blocks[i] &= other.padded_block(i);
                                        }
                                        return *this;
                                }
                        }
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] &= other.m_blocks[i];
                        }
                }
                return *this;
        }

        // Total across two widths and reading-neutral: the blocks the other lacks read as zero. Growing is not here.
        constexpr auto operator|=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] |= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] |= other.m_blocks[0];
                        this->m_blocks[1] |= other.m_blocks[1];
                } else if constexpr (not(has_static_size and N == 0)) {
                        if constexpr (not has_static_size) {
                                if (this->size() != other.size()) {
                                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                                this->m_blocks[i] |= other.padded_block(i);
                                        }
                                        return *this;
                                }
                        }
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] |= other.m_blocks[i];
                        }
                }
                return *this;
        }

        // Total across two widths and reading-neutral: the blocks the other lacks read as zero. Growing is not here.
        constexpr auto operator^=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] ^= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] ^= other.m_blocks[0];
                        this->m_blocks[1] ^= other.m_blocks[1];
                } else if constexpr (not(has_static_size and N == 0)) {
                        if constexpr (not has_static_size) {
                                if (this->size() != other.size()) {
                                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                                this->m_blocks[i] ^= other.padded_block(i);
                                        }
                                        return *this;
                                }
                        }
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] ^= other.m_blocks[i];
                        }
                }
                return *this;
        }

        // Total across two widths and reading-neutral: the blocks the other lacks read as zero. Growing is not here.
        constexpr auto operator-=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] &= static_cast<block_type>(~other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] &= static_cast<block_type>(~other.m_blocks[0]);
                        this->m_blocks[1] &= static_cast<block_type>(~other.m_blocks[1]);
                } else if constexpr (not(has_static_size and N == 0)) {
                        if constexpr (not has_static_size) {
                                if (this->size() != other.size()) {
                                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                                this->m_blocks[i] &= static_cast<block_type>(~other.padded_block(i));
                                        }
                                        return *this;
                                }
                        }
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] &= static_cast<block_type>(~other.m_blocks[i]);
                        }
                }
                return *this;
        }

        constexpr auto operator<<=(std::size_t n [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                assert(is_valid(n));
                if constexpr (has_static_size and static_num_blocks == 1) {
                        // m_blocks[0] <<= n narrows the promoted int, which -fsanitize=implicit-conversion aborts on.
                        m_blocks[0] = shl(m_blocks[0], n);
                } else {
                        auto const [n_blocks, L_shift] = xstd::div(n, bits_per_block);
                        // Restated because GCC drops the range through xstd::div.
                        assert(n_blocks <= last_block());
                        if (L_shift == 0) {
                                std::shift_right(std::ranges::begin(m_blocks), std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks));
                        } else {
                                auto const R_shift = bits_per_block - L_shift;
                                for (auto i = last_block(); i > n_blocks; --i) {
                                        // Read one block lower: the splice of [i - n_blocks - 1, i - n_blocks].
                                        m_blocks[i] = straddled_block(i - n_blocks - 1UZ, L_shift, R_shift);
                                }
                                m_blocks[n_blocks] = shl(m_blocks[0], L_shift);
                        }
                        std::ranges::fill_n(std::ranges::begin(m_blocks), static_cast<std::ptrdiff_t>(n_blocks), zero);
                }
                erase_unused();
                return *this;
        }

        constexpr auto operator>>=(std::size_t n [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                assert(is_valid(n));
                if constexpr (has_static_size and static_num_blocks == 1) {
                        // m_blocks[0] >>= n narrows the promoted int, which -fsanitize=implicit-conversion sees.
                        m_blocks[0] = shr(m_blocks[0], n);
                } else {
                        auto const [n_blocks, R_shift] = xstd::div(n, bits_per_block);
                        // See operator<<=: the same bound, for the same reason.
                        assert(n_blocks <= last_block());
                        if (R_shift == 0) {
                                std::shift_left(std::ranges::begin(m_blocks), std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks));
                        } else {
                                auto const L_shift = bits_per_block - R_shift;
                                for (auto i = 0UZ; i + n_blocks < last_block(); ++i) {
                                        // block_at(i * bits_per_block + n), without recomputing the division.
                                        m_blocks[i] = straddled_block(i + n_blocks, L_shift, R_shift);
                                }
                                m_blocks[last_block() - n_blocks] = shr(m_blocks[last_block()], R_shift);
                        }
                        std::ranges::fill_n(std::ranges::prev(std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks)), static_cast<std::ptrdiff_t>(n_blocks), zero);
                }
                return *this;
        }

        constexpr auto set() noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and static_has_unused_bits) {
                        std::ranges::fill_n(std::ranges::begin(m_blocks), static_cast<std::ptrdiff_t>(static_last_block), ones);
                        m_blocks[static_last_block] = static_used_bits;
                } else if constexpr (has_static_size and N > 0) {
                        std::ranges::fill(m_blocks, ones);
                } else if constexpr (not has_static_size) {
                        // Uniform: used_bits() is the whole block on an even division, and none of it at width zero.
                        std::ranges::fill_n(std::ranges::begin(m_blocks), static_cast<std::ptrdiff_t>(last_block()), ones);
                        m_blocks[last_block()] = used_bits();
                }
                assert(all());
                return *this;
        }

        constexpr auto reset() noexcept
                -> contiguous_bit_container&
        {
                std::ranges::fill(m_blocks, zero);
                assert(none());
                return *this;
        }

        constexpr auto flip() noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        m_blocks[0] = static_cast<block_type>(~m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        m_blocks[0] = static_cast<block_type>(~m_blocks[0]);
                        m_blocks[1] = static_cast<block_type>(~m_blocks[1]);
                } else if constexpr (not(has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                m_blocks[i] = static_cast<block_type>(~m_blocks[i]);
                        }
                }
                erase_unused();
                return *this;
        }

        constexpr auto swap(contiguous_bit_container& other) noexcept(noexcept(std::ranges::swap(this->m_size, other.m_size)) and noexcept(std::ranges::swap(this->m_blocks, other.m_blocks)))
                -> void
        {
                // m_size is empty_type under a static width, and swapping that is a no-op.
                std::ranges::swap(this->m_size, other.m_size);
                std::ranges::swap(this->m_blocks, other.m_blocks);
        }

        // ranges::swap finds a free swap by ADL and a member never, so the member is reached through this.
        friend constexpr auto swap(contiguous_bit_container& x, contiguous_bit_container& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }

        // Growth, at a run-time width alone; every path leaves the unused tail clear.
        constexpr auto resize(std::size_t n, bool value = false)
                -> void
                requires (not has_static_size)
        {
                // The ceiling first, so a width this storage cannot count throws before anything below is written.
                resize_to(n, value);
        }

        // Widen to the other's largest element, not its size(); a static width has nothing to widen.
        constexpr auto grow_to_admit(contiguous_bit_container const& other [[maybe_unused]]) noexcept(has_static_size)
                -> void
        {
                if constexpr (not has_static_size) {
                        if (not other.any()) {
                                return;
                        }
                        if (auto const n = other.exclusive_find_prev(other.size()) + 1UZ; n > this->size()) {
                                resize_to(n, false);
                        }
                }
        }

        // Width zero, one block, all of it padding: the same object a default constructor makes.
        constexpr auto clear()
                -> void
                requires (not has_static_size)
        {
                resize_to(0UZ, false);
        }

        constexpr auto push_back(bool value)
                -> void
                requires (not has_static_size)
        {
                resize(size() + 1UZ, value);
        }

        constexpr auto pop_back()
                -> void
                requires (not has_static_size)
        {
                assert(size() != 0UZ);
                resize_to(size() - 1UZ, false);
        }

        // Boost's append: the block's bits become [size(), size() + bits_per_block), split where unaligned.
        constexpr auto append(block_type value)
                -> void
                requires (not has_static_size)
        {
                auto const offset = size() % bits_per_block;
                if (offset != 0UZ) {
                        m_blocks[last_block()] |= shl(value, offset);
                        m_blocks.push_back(shr(value, bits_per_block - offset));
                } else if (size() != 0UZ) {
                        m_blocks.push_back(value);
                } else {
                        // The floor block is the fresh one.
                        m_blocks[0] = value;
                }
                m_size += bits_per_block;
        }

        // Reserved first where the distance is known, so nothing below reallocates: boost's strong guarantee.
        template<std::input_iterator I>
        constexpr auto append(I first, I last)
                -> void
                requires (not has_static_size)
        {
                if constexpr (std::forward_iterator<I> and requires (Blocks& b, std::size_t n) { b.reserve(n); }) {
                        reserve(size() + (static_cast<std::size_t>(std::ranges::distance(first, last)) * bits_per_block));
                }

                // Whole blocks land whole where the width is a multiple, which every block-range construction is.
                if constexpr (std::forward_iterator<I>) {
                        if (first != last and size() % bits_per_block == 0UZ) {
                                auto const n = static_cast<std::size_t>(std::ranges::distance(first, last));
                                if (size() == 0UZ) {
                                        // The floor block is the one an empty container has, and the first replaces it.
                                        m_blocks.assign(first, last);
                                } else {
                                        m_blocks.insert(m_blocks.end(), first, last);
                                }
                                m_size += n * bits_per_block;
                                return;
                        }
                }

                for (; first != last; ++first) {
                        append(*first);
                }
        }

        // In bits, where the blocks have the member: vector and inplace_vector do, array does not.
        constexpr auto reserve(std::size_t n)
                -> void
                requires (not has_static_size) and requires (Blocks& b) { b.reserve(blocks_for(n)); }
        {
                m_blocks.reserve(blocks_for(n));
        }

        [[nodiscard]] constexpr auto capacity() const noexcept
                -> std::size_t
                requires (not has_static_size) and requires (Blocks const& b) { b.capacity(); }
        {
                return m_blocks.capacity() * bits_per_block;
        }

        constexpr auto shrink_to_fit()
                -> void
                requires (not has_static_size) and requires (Blocks& b) { b.shrink_to_fit(); }
        {
                m_blocks.shrink_to_fit();
        }

        constexpr auto set(std::size_t n) noexcept
                -> contiguous_bit_container&
        {
                assert(is_valid(n));
                auto&& [block, mask] = block_mask(n);
                block |= mask;
                assert(test(n));
                return *this;
        }

        [[nodiscard]] constexpr auto insert(std::size_t n) noexcept
                -> bool
        {
                assert(is_valid(n));
                auto&& [block, mask] = block_mask(n);
                auto const inserted = not detail::bits::intersects(block, mask);
                block |= mask;
                assert(test(n));
                return inserted;
        }

        // insert(n) above is partial, n being a precondition; this one is total, growing a run-time width.
        constexpr auto growing_insert(std::size_t n) noexcept(has_static_size)
                -> bool
        {
                if constexpr (not has_static_size) {
                        if (n >= size()) {
                                // width_sum, not n + 1: at the top of size_t n + 1 is a width of zero.
                                resize(width_sum(n, 1UZ));
                                set(n);
                                return true;
                        }
                }
                return insert(n);
        }

        // set(n) and reset(n) under one name, for a reading holding the value rather than the verb.
        constexpr auto assign(std::size_t n, bool value) noexcept
                -> contiguous_bit_container&
        {
                return value ? set(n) : reset(n);
        }

        // The bulk counterpart: set(bool) would be ambiguous with set(std::size_t) for a literal 0.
        constexpr auto fill(bool value) noexcept
                -> contiguous_bit_container&
        {
                return value ? set() : reset();
        }

        constexpr auto reset(std::size_t n) noexcept
                -> contiguous_bit_container&
        {
                assert(is_valid(n));
                auto&& [block, mask] = block_mask(n);
                block &= static_cast<block_type>(~mask);
                assert(not test(n));
                return *this;
        }

        [[nodiscard]] constexpr auto erase(std::size_t n) noexcept
                -> bool
        {
                assert(is_valid(n));
                auto&& [block, mask] = block_mask(n);
                auto const erased = detail::bits::intersects(block, mask);
                block &= static_cast<block_type>(~mask);
                assert(not test(n));
                return erased;
        }

        constexpr auto flip(std::size_t n) noexcept
                -> contiguous_bit_container&
        {
                assert(is_valid(n));
                auto&& [block, mask] = block_mask(n);
                block ^= mask;
                return *this;
        }

        // test, not operator[]: this returns bool, std::bitset's a proxy.
        [[nodiscard]] constexpr auto test(std::size_t n) const noexcept
                -> bool
        {
                assert(is_valid(n));
                auto&& [block, mask] = block_mask(n);
                return detail::bits::intersects(block, mask);
        }

        [[nodiscard]] constexpr auto count() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size and N == 0) {
                        return 0UZ;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return detail::bits::popcount(m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return detail::bits::popcount(m_blocks[0]) + detail::bits::popcount(m_blocks[1]);
                } else {
                        return std::ranges::fold_left(
                                m_blocks | std::views::transform([](auto block) { return detail::bits::popcount(block); }),
                                0UZ, std::plus<>()
                        );
                }
        }

        [[nodiscard]] constexpr auto all() const noexcept
                -> bool
        {
                if constexpr (has_static_size and static_has_unused_bits) {
                        if constexpr (static_num_blocks == 1) {
                                return m_blocks[0] == static_used_bits;
                        } else if constexpr (static_num_blocks == 2) {
                                return m_blocks[0] == ones and m_blocks[1] == static_used_bits;
                        } else {
                                return all_but_last_are_ones() and m_blocks[static_last_block] == static_used_bits;
                        }
                } else if constexpr (has_static_size) {
                        if constexpr (N == 0) {
                                return true;
                        } else if constexpr (static_num_blocks == 1) {
                                return m_blocks[0] == ones;
                        } else if constexpr (static_num_blocks == 2) {
                                return m_blocks[0] == ones and m_blocks[1] == ones;
                        } else {
                                return std::ranges::all_of(m_blocks, [](auto block) { return block == ones; });
                        }
                } else {
                        // One shape for both; the static arms keep the split only to stay compile-time branches.
                        return all_but_last_are_ones() and m_blocks[last_block()] == used_bits();
                }
        }

        [[nodiscard]] constexpr auto any() const noexcept
                -> bool
        {
                return not none();
        }

        [[nodiscard]] constexpr auto none() const noexcept
                -> bool
        {
                if constexpr (has_static_size and N == 0) {
                        return true;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return m_blocks[0] == zero;
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return m_blocks[0] == zero and m_blocks[1] == zero;
                } else {
                        return std::ranges::all_of(m_blocks, [](auto block) { return block == zero; });
                }
        }

        [[nodiscard]] constexpr auto is_subset_of(contiguous_bit_container const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (has_static_size and N == 0) {
                        return true;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return detail::bits::is_subset_of(this->m_blocks[0], other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return detail::bits::is_subset_of(this->m_blocks[0], other.m_blocks[0]) and
                               detail::bits::is_subset_of(this->m_blocks[1], other.m_blocks[1]);
                } else {
                        // zip stops at the shorter, which is exactly the blocks both storages have.
                        auto const shared = std::ranges::all_of(
                                std::views::zip(this->m_blocks, other.m_blocks), [](auto&& _) {
                                        auto&& [lhs, rhs] = _;
                                        return detail::bits::is_subset_of(lhs, rhs);
                                }
                        );
                        if constexpr (has_static_size) {
                                // One width, so the shared blocks are all the blocks.
                                return shared;
                        } else {
                                // Above the shared blocks only ours can hold a position, which denies the subset.
                                return shared and not this->any_block_set(other.num_blocks(), this->num_blocks());
                        }
                }
        }

        // A proper subset is a subset that differs, and both halves are already here.
        [[nodiscard]] constexpr auto is_proper_subset_of(contiguous_bit_container const& other) const noexcept
                -> bool
        {
                return is_subset_of(other) and not set_equal(*this, other);
        }

        [[nodiscard]] constexpr auto intersects(contiguous_bit_container const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                // Only the blocks both storages have can meet, so zip stopping at the shorter is the question.
                if constexpr (has_static_size and N == 0) {
                        return false;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return detail::bits::intersects(this->m_blocks[0], other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return detail::bits::intersects(this->m_blocks[0], other.m_blocks[0]) or
                               detail::bits::intersects(this->m_blocks[1], other.m_blocks[1]);
                } else {
                        return std::ranges::any_of(
                                std::views::zip(this->m_blocks, other.m_blocks), [](auto&& _) {
                                        auto&& [lhs, rhs] = _;
                                        return detail::bits::intersects(lhs, rhs);
                                }
                        );
                }
        }

        // A hidden friend beside the member: a member of this name stops ADL ([basic.lookup.argdep]/1).
        [[nodiscard]] friend constexpr auto intersects(contiguous_bit_container const& x, contiguous_bit_container const& y) noexcept
                -> bool
        {
                return x.intersects(y);
        }

        // The first block at which two values differ, with that block's xor; equal values answer a zero xor.
        [[nodiscard]] constexpr auto first_difference(contiguous_bit_container const& other) const noexcept
                -> std::pair<std::size_t, block_type>
        {
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return {0UZ, static_cast<block_type>(this->m_blocks[0] ^ other.m_blocks[0])};
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        if (auto const diff = static_cast<block_type>(this->m_blocks[0] ^ other.m_blocks[0]); diff != zero) {
                                return {0UZ, diff};
                        }
                        return {1UZ, static_cast<block_type>(this->m_blocks[1] ^ other.m_blocks[1])};
                } else {
                        auto const last = num_blocks() - 1UZ;
                        for (auto i = 0UZ; i < last; ++i) {
                                if (auto const diff = static_cast<block_type>(this->m_blocks[i] ^ other.m_blocks[i]); diff != zero) {
                                        return {i, diff};
                                }
                        }
                        return {last, static_cast<block_type>(this->m_blocks[last] ^ other.m_blocks[last])};
                }
        }

private:
        // Whether any position strictly above the given one is set, which one shift down leaves.
        [[nodiscard]] constexpr auto any_above(std::size_t index, std::size_t offset) const noexcept
                -> bool
        {
                assert(not test((index * bits_per_block) + offset));
                if (shr(m_blocks[index], offset) != zero) {
                        return true;
                }
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return false;
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return index == 0UZ and m_blocks[1] != zero;
                } else {
                        for (auto i = index + 1UZ, n = num_blocks(); i < n; ++i) {
                                if (m_blocks[i] != zero) {
                                        return true;
                                }
                        }
                        return false;
                }
        }

        // Comparing two widths needs blocks one storage lacks; reading them as zero is the invariant restated.
        [[nodiscard]] constexpr auto padded_block(std::size_t index) const noexcept
                -> block_type
        {
                return index < num_blocks() ? m_blocks[index] : zero;
        }

        // Whether any block in the half-open range is set; an empty or inverted range answers no.
        [[nodiscard]] constexpr auto any_block_set(std::size_t first, std::size_t last) const noexcept
                -> bool
        {
                return std::ranges::any_of(std::views::iota(first, std::ranges::max(first, last)), [this](std::size_t index) -> bool { return m_blocks[index] != zero; });
        }

        // any_above with no precondition: the position may lie past this storage's last block.
        [[nodiscard]] constexpr auto padded_any_above(std::size_t index, std::size_t offset) const noexcept
                -> bool
        {
                auto const n = num_blocks();
                if (index < n and shr(m_blocks[index], offset) != zero) {
                        return true;
                }
                return any_block_set(std::ranges::min(index + 1UZ, n), n);
        }

        // The lowest block at which two storages differ, or n when they hold the same positions.
        [[nodiscard]] constexpr auto padded_first_difference(contiguous_bit_container const& other, std::size_t n) const noexcept
                -> std::size_t
        {
                auto const blocks = std::views::iota(0UZ, n);
                auto const found = std::ranges::find_if(blocks, [&](std::size_t index) -> bool { return this->padded_block(index) != other.padded_block(index); });
                return found == std::ranges::end(blocks) ? n : *found;
        }

        // The set ordering across two widths, turning on the lowest position at which the two disagree.
        [[nodiscard]] constexpr auto padded_set_three_way(contiguous_bit_container const& other) const noexcept
                -> std::strong_ordering
        {
                auto const n = std::ranges::max(this->num_blocks(), other.num_blocks());
                auto const index = padded_first_difference(other, n);
                if (index == n) {
                        return std::strong_ordering::equal;
                }
                auto const diff = static_cast<block_type>(this->padded_block(index) ^ other.padded_block(index));
                auto const offset = static_cast<std::size_t>(detail::bits::countr_zero(diff));
                if (detail::bits::intersects(this->padded_block(index), shl(unit, offset))) {
                        return other.padded_any_above(index, offset) ? std::strong_ordering::less : std::strong_ordering::greater;
                }
                return this->padded_any_above(index, offset) ? std::strong_ordering::greater : std::strong_ordering::less;
        }

        // The sequence ordering: position 0 is the first element, so the lowest disagreement decides alone.
        [[nodiscard]] constexpr auto padded_sequence_three_way(contiguous_bit_container const& other) const noexcept
                -> std::strong_ordering
        {
                auto const n = std::ranges::max(this->num_blocks(), other.num_blocks());
                auto const index = padded_first_difference(other, n);
                if (index == n) {
                        // The two answers this can give: the caller arrives only with the sizes differing.
                        return this->size() < other.size() ? std::strong_ordering::less : std::strong_ordering::greater;
                }
                auto const diff = static_cast<block_type>(this->padded_block(index) ^ other.padded_block(index));
                auto const offset = static_cast<std::size_t>(detail::bits::countr_zero(diff));
                return detail::bits::intersects(this->padded_block(index), shl(unit, offset))
                               ? std::strong_ordering::greater
                               : std::strong_ordering::less;
        }

        // The block straddling index and index + 1, spliced from a shift up and a shift down.
        [[nodiscard]] constexpr auto straddled_block(std::size_t index, std::size_t L_shift, std::size_t R_shift) const noexcept
                -> block_type
        {
                assert(L_shift + R_shift == bits_per_block);
                assert(0UZ < R_shift and R_shift < bits_per_block);
                assert(index + 1UZ < num_blocks());
                return static_cast<block_type>(
                        shl(m_blocks[index + 1UZ], L_shift) |
                        shr(m_blocks[index], R_shift)
                );
        }

        [[nodiscard]] constexpr auto last_block() const noexcept
                -> std::size_t
        {
                return num_blocks() - 1UZ;
        }

        // The words a range of positions spans, each with its mask: whole words, a partial one at the end.
        template<class F>
        constexpr auto for_each_block(std::size_t n, std::size_t len, F f) const noexcept
                -> void
        {
                for (auto pos = n; pos < n + len; pos += bits_per_block) {
                        auto const count = std::ranges::min(bits_per_block, n + len - pos);
                        f(pos, count == bits_per_block ? ones : static_cast<block_type>(shl(unit, count) - unit));
                }
        }

        [[nodiscard]] constexpr auto all_but_last_are_ones() const noexcept
                -> bool
        {
                auto const first = std::ranges::begin(m_blocks);
                return std::ranges::all_of(first, first + static_cast<std::ptrdiff_t>(last_block()), [](auto block) { return block == ones; });
        }

        // The top bit of the last block; both callers assert any(), so width zero never reaches here.
        [[nodiscard]] constexpr auto last_bit() const noexcept
                -> std::size_t
        {
                return (num_blocks() * bits_per_block) - 1UZ;
        }

        // Whether the blocks hold more bits than the width names; width zero is why it is not a modulo.
        [[nodiscard]] constexpr auto has_unused_bits() const noexcept
                -> bool
        {
                return num_blocks() * bits_per_block != size();
        }

        // static_used_bits at a run-time width, width zero selected rather than computed.
        [[nodiscard]] constexpr auto used_bits() const noexcept
                -> block_type
        {
                return size() == 0 ? zero : shr(ones, (num_blocks() * bits_per_block) - size());
        }

        [[nodiscard]] constexpr auto is_valid(std::size_t n [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (has_static_size and N == 0) {
                        // Unreachable: only an assert calls is_valid, and MSVC's /W4 rejects a bare n < N (C4296).
                        return false; // GCOVR_EXCL_LINE
                } else {
                        return n < size();
                }
        }

        [[nodiscard]] static constexpr auto index_offset(std::size_t n) noexcept
                -> xstd::div_result<std::size_t>
        {
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return {.quotient = 0UZ, .remainder = n};
                } else {
                        return xstd::div(n, bits_per_block);
                }
        }

        // cl rejects a member of the explicit object parameter in a trailing return type (C2228).
        template<class Self>
        using block_reference_t = std::conditional_t<
                std::is_const_v<std::remove_reference_t<Self>>, block_type const&, block_type&>;

        [[nodiscard]] constexpr auto block_mask(this auto&& self, std::size_t n) noexcept
                -> std::pair<block_reference_t<decltype(self)>, block_type>
        {
                auto const [index, offset] = index_offset(n);
                return {std::forward<decltype(self)>(self).m_blocks[index], shl(unit, offset)};
        }

        // The growth itself, over a width already known to be one this storage can count.
        constexpr auto resize_to(std::size_t n, bool value)
                -> void
                requires (not has_static_size)
        {
                auto const count = blocks_for(n);
                // Growing with ones: which bits become new is read off the old width, before the blocks grow.
                if (value and n > size()) {
                        auto const index = last_block();
                        auto const tail = static_cast<block_type>(~used_bits());
                        m_blocks.resize(count, ones);
                        m_blocks[index] |= tail;
                } else {
                        // No new block can be a one here: either the value is false or the width is not growing.
                        m_blocks.resize(count, zero);
                }
                m_size = n;
                erase_unused();
        }

        // std::length_error, what a container throws for a size it cannot represent; the readings inherit it.
        [[nodiscard]] static constexpr auto length_error(std::size_t n, std::source_location const& loc = std::source_location::current())
        {
                return std::length_error(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘n‘ is no width this storage can count [{} > {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), n, max_width
                        )
                );
        }

        // The same, against the ceiling a difference_type sets rather than the one a size_t sets.
        [[nodiscard]] static constexpr auto addressable_length_error(std::size_t n, std::source_location const& loc = std::source_location::current())
        {
                return std::length_error(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘n‘ is no width a distance can name [{} > {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), n, max_addressable_width
                        )
                );
        }
};

} // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_CONTIGUOUS_BIT_CONTAINER_HPP
