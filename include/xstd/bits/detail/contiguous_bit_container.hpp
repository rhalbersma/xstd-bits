//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BIT_CONTAINER_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BIT_CONTAINER_HPP

#include <xstd/bits/detail/allocator_base_type.hpp>          // allocator_base_type
#include <xstd/bits/detail/contiguous_block_range.hpp>   // contiguous_block_range
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
#include <cassert>                                           // assert
#include <compare>                                           // strong_ordering
#include <concepts>                                          // same_as
#include <cstddef>                                           // ptrdiff_t, size_t
#include <format>                                            // format
#include <functional>                                        // plus
#include <iterator>                                          // distance, forward_iterator, input_iterator, prev
#include <limits>                                            // numeric_limits
#include <ranges>                                            // begin, drop, iota, rbegin, rend, size, swap, take, transform, zip
                                                             // (views::drop_last when P22014R2 is accepted)
#include <source_location>                                   // source_location
#include <span>                                              // dynamic_extent
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

        static constexpr auto bits_per_block  = static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits);
        static constexpr auto has_static_size = N != std::dynamic_extent;

        // The width as a type, dynamic_extent where there is none: what a reading asks when it needs the width before an object exists.
        static constexpr std::size_t extent = N;

        // The widest width a size_t can count in whole blocks, and the widest a ptrdiff_t can: the two ceilings the readings choose between, neither of them enforced here. What the blocks can actually hold is narrower still, and that is max_size() and the two answers beside it.
        static constexpr auto max_num_blocks = std::numeric_limits<std::size_t>::max() / bits_per_block;
        static constexpr auto max_width      = max_num_blocks * bits_per_block;

        // A width whose positions a difference_type can all name: what a random access range over these blocks can address, and so what std::vector<bool> reports and refuses against. Whole blocks, like the one above.
        static constexpr auto max_addressable_num_blocks = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / bits_per_block;
        static constexpr auto max_addressable_width      = max_addressable_num_blocks * bits_per_block;

