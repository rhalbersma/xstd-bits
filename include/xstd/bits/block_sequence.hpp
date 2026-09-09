//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BLOCK_SEQUENCE_HPP
#define XSTD_BITS_BLOCK_SEQUENCE_HPP

#include <boost/hash2/hash_append_fwd.hpp>                     // hash_append, hash_append_tag
#include <xstd/bits/bit_traits.hpp>                            // bit_traits
#include <xstd/bits/detail/allocator_typedef.hpp>              // allocator_typedef
#include <xstd/bits/detail/intrin.hpp>                         // countl_zero, countr_zero, popcount
#include <xstd/bits/detail/pred.hpp>                           // intersects, is_subset_of, not_equal_to
#include <xstd/ints/concepts/unsigned_integer.hpp>             // unsigned_integer
#include <xstd/ints/cstdlib/div.hpp>                           // div, div_result
#include <xstd/ints/limits.hpp>                                // numeric_limits
#include <xstd/ints/memory.hpp>                                // align_up
#include <xstd/misc/type_traits/conditional_data_member.hpp>   // XSTD_NO_UNIQUE_ADDRESS, conditional_data_member_t
#include <algorithm>                                           // all_of, any_of, fill, fill_n, fold_left, max, min, shift_left, shift_right
#include <array>                                               // array
#include <cassert>                                             // assert
#include <compare>                                             // strong_ordering
#include <concepts>                                            // regular, same_as, swap
#include <cstddef>                                             // ptrdiff_t, size_t
#include <functional>                                          // plus
#include <iterator>                                            // distance, forward_iterator, input_iterator, prev
#include <limits>                                              // numeric_limits
#include <memory>                                              // allocator
#include <ranges>                                              // begin, drop, iota, size, swap, transform, zip
                                                               // (views::drop_last when P22014R2 is accepted)
#include <span>                                                // dynamic_extent
#include <type_traits>                                         // conditional_t, is_const_v, is_nothrow_swappable_v, remove_reference_t
#include <utility>                                             // exchange, move, pair
#include <vector>                                              // vector
#include <version>                                             // IWYU pragma: keep; __cpp_lib_inplace_vector
#ifdef __cpp_lib_inplace_vector
#include <inplace_vector>                                      // inplace_vector
#endif

namespace xstd {

// Whether a range IS blocks; block_readable asks if a trait hands a container's blocks over. [design.md#block-storage]
template<class R>
concept block_storage =
        std::regular<R> and
        std::ranges::contiguous_range<R> and
        std::ranges::sized_range<R> and
        xstd::unsigned_integer<std::ranges::range_value_t<R>>
;

// Floored at one so a zero width still names a block. [design.md#the-one-vehicle]
template<xstd::unsigned_integer Block, std::size_t N>
inline constexpr auto num_blocks_v = std::ranges::max(
        align_up(N, static_cast<std::size_t>(xstd::numeric_limits<Block>::digits)) /
                    static_cast<std::size_t>(xstd::numeric_limits<Block>::digits),
        1UZ
);

// The one vehicle: it owns the unused-tail invariant, and has no iterators. [design.md#the-one-vehicle]
template<block_storage Blocks, std::size_t N = std::dynamic_extent>
class block_sequence : public detail::bits::allocator_typedef<Blocks>
{
public:
        using block_type = std::ranges::range_value_t<Blocks>;

        static constexpr auto bits_per_block  = static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits);
        static constexpr auto has_static_size = N != std::dynamic_extent;

private:
        static constexpr auto static_num_bits   = has_static_size ? align_up(N, bits_per_block) : 0UZ;
        static constexpr auto static_num_blocks = has_static_size ? std::ranges::max(static_num_bits / bits_per_block, 1UZ) : 0UZ;
        static constexpr auto static_last_block = static_num_blocks - 1UZ;

        static constexpr auto left_bit = bits_per_block - 1UZ;
        static constexpr auto unit     = static_cast<block_type>( 1);
        static constexpr auto zero     = static_cast<block_type>( 0);
        static constexpr auto ones     = static_cast<block_type>(-1);

        // The width is a size_t, unless the blocks out-align one: then it is a block, which fills what would
        // otherwise be padding in front of them. Only a storage holding its blocks inline out-aligns a size_t,
        // and it does so by their alignment, so a block is both wide enough to hold any width and exactly the
        // size the gap is. [design.md#padding]
        using width_type = std::conditional_t<(alignof(std::size_t) >= alignof(Blocks)), std::size_t, block_type>;
        static_assert(sizeof(width_type) >= sizeof(std::size_t) and alignof(width_type) >= alignof(Blocks));

        // Width zero named, not computed: MSVC folds both ?: arms and answers C4293. [design.md#padding]
        static constexpr auto static_num_unused_bits = has_static_size ? static_num_bits - N : 0UZ;
        static constexpr auto static_used_bits       = has_static_size and N == 0 ? zero : static_cast<block_type>(ones >> static_num_unused_bits);
        static constexpr auto static_unused_bits     = static_cast<block_type>(~static_used_bits);
        static constexpr auto static_has_unused_bits = has_static_size and static_used_bits != ones;

        // How many blocks a run-time width needs, floored at one like num_blocks_v. [design.md#the-one-vehicle]
        [[nodiscard]] static constexpr auto blocks_for(std::size_t n) noexcept
                -> std::size_t
        {
                return std::ranges::max(align_up(n, bits_per_block) / bits_per_block, 1UZ);
        }

        // An NSDMI, not extent-constrained constructors: vector starts empty. [design.md#default-construction]
        [[nodiscard]] static constexpr auto make_blocks(std::size_t n)
                -> Blocks
        {
                if constexpr (has_static_size) {
                        return Blocks{};
                } else {
                        return Blocks(blocks_for(n));
                }
        }

        // Dynamic widths only; the tag keeps the absent member distinct from any other in an enclosing layout.
        // Declared first, so the defaulted == rejects on the width before it reads a block. [design.md#block-storage]
        [[XSTD_NO_UNIQUE_ADDRESS]]
        conditional_data_member_t<not has_static_size, width_type, struct size_tag> m_size{};

        Blocks m_blocks = make_blocks(0UZ);

public:
        block_sequence() = default;

        // The width is a constructor argument exactly when it is not a template argument.
        [[nodiscard]] constexpr explicit block_sequence(std::size_t n)
                requires (not has_static_size)
        :
                m_size(n),
                m_blocks(make_blocks(n))
        {}

        // boost's allocator arguments, where the blocks take one: deduced and matched, so a storage without an allocator has no such constructor. [design.md#a-strict-extension]
        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr explicit block_sequence(Alloc const& alloc)
        :
                m_blocks(blocks_for(0UZ), alloc)
        {}

        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr block_sequence(std::size_t n, Alloc const& alloc)
        :
                m_size(n),
                m_blocks(blocks_for(n), alloc)
        {}

        // [container.alloc.reqmts]'s allocator-extended copy and move; the moved-from is left empty whichever way the blocks went.
        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr block_sequence(block_sequence const& other, Alloc const& alloc)
        :
                m_size(other.m_size),
                m_blocks(other.m_blocks, alloc)
        {}

        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr block_sequence(block_sequence&& other, Alloc const& alloc)
        :
                m_size(std::exchange(other.m_size, 0UZ)),
                m_blocks(std::move(other.m_blocks), alloc)
        {
                other.m_blocks.clear();
        }

        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires requires (Blocks const& b) { b.get_allocator(); }
        {
                return m_blocks.get_allocator();
        }