private:
        static constexpr auto static_num_bits   = has_static_size ? align_up(N, bits_per_block) : 0UZ;
        static constexpr auto static_num_blocks = has_static_size ? std::ranges::max(static_num_bits / bits_per_block, 1UZ) : 0UZ;
        static constexpr auto static_last_block = static_num_blocks - 1UZ;

        static constexpr auto left_bit = bits_per_block - 1UZ;
        static constexpr auto unit     = static_cast<block_type>( 1);
        static constexpr auto zero     = static_cast<block_type>( 0);
        static constexpr auto ones     = static_cast<block_type>(-1);

        // Width zero named, not computed: MSVC folds both ?: arms and answers C4293.
        static constexpr auto static_num_unused_bits = has_static_size ? static_num_bits - N : 0UZ;
        static constexpr auto static_used_bits       = has_static_size and N == 0 ? zero : shr(ones, static_num_unused_bits);
        static constexpr auto static_unused_bits     = static_cast<block_type>(~static_used_bits);
        static constexpr auto static_has_unused_bits = has_static_size and static_used_bits != ones;

        // How many blocks a run-time width needs, floored at one like num_blocks_v. Total over every size_t, and said as boost's own calc_num_blocks says it -- divide, then round up by the remainder -- because that CANNOT overflow, where align_up(n, bits_per_block) adds first and wraps for the 63 widths above max_width, rounding them to zero blocks that the floor then turns into one. A guard against that wrap is a guard against a spelling; this spelling has nothing to guard.
        [[nodiscard]] static constexpr auto blocks_for(std::size_t n) noexcept
                -> std::size_t
        {
                return std::ranges::max((n / bits_per_block) + (n % bits_per_block != 0UZ ? 1UZ : 0UZ), 1UZ);
        }

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

        // The width is a size_t, unless the blocks out-align one: then it is a block, which fills what would otherwise be padding in front of them.
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
        :
                m_size(n),
                m_blocks(make_blocks(n))
        {}

        // boost's allocator arguments, where the blocks take one: deduced and matched, so a storage without an allocator has no such constructor.
        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr explicit contiguous_bit_container(Alloc const& alloc)
        :
                m_blocks(blocks_for(0UZ), alloc)
        {}

        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr contiguous_bit_container(std::size_t n, Alloc const& alloc)
        :
                m_size(n),
                m_blocks(blocks_for(n), alloc)
        {}

        // [container.alloc.reqmts]'s allocator-extended copy and move; the moved-from is left empty whichever way the blocks went.
        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr contiguous_bit_container(contiguous_bit_container const& other, Alloc const& alloc)
        :
                m_size(other.m_size),
                m_blocks(other.m_blocks, alloc)
        {}

        template<class Alloc>
                requires (not has_static_size) and std::same_as<Alloc, typename Blocks::allocator_type>
        [[nodiscard]] constexpr contiguous_bit_container(contiguous_bit_container&& other, Alloc const& alloc)
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

        // Memberwise, width first: the unused bits are kept clear, so the blocks compare as the bits do, and a zero width through the one block it still holds.
        [[nodiscard]] friend constexpr auto operator==(contiguous_bit_container const&, contiguous_bit_container const&) noexcept -> bool = default;

        // The set reading's equality, which operator== is not: that one is width first, meaning the sequence reading and dynamic_bitset. Here width is capacity, so two storages holding the same positions are equal whatever their widths. A hidden friend beside the defaulted operator==, and for the reason the three orderings are: equality is a question about two values and neither is the subject, so x.set_equal(y) spelled a symmetry the operation has and the call did not.
        [[nodiscard]] friend constexpr auto set_equal(contiguous_bit_container const& x, contiguous_bit_container const& y) noexcept
                -> bool
        {
                if constexpr (has_static_size) {
                        // One width, so holding the same positions and being equal are the same statement.
                        return x == y;
                } else {
                        // ranges::equal over the shared prefix, NOT over the two block ranges: on two sized ranges it compares size() first and answers false without looking at an element, which is the one case this asks about. Taking the prefix as an ITERATOR PAIR keeps the answer and gets the algorithm, which lowers to a memcmp on trivially comparable contiguous blocks where all_of over a zip stays an element loop -- 2.15us to 1.29us over 4700 blocks. Not views::take, which libc++ 18 cannot form over these blocks, as all_but_last_are_ones already records. The other two block walks cannot follow: is_subset_of and intersects do bitwise work per block and have no such algorithm.
                        auto const shared = static_cast<std::ptrdiff_t>(std::ranges::min(x.num_blocks(), y.num_blocks()));
                        auto const xf = std::ranges::begin(x.m_blocks);
                        auto const yf = std::ranges::begin(y.m_blocks);
                        return
                                std::ranges::equal(xf, xf + shared, yf, yf + shared) and
                                (x.num_blocks() < y.num_blocks()
                                        ? not y.any_block_set(x.num_blocks(), y.num_blocks())
                                        : not x.any_block_set(y.num_blocks(), x.num_blocks()))
                        ;
                }
        }

        // No operator<=>: contiguous_bit_container is pure storage with no opinion on which reading orders it, so it names all three and picks none. A hidden friend, not a member: an ordering is a question about two values and neither is the subject, so x.set_lexicographical_compare_three_way(y) spelled a symmetry the operation has and the call did not.
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
                        auto const [ index, diff ] = x.first_difference(y);
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

        // The sequence reading a word at a time: whoever HOLDS the lowest differing position is greater, position 0 being the sequence's first element. Total across widths as the set reading is, the prefix clause living in the arm that needs it.
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
                        auto const [ index, diff ] = x.first_difference(y);
                        if (diff == zero) {
                                return std::strong_ordering::equal;
                        }
                        auto const offset = detail::bits::countr_zero(diff);
                        return detail::bits::intersects(x.m_blocks[index], shl(unit, offset))
                                ? std::strong_ordering::greater
                                : std::strong_ordering::less
                        ;
                }
        }

        // The bitset reading a word at a time: the bit string, most significant position first, IS the blocks from the top block down, the unused tail being clear, so the reading is the standard algorithm over the blocks reversed and there is nothing here to hand-roll. The degenerate widths need no arm of their own: a zero width still holds its one all-padding block, which is clear in both, and a one-block width is the algorithm's first step.
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

        // base positions and count more, saturated at the top of size_t rather than wrapped: the one addition every growth is spelled through, here and in the three readings. A wrapped sum is small, so it passes the ceiling it was meant to fail and then sizes the blocks for far fewer positions than the operation goes on to write; a saturated one fails that ceiling, which is what an unrepresentable width should do. blocks_for is where it fails, with std::length_error.
        // The ceiling, and no longer the storage's own: blocks_for divides and cannot wrap, so nothing here needs a bounded width to stay safe. What is left is a policy, and the policy belongs to the reading, because the counterparts disagree about it. std::vector throws length_error for a size it cannot represent and the sequence reading says so with this; the set reading refuses a key past the widest it could grow to, and says so with this; boost::dynamic_bitset has no ceiling at all -- its calc_num_blocks divides as blocks_for now does, and a width it cannot hold reaches the allocator and answers bad_alloc. The bitset reading is a strict extension of boost, so it calls none of this and answers as boost does.
        [[nodiscard]] static constexpr auto check_width(std::size_t n)
                -> std::size_t
        {
                if (n > max_width) {
                        throw length_error(n);
                }
                return n;
        }

        // The same refusal against the other ceiling, for the reading whose counterpart keeps that one: [container.reqmts] puts a sequence's at max_size(), and a size past what a distance can name is std::length_error there as it is in std::vector<bool>.
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

        // In bits: the width, or the widest whole number of blocks the blocks can hold and the address space can count. The set reading's answer, having no counterpart that names another.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> std::size_t
        {
                if constexpr (has_static_size) {
                        return N;
                } else {
                        return std::ranges::min(m_blocks.max_size(), max_num_blocks) * bits_per_block;
                }
        }

        // boost::dynamic_bitset's answer over the same blocks, which saturates where the one above clamps: boost multiplies the blocks' limit by the bits in one and gives SIZE_MAX where that product is not representable, sixty-three positions above max_width. Said as a sum and not as boost's choice: bits_per_block is a power of two, so SIZE_MAX is max_width plus one block's bits less one, and the clamped answer needs exactly that much back wherever the clamp bit. A choice would be a branch whose two arms belong to different allocators -- std::allocator's ceiling always saturates -- and no one instantiation could take both.
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

        // std::vector<bool>'s answer over the same blocks, which clamps further: a random access range's positions are counted by a difference_type, so a width above max_addressable_width has distances it cannot represent, whatever the blocks could hold.
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

        // The block, and NOT operator[]: a subscript on a bit container means a bit, which is what std::bitset and vector<bool> both spell that way and what test() answers here. Naming the block access block() keeps c[n] unclaimed rather than making it mean something no other bit container means by it.
        [[nodiscard]] constexpr auto block(std::size_t i) const noexcept
                -> block_type
        {
                assert(i < num_blocks());
                return m_blocks[i];
        }

        // The write side, a reference rather than a setter: a caller writing blocks writes them, and restores the invariant itself with erase_unused when it is done. A setter could only erase after every block, which is once per block where once per loop will do.
        [[nodiscard]] constexpr auto block(std::size_t i) noexcept
                -> block_type&
        {
                assert(i < num_blocks());
                return m_blocks[i];
        }

        // Public, because restoring the invariant belongs to whoever wrote the blocks that broke it.
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

        // A word at any position, aligned or not: the bits [n, n + bits_per_block), the clear tail and nothing beyond the last block.
        [[nodiscard]] constexpr auto block_at(std::size_t n) const noexcept
                -> block_type
        {
                auto const [ index, offset ] = index_offset(n);
                assert(index < num_blocks());
                if (offset == 0UZ or index == last_block()) {
                        return shr(m_blocks[index], offset);
                }
                return straddled_block(index, bits_per_block - offset, offset);
        }

        // The write side of block_at, masked: the bits of value under mask land at [n, n + bits_per_block), split over two blocks where n is not aligned, and the tail stays clear.
        // The word-level primitive, which writes what the mask selects and restores nothing: the mask is required to stay inside size(), so the padding above it is untouched and the ranged forms below need no erase at all. libstdc++ splits the same way, _Base_bitset knowing only its word count and bitset<_Nb> calling _M_do_sanitize once after; here one type knows both, and the erase is left to the four operations that can actually dirty the padding -- a write through block(i), operator<<=, flip() and resize.
        constexpr auto block_at(std::size_t n, block_type value, block_type mask) noexcept
                -> void
        {
                auto const [ index, offset ] = index_offset(n);
                assert(index < num_blocks());
                assert(n + bits_per_block <= size() or shr(mask, size() - n) == zero);
                auto const bits = static_cast<block_type>(value & mask);

                // Each step lands back in block_type: a promoted operand feeding the next bitwise operator is what bugprone-signed-bitwise reads.
                auto const low_kept = static_cast<block_type>(m_blocks[index] & static_cast<block_type>(~shl(mask, offset)));
                m_blocks[index] = static_cast<block_type>(low_kept | shl(bits, offset));
                if (offset != 0UZ and index != last_block()) {
                        auto const shift = bits_per_block - offset;
                        auto const high_kept = static_cast<block_type>(m_blocks[index + 1UZ] & static_cast<block_type>(~shr(mask, shift)));
                        m_blocks[index + 1UZ] = static_cast<block_type>(high_kept | shr(bits, shift));
                }

        }

        // boost's ranged forms, a word at a time through block_at: [n, n + len) set, cleared or flipped, the rest untouched. The precondition is said as a subtraction throughout, n + len being the sum that wraps for an n near the top of size_t -- and a wrapped sum is below any width, so the assertion it was meant to fail is the one it passes.
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
                        auto const [ index, offset ] = index_offset(n);
                        if (auto const block = shr(m_blocks[index], offset); block != zero) {
                                return n + detail::bits::countr_zero(block);
                        }
                        if (index == 0 and m_blocks[1] != zero) {
                                return bits_per_block + detail::bits::countr_zero(m_blocks[1]);
                        }
                } else {
                        // No offset != 0 guard: >> 0 is the identity.
                        auto [ index, offset ] = index_offset(n);
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
                        auto const [ index, offset ] = index_offset(n);
                        if (auto const block = shl(m_blocks[index], left_bit - offset); block != zero) {
                                return n - detail::bits::countl_zero(block);
                        }
                        // Reaching here at index 0 would break the precondition the general path asserts instead.
                        assert(index == 1);
                        assert(m_blocks[0] != zero);
                        return left_bit - detail::bits::countl_zero(m_blocks[0]);
                } else {
                        auto [ index, offset ] = index_offset(n);
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

        // Total across two widths, and reading-neutral: the blocks the other storage does not have read as the zero the invariant already keeps above its size(), so the result is this storage's own width restricted or left alone. Growing is NOT here -- that is the set reading's rule about capacity, and belongs to the reading that has it.
        constexpr auto operator&=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] &= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] &= other.m_blocks[0];
                        this->m_blocks[1] &= other.m_blocks[1];
                } else if constexpr (not (has_static_size and N == 0)) {
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

        // Total across two widths, and reading-neutral: the blocks the other storage does not have read as the zero the invariant already keeps above its size(), so the result is this storage's own width restricted or left alone. Growing is NOT here -- that is the set reading's rule about capacity, and belongs to the reading that has it.
        constexpr auto operator|=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] |= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] |= other.m_blocks[0];
                        this->m_blocks[1] |= other.m_blocks[1];
                } else if constexpr (not (has_static_size and N == 0)) {
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

        // Total across two widths, and reading-neutral: the blocks the other storage does not have read as the zero the invariant already keeps above its size(), so the result is this storage's own width restricted or left alone. Growing is NOT here -- that is the set reading's rule about capacity, and belongs to the reading that has it.
        constexpr auto operator^=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] ^= other.m_blocks[0];
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] ^= other.m_blocks[0];
                        this->m_blocks[1] ^= other.m_blocks[1];
                } else if constexpr (not (has_static_size and N == 0)) {
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

        // Total across two widths, and reading-neutral: the blocks the other storage does not have read as the zero the invariant already keeps above its size(), so the result is this storage's own width restricted or left alone. Growing is NOT here -- that is the set reading's rule about capacity, and belongs to the reading that has it.
        constexpr auto operator-=(contiguous_bit_container const& other [[maybe_unused]]) noexcept
                -> contiguous_bit_container&
        {
                if constexpr (has_static_size and N > 0 and static_num_blocks == 1) {
                        this->m_blocks[0] &= static_cast<block_type>(~other.m_blocks[0]);
                } else if constexpr (has_static_size and static_num_blocks == 2) {
                        this->m_blocks[0] &= static_cast<block_type>(~other.m_blocks[0]);
                        this->m_blocks[1] &= static_cast<block_type>(~other.m_blocks[1]);
                } else if constexpr (not (has_static_size and N == 0)) {
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
                        // m_blocks[0] <<= n narrows the promoted int back to a block_type implicitly, which -fsanitize=implicit-conversion aborts on once a bit shifts out.
                        m_blocks[0] = shl(m_blocks[0], n);
                } else {
                        auto const [ n_blocks, L_shift ] = xstd::div(n, bits_per_block);
                        // Restated because GCC drops the range through xstd::div.
                        assert(n_blocks <= last_block());
                        if (L_shift == 0) {
                                std::shift_right(std::ranges::begin(m_blocks), std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks));
                        } else {
                                auto const R_shift = bits_per_block - L_shift;
                                for (auto i = last_block(); i > n_blocks; --i) {
                                        // Read one block lower than the destination: the splice of [i - n_blocks - 1, i - n_blocks].
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
                        // m_blocks[0] >>= n narrows the promoted int back to a block_type implicitly, which -fsanitize=implicit-conversion instruments.
                        m_blocks[0] = shr(m_blocks[0], n);
                } else {
                        auto const [ n_blocks, R_shift ] = xstd::div(n, bits_per_block);
                        // See operator<<=: the same bound, for the same reason.
                        assert(n_blocks <= last_block());
                        if (R_shift == 0) {
                                std::shift_left(std::ranges::begin(m_blocks), std::ranges::end(m_blocks), static_cast<std::ptrdiff_t>(n_blocks));
                        } else {
                                auto const L_shift = bits_per_block - R_shift;
                                for (auto i = 0UZ; i + n_blocks < last_block(); ++i) {
                                        // Which is block_at(i * bits_per_block + n), reached without recomputing the division.
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
                } else if constexpr (not (has_static_size and N == 0)) {
                        for (auto const i : std::views::iota(0UZ, num_blocks())) {
                                m_blocks[i] = static_cast<block_type>(~m_blocks[i]);
                        }
                }
                erase_unused();
                return *this;
        }

        constexpr auto swap(contiguous_bit_container& other)
                noexcept(noexcept(std::ranges::swap(this->m_size, other.m_size)) and noexcept(std::ranges::swap(this->m_blocks, other.m_blocks)))
                -> void
        {
                // m_size is empty_type under a static width, and swapping that is a no-op.
                std::ranges::swap(this->m_size,   other.m_size);
                std::ranges::swap(this->m_blocks, other.m_blocks);
        }

        // ranges::swap finds a free swap by ADL and a member never, so the member above is reached through this one and not directly; without it every adaptor's ranges::swap(m_bits, other.m_bits) moves a whole contiguous_bit_container three times instead of swapping its blocks once. Hidden rather than at namespace scope, as the three adaptors' are: one shape for the whole tree.
        friend constexpr auto swap(contiguous_bit_container& x, contiguous_bit_container& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }

        // Growth, at a run-time width alone; every path leaves the unused tail clear, so the block walks read nothing above size().
        constexpr auto resize(std::size_t n, bool value = false)
                -> void
                requires (not has_static_size)
        {
                // The ceiling first, so a width this storage cannot count throws before anything below is written.
                resize_to(n, value);
        }

        // Widen just enough to hold every element the other has, and not at all when it has none above this width. Its largest element, not its size(), is what the growing insert of each in turn would have reached. A static width has nothing to widen and no other width to meet, so there the whole thing is nothing. Through resize_to, as clear() and pop_back() are: the width comes from the other storage's own, so it is one this one can count already, and asking the ceiling here would put its throw on every set operation across widths.
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

        // Boost's append: the block's bits become the positions [size(), size() + bits_per_block), split over two blocks where size() is not aligned.
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
                -> contiguous_bit_container&
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

        // insert(n) above is partial, n being a precondition; this one is total, a position past the end growing a run-time width to admit it and a static one having nowhere to grow. The set reading's insert is the one operation that can grow, which is the whole of the difference.
        constexpr auto growing_insert(std::size_t n) noexcept(has_static_size)
                -> bool
        {
                if constexpr (not has_static_size) {
                        if (n >= size()) {
                                // width_sum, not n + 1: the position past the top of size_t is one the set reading accepts as a key, and n + 1 there is a width of zero.
                                resize(width_sum(n, 1UZ));
                                set(n);
                                return true;
                        }
                }
                return insert(n);
        }

        // set(n) and reset(n) under one name, for a reading that has the value in hand rather than the verb. Deliberately not spelled set(n, value): that is std::bitset's two-argument set, and this container's not having it is one of the five absences that make contiguous_bit_sequence the intersection of the three vocabularies rather than their union. TheCommonVocabulary asserts it.
        constexpr auto assign(std::size_t n, bool value) noexcept
                -> contiguous_bit_container&
        {
                return value ? set(n) : reset(n);
        }

        // The bulk counterpart, and not an overload of set either: set(bool) would be ambiguous with set(std::size_t) for a literal 0, both conversions being standard.
        constexpr auto fill(bool value) noexcept
                -> contiguous_bit_container&
        {
                return value ? set() : reset();
        }

        constexpr auto reset(std::size_t n) noexcept
                -> contiguous_bit_container&
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
                -> contiguous_bit_container&
        {
                assert(is_valid(n));
                auto&& [ block, mask ] = block_mask(n);
                block ^= mask;
                return *this;
        }

        // test, not operator[]: this returns bool, std::bitset's a proxy.
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

        [[nodiscard]] constexpr auto is_subset_of(contiguous_bit_container const& other [[maybe_unused]]) const noexcept
                -> bool
        {
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
                        // zip stops at the shorter, which is exactly the blocks both storages have.
                        auto const shared = std::ranges::all_of(
                                std::views::zip(this->m_blocks, other.m_blocks), [](auto&& _) { auto&& [ lhs, rhs] = _;
                                return detail::bits::is_subset_of(lhs, rhs);
                        });
                        if constexpr (has_static_size) {
                                // One width, so the shared blocks are all the blocks and there is nothing past them to ask about.
                                return shared;
                        } else {
                                // Above the shared blocks only ours can hold a position, and any position of ours the other cannot hold denies the subset.
                                return shared and not this->any_block_set(other.num_blocks(), this->num_blocks());
                        }
                }
        }

        // A proper subset is a subset that differs, and both halves are already here: the unrolled arms are is_subset_of's, and != is the defaulted memberwise comparison.
        [[nodiscard]] constexpr auto is_proper_subset_of(contiguous_bit_container const& other) const noexcept
                -> bool
        {
                return is_subset_of(other) and not set_equal(*this, other);
        }

        [[nodiscard]] constexpr auto intersects(contiguous_bit_container const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                // Only the blocks both storages have can meet: above them one of the two holds nothing, so zip stopping at the shorter is the whole question.
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

        // A hidden friend beside the member, the pair swap already carries: a meets b exactly when b meets a, so the symmetric spelling is the honest one, and intersects is to set_intersection what contains is to find -- a predicate over the free two-range algorithm, not a lookup asked of one value. The member stays and does the work, which set_equal's did not have to. Both adaptors carry a MEMBER named intersects -- boost's spelling, which the bitset reading keeps by the extension rule -- and a member of that name stops ADL at the call site ([basic.lookup.argdep]/1: ordinary lookup finding a class member ends the search), so from inside those members the friend is unreachable by any spelling. Measured, not assumed.
        [[nodiscard]] friend constexpr auto intersects(contiguous_bit_container const& x, contiguous_bit_container const& y) noexcept
                -> bool
        {
                return x.intersects(y);
        }

        // The first block at which two values differ, with that block's xor; equal values answer the last block and a zero xor, every arm alike.
        [[nodiscard]] constexpr auto first_difference(contiguous_bit_container const& other) const noexcept
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
        // Whether any position strictly above the given one is set; the bit there is clear, so one shift down leaves exactly what is above it.
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

        // Comparing two widths needs blocks one storage does not have. They hold no position, and the invariant already keeps the padding above size() clear, so reading them as zero is not a convention but the same fact one block further out.
        [[nodiscard]] constexpr auto padded_block(std::size_t index) const noexcept
                -> block_type
        {
                return index < num_blocks() ? m_blocks[index] : zero;
        }

        // Whether any block in the half-open range is set. An empty or inverted range answers no, which is what a storage with nothing past the other's last block says.
        [[nodiscard]] constexpr auto any_block_set(std::size_t first, std::size_t last) const noexcept
                -> bool
        {
                return std::ranges::any_of(std::views::iota(first, std::ranges::max(first, last)), [this](std::size_t index) -> bool { return m_blocks[index] != zero; });
        }

        // any_above with no precondition on the index: the position may lie past this storage's last block, where it holds neither the position nor anything above it.
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

        // The set ordering across two widths. Lexicographic order over the ascending positions turns on ONE position: the lowest at which the two disagree. Whoever lacks it is less -- holding a larger element there, or, when it holds nothing above it at all, because its positions are a proper prefix of the other's and it runs out first.
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

        // The sequence ordering across two widths. The same one position decides -- the lowest at which the two disagree -- but here it decides alone: position 0 is the sequence's FIRST element, so whoever holds that position is greater and nothing above it is consulted, where the set reading has to ask. Agreeing at every position the two share leaves only length, and the shorter is then a proper prefix of the longer and so less.
        [[nodiscard]] constexpr auto padded_sequence_three_way(contiguous_bit_container const& other) const noexcept
                -> std::strong_ordering
        {
                auto const n = std::ranges::max(this->num_blocks(), other.num_blocks());
                auto const index = padded_first_difference(other, n);
                if (index == n) {
                        // The two answers this can give, rather than size() <=> size(), whose equal case is unreachable: the caller arrives here only with the sizes differing.
                        return this->size() < other.size() ? std::strong_ordering::less : std::strong_ordering::greater;
                }
                auto const diff = static_cast<block_type>(this->padded_block(index) ^ other.padded_block(index));
                auto const offset = static_cast<std::size_t>(detail::bits::countr_zero(diff));
                return detail::bits::intersects(this->padded_block(index), shl(unit, offset))
                        ? std::strong_ordering::greater
                        : std::strong_ordering::less
                ;
        }

        // The block straddling index and index + 1: the high one shifted up by L_shift and the low one down by R_shift, spliced into one.
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

        // The words a range of positions spans, each with the mask of what it holds: whole words, and a partial one at the end.
        template<class F>
        constexpr auto for_each_block(std::size_t n, std::size_t len, F f) const noexcept
                -> void
        {
                for (auto pos = n; pos < n + len; pos += bits_per_block) {
                        auto const count = std::ranges::min(bits_per_block, n + len - pos);
                        f(pos, count == bits_per_block ? ones : static_cast<block_type>(shl(unit, count) - unit));
                }
        }

        // An iterator pair, not views::take, which libc++ 18 cannot form here.
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
                        // Unreachable: only an assert calls is_valid, and a zero-width contiguous_bit_container has no member that reaches one. Not removable either - MSVC's /W4 rejects a bare n < N as always false (C4296).
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
                return { std::forward<decltype(self)>(self).m_blocks[index], shl(unit, offset) };
        }

        // The growth itself, over a width already known to be one this storage can count: clear() and pop_back() reach it directly, naming no width of their own.
        constexpr auto resize_to(std::size_t n, bool value)
                -> void
                requires (not has_static_size)
        {
                auto const count = blocks_for(n);
                // Growing with ones: the tail above size() in the last block is clear by the invariant, and becomes the first new bits.
                if (value and n > size()) {
                        m_blocks[last_block()] |= static_cast<block_type>(~used_bits());
                }
                m_blocks.resize(count, value ? ones : zero);
                m_size = n;
                erase_unused();
        }

        // std::length_error, which is what a container throws for a size it cannot represent; the three readings inherit it through every growth, having no ceiling of their own to name.
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

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_CONTIGUOUS_BIT_CONTAINER_HPP