        // In bits: the width, or the widest whole number of blocks the blocks can hold and the address space can count. [design.md#a-strict-extension]
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size) {
                        return N;
                } else {
                        return std::ranges::min(m_blocks.max_size(), std::numeric_limits<std::size_t>::max() / bits_per_block) * bits_per_block;
                }
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

        // The block, behind the trait's block entry; padding above size() stays zero, which is what makes whole-block comparison mean anything.
        [[nodiscard]] constexpr auto block(std::size_t i) const noexcept
                -> block_type
        {
                assert(i < num_blocks());
                return m_blocks[i];
        }

        // The write side of block(), and no trait entry. [design.md#block-writes]
        constexpr auto set_block(std::size_t i, block_type value) noexcept
                -> void
        {
                assert(i < num_blocks());
                m_blocks[i] = value;
                erase_unused();
        }

        // A word at any position, aligned or not: the bits [n, n + bits_per_block), the clear tail and nothing beyond the last block. [design.md#the-blit]
        [[nodiscard]] constexpr auto word_at(std::size_t n) const noexcept
                -> block_type
        {
                auto const [ index, offset ] = index_offset(n);
                assert(index < num_blocks());
                if (offset == 0UZ or index == last_block()) {
                        return static_cast<block_type>(m_blocks[index] >> offset);
                }
                return straddled_block(index, bits_per_block - offset, offset);
        }

        // The write side of word_at, masked: the bits of value under mask land at [n, n + bits_per_block), split over two blocks where n is not aligned, and the tail stays clear. [design.md#the-blit]
        constexpr auto set_word(std::size_t n, block_type value, block_type mask) noexcept
                -> void
        {
                auto const [ index, offset ] = index_offset(n);
                assert(index < num_blocks());
                auto const bits = static_cast<block_type>(value & mask);

                // Each step lands back in block_type: a promoted operand feeding the next bitwise operator is what bugprone-signed-bitwise reads. [design.md#block-writes]
                auto const low_kept = static_cast<block_type>(m_blocks[index] & static_cast<block_type>(~static_cast<block_type>(mask << offset)));
                m_blocks[index] = static_cast<block_type>(low_kept | static_cast<block_type>(bits << offset));
                if (offset != 0UZ and index != last_block()) {
                        auto const shift = bits_per_block - offset;
                        auto const high_kept = static_cast<block_type>(m_blocks[index + 1UZ] & static_cast<block_type>(~static_cast<block_type>(mask >> shift)));
                        m_blocks[index + 1UZ] = static_cast<block_type>(high_kept | static_cast<block_type>(bits >> shift));
                }
                erase_unused();
        }

        // boost's ranged forms, a word at a time through set_word: [n, n + len) set, cleared or flipped, the rest untouched. [design.md#the-blit]
        constexpr auto set(std::size_t n, std::size_t len, bool value) noexcept
                -> block_sequence&
        {
                assert(n + len <= size());
                for_each_word(n, len, [&](std::size_t pos, block_type mask) -> void { set_word(pos, value ? ones : zero, mask); });
                return *this;
        }

        constexpr auto flip(std::size_t n, std::size_t len) noexcept
                -> block_sequence&
        {
                assert(n + len <= size());
                for_each_word(n, len, [&](std::size_t pos, block_type mask) -> void { set_word(pos, static_cast<block_type>(~word_at(pos)), mask); });
                return *this;
        }

        // Memberwise, width first: the unused bits are kept clear, so the blocks compare as the bits do, and a zero width through the one block it still holds. [design.md#block-storage]
        [[nodiscard]] friend constexpr auto operator==(block_sequence const&, block_sequence const&) noexcept -> bool = default;

        // No operator<=>: block_sequence is pure storage with no opinion on which reading orders it, so it names all three and picks none. [design.md#two-readings-disagree]

        // The set reading a word at a time: whoever HOLDS the lowest differing position is greater, unless the other holds nothing above it. [design.md#the-ordering-primitive]
        [[nodiscard]] constexpr auto set_three_way(block_sequence const& other [[maybe_unused]]) const noexcept
                -> std::strong_ordering
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N == 0) {
                        return std::strong_ordering::equal;
                } else if constexpr (has_static_size and N == 1) {
                        // One position, so the loser is empty and any_above is constantly false. [design.md#degenerate-widths]
                        return this->test(0UZ) <=> other.test(0UZ);
                } else {
                        auto const [ index, diff ] = first_difference(other);
                        if (diff == zero) {
                                return std::strong_ordering::equal;
                        }
                        auto const offset = detail::bits::countr_zero(diff);
                        if (detail::bits::intersects(this->m_blocks[index], static_cast<block_type>(unit << offset))) {
                                return other.any_above(index, offset) ? std::strong_ordering::less : std::strong_ordering::greater;
                        }
                        return this->any_above(index, offset) ? std::strong_ordering::greater : std::strong_ordering::less;
                }
        }

        // The sequence reading a word at a time: whoever HOLDS the lowest differing position is greater, with no prefix clause, the widths being equal. [design.md#the-ordering-primitive]
        [[nodiscard]] constexpr auto sequence_three_way(block_sequence const& other [[maybe_unused]]) const noexcept
                -> std::strong_ordering
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N == 0) {
                        return std::strong_ordering::equal;
                } else {
                        auto const [ index, diff ] = first_difference(other);
                        if (diff == zero) {
                                return std::strong_ordering::equal;
                        }
                        auto const offset = detail::bits::countr_zero(diff);
                        return detail::bits::intersects(this->m_blocks[index], static_cast<block_type>(unit << offset))
                                ? std::strong_ordering::greater
                                : std::strong_ordering::less
                        ;
                }
        }

        // The bitset reading a word at a time: the bit string, most significant position first, is the blocks from the top block down, the unused tail being clear. [design.md#the-ordering-primitive]
        [[nodiscard]] constexpr auto bitset_three_way(block_sequence const& other [[maybe_unused]]) const noexcept
                -> std::strong_ordering
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N == 0) {
                        return std::strong_ordering::equal;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return this->m_blocks[0] <=> other.m_blocks[0];
                } else {
                        for (auto i = num_blocks(); i-- != 0UZ;) {
                                if (auto const cmp = this->m_blocks[i] <=> other.m_blocks[i]; cmp != std::strong_ordering::equal) {
                                        return cmp;
                                }
                        }
                        return std::strong_ordering::equal;
                }
        }

        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, block_sequence const* v) noexcept
                -> void
        {
                boost::hash2::hash_append(h, f, v->m_blocks);
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
                        // A while, not a for: any() makes a for's exit untestable. [design.md#while-not-for]
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

        // Its own 0, and the same instructions the hand-written version emitted. [design.md#inclusive-is-the-primitive]
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

        // The primitive: inclusive, so both derivations are + 1 and nothing wraps. [design.md#inclusive-is-the-primitive]
        [[nodiscard]] constexpr auto inclusive_find_next(std::size_t n) const noexcept
                -> std::size_t
        {
                assert(n <= size());
                if (n == size()) {
                        return size();
                }
                if constexpr (has_static_size and static_num_blocks == 1) {
                        if (auto const block = static_cast<block_type>(m_blocks[0] >> n); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        // Indexed, not branched: an if cost 10 instructions at -O3. [design.md#two-block-case]
                        auto const [ index, offset ] = index_offset(n);
                        if (auto const block = static_cast<block_type>(m_blocks[index] >> offset); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                        if (index == 0 and m_blocks[1] != zero) {
                                return bits_per_block + detail::bits::countr_zero(m_blocks[1]);
                        }
                } else {
                        // No offset != 0 guard: >> 0 is the identity. [design.md#offset-guards]
                        auto [ index, offset ] = index_offset(n);
                        if (auto const block = static_cast<block_type>(m_blocks[index] >> offset); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                        ++index;
                        n += bits_per_block - offset;
                        // A plain index walk: drop + find_if made distance() recover the index. [design.md#index-walks]
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

        // Deliberately NOT total: three instructions cheaper, and rend() supplies the guard. [design.md#total-versus-precondition]
        [[nodiscard]] constexpr auto exclusive_find_prev(std::size_t n) const noexcept
                -> std::size_t
        {
                // States 1 <= n <= size() in one predicate, 0 - 1 being SIZE_MAX. [design.md#the-wraparound-assert]
                assert(is_valid(n - 1));
                assert(any());
                --n;
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return n - detail::bits::countl_zero(static_cast<block_type>(m_blocks[0] << (left_bit - n)));
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        // Naming the fallback block removes the general path's start-index guard. [design.md#offset-guards]
                        auto const [ index, offset ] = index_offset(n);
                        if (auto const block = static_cast<block_type>(m_blocks[index] << (left_bit - offset)); block != zero) {
                                return n - detail::bits::countl_zero(block);
                        }
                        // Reaching here at index 0 would break the precondition the general path asserts instead.
                        assert(index == 1);
                        assert(m_blocks[0] != zero);
                        return left_bit - detail::bits::countl_zero(m_blocks[0]);
                } else {
                        auto [ index, offset ] = index_offset(n);
                        if (auto const reverse_offset = left_bit - offset; reverse_offset != 0) {
                                if (auto const block = static_cast<block_type>(m_blocks[index] << reverse_offset); block != zero) {
                                        return n - detail::bits::countl_zero(block);
                                }
                                --index;
                                n -= bits_per_block - reverse_offset;
                        }
                        // A while, not a for: the precondition makes a for's exit untestable. [design.md#while-not-for]
                        auto i = index;
                        while (m_blocks[i] == zero) {
                                assert(i != 0);
                                --i;
                        }
                        return n - detail::bits::countl_zero(m_blocks[i]) - (bits_per_block * (index - i));
                }
        }

        constexpr auto operator&=(block_sequence const& other [[maybe_unused]]) noexcept
                -> block_sequence&
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] &= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] &= other.m_blocks[0];
                        this->m_blocks[1] &= other.m_blocks[1];
                } else if constexpr (not (has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] &= other.m_blocks[i];
                        }
                }
                return *this;
        }

        constexpr auto operator|=(block_sequence const& other [[maybe_unused]]) noexcept
                -> block_sequence&
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] |= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] |= other.m_blocks[0];
                        this->m_blocks[1] |= other.m_blocks[1];
                } else if constexpr (not (has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] |= other.m_blocks[i];
                        }
                }
                return *this;
        }

        constexpr auto operator^=(block_sequence const& other [[maybe_unused]]) noexcept
                -> block_sequence&
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] ^= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] ^= other.m_blocks[0];
                        this->m_blocks[1] ^= other.m_blocks[1];
                } else if constexpr (not (has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] ^= other.m_blocks[i];
                        }
                }
                return *this;
        }

        constexpr auto operator-=(block_sequence const& other [[maybe_unused]]) noexcept
                -> block_sequence&
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] &= static_cast<block_type>(~other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] &= static_cast<block_type>(~other.m_blocks[0]);
                        this->m_blocks[1] &= static_cast<block_type>(~other.m_blocks[1]);
                } else if constexpr (not (has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                this->m_blocks[i] &= static_cast<block_type>(~other.m_blocks[i]);
                        }
                }
                return *this;
        }

        constexpr auto operator<<=(std::size_t n [[maybe_unused]]) noexcept
                -> block_sequence&
        {
                assert(is_valid(n));
                if constexpr (has_static_size and static_num_blocks == 1) {
                        // m_blocks[0] <<= n narrows the promoted int back to a block_type implicitly, which -fsanitize=implicit-conversion aborts on once a bit shifts out.
                        m_blocks[0] = static_cast<block_type>(m_blocks[0] << n);
                } else {
                        auto const [ n_blocks, L_shift ] = xstd::div(n, bits_per_block);
                        // Restated because GCC drops the range through xstd::div. [design.md#gcc-array-bounds]
                        assert(n_blocks <= last_block());
                        if (L_shift == 0) {
                                std::shift_right(std::ranges::begin(m_blocks), std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks));
                        } else {
                                auto const R_shift = bits_per_block - L_shift;
                                for (auto i = last_block(); i > n_blocks; --i) {
                                        // Read one block lower than the destination: the splice of [i - n_blocks - 1, i - n_blocks]. [design.md#the-funnel-shift]
                                        m_blocks[i] = straddled_block(i - n_blocks - 1UZ, L_shift, R_shift);
                                }
                                m_blocks[n_blocks] = static_cast<block_type>(m_blocks[0] << L_shift);
                        }
                        std::ranges::fill_n(std::ranges::begin(m_blocks), static_cast<std::ptrdiff_t>(n_blocks), zero);
                }
                erase_unused();
                return *this;
        }

        constexpr auto operator>>=(std::size_t n [[maybe_unused]]) noexcept
                -> block_sequence&
        {
                assert(is_valid(n));
                if constexpr (has_static_size and static_num_blocks == 1) {
                        // m_blocks[0] >>= n narrows the promoted int back to a block_type implicitly, which -fsanitize=implicit-conversion instruments.
                        m_blocks[0] = static_cast<block_type>(m_blocks[0] >> n);
                } else {
                        auto const [ n_blocks, R_shift ] = xstd::div(n, bits_per_block);
                        // See operator<<=: the same bound, for the same reason.
                        assert(n_blocks <= last_block());
                        if (R_shift == 0) {
                                std::shift_left(std::ranges::begin(m_blocks), std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks));
                        } else {
                                auto const L_shift = bits_per_block - R_shift;
                                for (auto i = 0UZ; i + n_blocks < last_block(); ++i) {
                                        // Which is word_at(i * bits_per_block + n), reached without recomputing the division. [design.md#the-funnel-shift]
                                        m_blocks[i] = straddled_block(i + n_blocks, L_shift, R_shift);
                                }
                                m_blocks[last_block() - n_blocks] = static_cast<block_type>(m_blocks[last_block()] >> R_shift);
                        }
                        std::ranges::fill_n(std::ranges::prev(std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks)), static_cast<std::ptrdiff_t>(n_blocks), zero);
                }
                return *this;
        }

        constexpr auto set() noexcept
                -> block_sequence&
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
                -> block_sequence&
        {
                std::ranges::fill(m_blocks, zero);
                assert(none());
                return *this;
        }

        constexpr auto flip() noexcept
                -> block_sequence&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        m_blocks[0] = static_cast<block_type>(~m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        m_blocks[0] = static_cast<block_type>(~m_blocks[0]);
                        m_blocks[1] = static_cast<block_type>(~m_blocks[1]);
                } else if constexpr (not (has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                m_blocks[i] = static_cast<block_type>(~m_blocks[i]);
                        }
                }
                erase_unused();
                return *this;
        }

        constexpr auto swap(block_sequence& other) noexcept(std::is_nothrow_swappable_v<Blocks>)
                -> void
        {
                // m_size is empty_type under a static width, and swapping that is a no-op.
                std::ranges::swap(this->m_size,   other.m_size);
                std::ranges::swap(this->m_blocks, other.m_blocks);
        }

        // Growth, at a run-time width alone; every path leaves the unused tail clear, so the block walks read nothing above size(). [design.md#growth]
        constexpr auto resize(std::size_t n, bool value = false)
                -> void
                requires (not has_static_size)
        {
                // Growing with ones: the tail above size() in the last block is clear by the invariant, and becomes the first new bits.
                if (value and n > size()) {
                        m_blocks[last_block()] |= static_cast<block_type>(~used_bits());
                }
                m_blocks.resize(blocks_for(n), value ? ones : zero);
                m_size = n;
                erase_unused();
        }

        // Width zero, one block, all of it padding: the same object a default constructor makes. [design.md#default-construction]
        constexpr auto clear()
                -> void
                requires (not has_static_size)
        {
                resize(0UZ);
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
                resize(size() - 1UZ);
        }

        // Boost's append: the block's bits become the positions [size(), size() + bits_per_block), split over two blocks where size() is not aligned.
        constexpr auto append(block_type value)
                -> void
                requires (not has_static_size)
        {
                auto const offset = size() % bits_per_block;
                if (offset != 0UZ) {
                        m_blocks[last_block()] |= static_cast<block_type>(value << offset);
                        m_blocks.push_back(static_cast<block_type>(value >> (bits_per_block - offset)));
                } else if (size() != 0UZ) {
                        m_blocks.push_back(value);
                } else {
                        // The floor block is the fresh one.
                        m_blocks[0] = value;
                }
                m_size += bits_per_block;
        }

        // Reserved first where the distance is known, so no push_back below can reallocate: the strong guarantee boost documents.
        template<std::input_iterator I>
        constexpr auto append(I first, I last)
                -> void
                requires (not has_static_size)
        {
                if constexpr (std::forward_iterator<I> and requires (Blocks& b, std::size_t n) { b.reserve(n); }) {
                        reserve(size() + (static_cast<std::size_t>(std::ranges::distance(first, last)) * bits_per_block));
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
                -> block_sequence&
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
                block |= mask;
                assert(test(n));
                return *this;
        }

        [[nodiscard]] constexpr auto insert(std::size_t n) noexcept
                -> bool
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
                auto const inserted = not detail::bits::intersects(block, mask);
                block |= mask;
                assert(test(n));
                return inserted;
        }

        constexpr auto reset(std::size_t n) noexcept
                -> block_sequence&
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
                block &= static_cast<block_type>(~mask);
                assert(not test(n));
                return *this;
        }

        [[nodiscard]] constexpr auto erase(std::size_t n) noexcept
                -> bool
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
                auto const erased = detail::bits::intersects(block, mask);
                block &= static_cast<block_type>(~mask);
                assert(not test(n));
                return erased;
        }

        constexpr auto flip(std::size_t n) noexcept
                -> block_sequence&
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
                block ^= mask;
                return *this;
        }

        // test, not operator[]: this returns bool, std::bitset's a proxy. [design.md#test-not-subscript]
        [[nodiscard]] constexpr auto test(std::size_t n) const noexcept
                -> bool
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
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

        [[nodiscard]] constexpr auto is_subset_of(block_sequence const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N == 0) {
                        return true;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return detail::bits::is_subset_of(this->m_blocks[0], other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return
                                detail::bits::is_subset_of(this->m_blocks[0], other.m_blocks[0]) and
                                detail::bits::is_subset_of(this->m_blocks[1], other.m_blocks[1])
                        ;
                } else {
                        return std::ranges::all_of(
                                std::views::zip(this->m_blocks, other.m_blocks), [](auto&& _) { auto&& [ lhs, rhs] = _;
                                return detail::bits::is_subset_of(lhs, rhs);
                        });
                }
        }

        // A proper subset is a subset that differs, and both halves are already here: the unrolled arms are
        // is_subset_of's, and != is the defaulted memberwise comparison. Nothing is left to walk by hand.
        // [design.md#the-cheapest-contract]
        [[nodiscard]] constexpr auto is_proper_subset_of(block_sequence const& other) const noexcept
                -> bool
        {
                assert(this->size() == other.size());
                return is_subset_of(other) and *this != other;
        }

        [[nodiscard]] constexpr auto intersects(block_sequence const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                assert(this->size() == other.size());
                if constexpr (has_static_size and N == 0) {
                        return false;
                } else if constexpr (has_static_size and static_num_blocks == 1) {
                        return detail::bits::intersects(this->m_blocks[0], other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        return
                                detail::bits::intersects(this->m_blocks[0], other.m_blocks[0]) or
                                detail::bits::intersects(this->m_blocks[1], other.m_blocks[1])
                        ;
                } else {
                        return std::ranges::any_of(
                                std::views::zip(this->m_blocks, other.m_blocks), [](auto&& _) { auto&& [ lhs, rhs ] = _;
                                return detail::bits::intersects(lhs, rhs);
                        });
                }
        }

        // The first block at which two values differ, with that block's xor; equal values answer the last block and a zero xor, every arm alike. [design.md#the-ordering-primitive]
        //
        // Public, because the sequence reading's mismatch is one countr_zero from it. The name stays: it scans
        // low block to high, which is the ascending orderings' answer and not bitset_three_way's, so calling it
        // mismatch here would repeat the mistake lexicographical_three_way made. [design.md#two-readings-disagree]
        [[nodiscard]] constexpr auto first_difference(block_sequence const& other) const noexcept
                -> std::pair<std::size_t, block_type>
        {
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return { 0UZ, static_cast<block_type>(this->m_blocks[0] ^ other.m_blocks[0]) };
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        if (auto const diff = static_cast<block_type>(this->m_blocks[0] ^ other.m_blocks[0]); diff != zero) {
                                return { 0UZ, diff };
                        }
                        return { 1UZ, static_cast<block_type>(this->m_blocks[1] ^ other.m_blocks[1]) };
                } else {
                        auto const last = num_blocks() - 1UZ;
                        for (auto i = 0UZ; i < last; ++i) {
                                if (auto const diff = static_cast<block_type>(this->m_blocks[i] ^ other.m_blocks[i]); diff != zero) {
                                        return { i, diff };
                                }
                        }
                        return { last, static_cast<block_type>(this->m_blocks[last] ^ other.m_blocks[last]) };
                }
        }

private:
        // Whether any position strictly above the given one is set; the bit there is clear, so one shift down leaves exactly what is above it. [design.md#the-ordering-primitive]
        [[nodiscard]] constexpr auto any_above(std::size_t index, std::size_t offset) const noexcept
                -> bool
        {
                assert(not test((index * bits_per_block) + offset));
                if (static_cast<block_type>(m_blocks[index] >> offset) != zero) {
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

        // The block straddling index and index + 1: the high one shifted up by L_shift and the low one down by
        // R_shift, spliced into one. The funnel shift word_at and both shift operators are each made of.
        // [design.md#the-funnel-shift]
        //
        // Named as both shift operators name them, and taken as a pair: they already hold both halves as loop
        // invariants, so neither has one derived back from what the other was derived from. word_at, holding only
        // the one, spells the complement at its single call site. That the two add to the block width is the whole
        // contract, and the assert says so.
        [[nodiscard]] constexpr auto straddled_block(std::size_t index, std::size_t L_shift, std::size_t R_shift) const noexcept
                -> block_type
        {
                assert(L_shift + R_shift == bits_per_block);
                assert(0UZ < R_shift and R_shift < bits_per_block);
                assert(index + 1UZ < num_blocks());
                return static_cast<block_type>(
                        static_cast<block_type>(m_blocks[index + 1UZ] << L_shift) |
                        static_cast<block_type>(m_blocks[index] >> R_shift)
                );
        }

        [[nodiscard]] constexpr auto last_block() const noexcept
                -> std::size_t
        {
                return num_blocks() - 1UZ;
        }

        // The words a range of positions spans, each with the mask of what it holds: whole words, and a partial one at the end.
        template<class F>
        constexpr auto for_each_word(std::size_t n, std::size_t len, F f) const noexcept
                -> void
        {
                for (auto pos = n; pos < n + len; pos += bits_per_block) {
                        auto const count = std::ranges::min(bits_per_block, n + len - pos);
                        f(pos, count == bits_per_block ? ones : static_cast<block_type>(static_cast<block_type>(unit << count) - unit));
                }
        }

        // An iterator pair, not views::take, which libc++ 18 cannot form here. [design.md#libcxx-views-take]
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

        // static_used_bits at a run-time width, width zero selected rather than computed. [design.md#padding]
        [[nodiscard]] constexpr auto used_bits() const noexcept
                -> block_type
        {
                return size() == 0 ? zero : static_cast<block_type>(ones >> ((num_blocks() * bits_per_block) - size()));
        }

        [[nodiscard]] constexpr auto is_valid(std::size_t n [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (has_static_size and N == 0) {
                        // Unreachable: only an assert calls is_valid, and a zero-width block_sequence has no member that reaches one. Not removable either - MSVC's /W4 rejects a bare n < N as always false (C4296).
                        return false;                   // GCOVR_EXCL_LINE
                } else {
                        return n < size();
                }
        }

        [[nodiscard]] static constexpr auto index_offset(std::size_t n) noexcept
                -> xstd::div_result<std::size_t>
        {
                if constexpr (has_static_size and static_num_blocks == 1) {
                        return { .quotient = 0UZ, .remainder = n };
                } else {
                        return xstd::div(n, bits_per_block);
                }
        }

        // cl rejects a member of the explicit object parameter in a trailing return type (C2228); bit_array.hpp's result_t is the same idiom.
        template<class Self>
        using block_reference_t = std::conditional_t<
                std::is_const_v<std::remove_reference_t<Self>>, block_type const&, block_type&>;

        [[nodiscard]] constexpr auto block_mask(this auto&& self, std::size_t n) noexcept
                -> std::pair<block_reference_t<decltype(self)>, block_type>
        {
                auto const [ index, offset ] = index_offset(n);
                return { std::forward<decltype(self)>(self).m_blocks[index], static_cast<block_type>(unit << offset) };
        }

        constexpr auto erase_unused() noexcept
                -> void
        {
                if constexpr (has_static_size and static_has_unused_bits) {
                        m_blocks[static_last_block] &= static_used_bits;
                        assert(not detail::bits::intersects(m_blocks[static_last_block], static_unused_bits));
                } else if constexpr (not has_static_size) {
                        m_blocks[last_block()] &= used_bits();
                }
        }
};

// The three vehicles: a width in the type, a width on the heap, and a run-time width under a compile-time capacity of N bits.
template<xstd::unsigned_integer Block, std::size_t N>
using block_array = block_sequence<std::array<Block, num_blocks_v<Block, N>>, N>;

template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using block_vector = block_sequence<std::vector<Block, Allocator>>;

// Behind the feature macro until every library in the matrix has it; the storage needs nothing else, already satisfying block_storage.
#ifdef __cpp_lib_inplace_vector
template<xstd::unsigned_integer Block, std::size_t N>
using block_inplace_vector = block_sequence<std::inplace_vector<Block, num_blocks_v<Block, N>>>;
#endif

// Forwards and nothing more, reaching none of the generic scans. [design.md#the-cheapest-contract]
template<class Blocks, std::size_t N>
struct bit_traits<block_sequence<Blocks, N>>
{
        using bits_type = block_sequence<Blocks, N>;

        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size(bits_type const& c) noexcept -> std::size_t { return c.size(); }
        [[nodiscard]] static constexpr auto at(bits_type const& c, std::size_t n) noexcept -> bool { return c.test(n); }

        // set(n)/reset(n), there being no set(n, value) here; both assert, so the position is a precondition.
        static constexpr auto unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept
                -> void
        {
                if (value) {
                        c.set(n);
                } else {
                        c.reset(n);
                }
        }

        [[nodiscard]] static constexpr auto count(bits_type const& c) noexcept -> std::size_t { return c.count(); }

        // The three the sequence reading asks and the bitset reading already had: entries, so neither is synthesized here. [design.md#the-sequence-aggregates]
        [[nodiscard]] static constexpr auto all (bits_type const& c) noexcept -> bool { return c.all();  }
        [[nodiscard]] static constexpr auto any (bits_type const& c) noexcept -> bool { return c.any();  }
        [[nodiscard]] static constexpr auto none(bits_type const& c) noexcept -> bool { return c.none(); }

        // What mismatch is made of: the first differing block and its xor. [design.md#the-sequence-aggregates]
        [[nodiscard]] static constexpr auto first_difference(bits_type const& x, bits_type const& y) noexcept { return x.first_difference(y); }

        // The two entries the readings cannot synthesize: insert is the one operation that can grow, and fill is bulk. [design.md#what-the-trait-reconciles]
        // A run-time width grows to hold the position, as boost's does; n + 1 must be addressable, the ruled-out position being the one whose successor wraps.
        static constexpr auto insert(bits_type& c, std::size_t n) noexcept(bits_type::has_static_size)
                -> void
        {
                if constexpr (not bits_type::has_static_size) {
                        if (n >= c.size()) {
                                assert(n < std::numeric_limits<std::size_t>::max());
                                c.resize(n + 1UZ);
                        }
                }
                c.set(n);
        }
        static constexpr auto fill(bits_type& c, bool value) noexcept
                -> void
        {
                if (value) {
                        c.set();
                } else {
                        c.reset();
                }
        }

        [[nodiscard]] static constexpr auto num_blocks(bits_type const& c) noexcept -> std::size_t { return c.num_blocks(); }
        [[nodiscard]] static constexpr auto block(bits_type const& c, std::size_t i) noexcept { return c.block(i); }

        [[nodiscard]] static constexpr auto find_first(bits_type const& c) noexcept -> std::size_t { return c.find_first(); }
        [[nodiscard]] static constexpr auto find_last (bits_type const& c) noexcept -> std::size_t { return c.find_last();  }

        // The trait keeps the cheaper contracts. [design.md#the-cheapest-contract]
        [[nodiscard]] static constexpr auto find_next(bits_type const& c, std::size_t n) noexcept -> std::size_t { return c.exclusive_find_next(n); }
        [[nodiscard]] static constexpr auto find_prev(bits_type const& c, std::size_t n) noexcept -> std::size_t { return c.exclusive_find_prev(n); }

        // Three named entries, never one "lexicographical_three_way": the readings disagree, so the caller names the one it means. [design.md#two-readings-disagree]
        [[nodiscard]] static constexpr auto set_three_way     (bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return x.set_three_way(y);      }
        [[nodiscard]] static constexpr auto sequence_three_way(bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return x.sequence_three_way(y); }
        [[nodiscard]] static constexpr auto bitset_three_way  (bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return x.bitset_three_way(y);   }
};

}       // namespace xstd

#endif  // XSTD_BITS_BLOCK_SEQUENCE_HPP
