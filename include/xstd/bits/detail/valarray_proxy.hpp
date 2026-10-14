//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_VALARRAY_PROXY_HPP
#define XSTD_BITS_DETAIL_VALARRAY_PROXY_HPP

#include <xstd/bits/bit_gslice.hpp>                   // bit_gslice
#include <xstd/bits/bit_slice.hpp>                    // bit_slice
#include <xstd/bits/detail/bit_block_container.hpp>   // bit_block_container
#include <xstd/bits/detail/block_compress_expand.hpp> // block_width, compress_blocks, expand_blocks, expand_indices, fill_blocks, fill_indices, gather_indices, scatter_and, scatter_assign, scatter_or, scatter_xor, slice_masks, stored_masks
#include <xstd/bits/detail/borrowed_bits.hpp>         // borrowed_bits
#include <xstd/bits/detail/gslice_index.hpp>          // gslice_access, gslice_index
#include <xstd/bits/detail/intrin.hpp>                // popcount
#include <xstd/bits/detail/ownership.hpp>             // owned_bits_t, owner_reading, storage_access, valarray_reading_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <algorithm>                                  // all_of, count
#include <cassert>                                    // assert
#include <concepts>                                   // derived_from, same_as
#include <cstddef>                                    // size_t
#include <iterator>                                   // ssize
#include <span>                                       // span
#include <type_traits>                                // remove_cvref_t
#include <utility>                                    // forward
#include <valarray>                                   // gslice, slice, valarray
#include <vector>                                     // vector

// The positions [valarray.sub]'s four subscripts select, and the proxy that writes through them to its owner's blocks.
namespace xstd::bits::detail {

// A valarray owner over blocks of type Block, which is what a proxy over such blocks takes as an array.
template<class V, class Block>
concept valarray_owner_of = owner_reading<std::remove_cvref_t<V>, valarray_reading_tag> and std::same_as<typename owned_bits_t<std::remove_cvref_t<V>>::block_type, Block>;

// Whether every position is below n, which each subscript asserts of the positions it selects.
[[nodiscard]] constexpr auto all_below(std::span<std::size_t const> indices, std::size_t n) noexcept
        -> bool
{
        return std::ranges::all_of(indices, [=](std::size_t i) noexcept -> bool { return i < n; });
}

// Whether no position is named twice, which [indirect.array.assign] leaves undefined.
[[nodiscard]] constexpr auto names_each_once(std::span<std::size_t const> indices, std::size_t width)
        -> bool
{
        auto seen = std::vector<bool>(width);
        for (auto const i : indices) {
                seen[i] = true;
        }
        return std::ranges::count(seen, true) == std::ssize(indices);
}

// [class.slice]: a run at stride one, a mask per block below the block width, and one position at a time beyond it.
class slice_selection
{
        std::size_t m_start;
        std::size_t m_size;
        std::size_t m_stride;

        template<class Block>
        [[nodiscard]] constexpr auto is_strided() const noexcept
                -> bool
        {
                return m_size != 0UZ and 1UZ < m_stride and m_stride < block_width<Block>;
        }

        template<class Block>
        [[nodiscard]] constexpr auto masks() const noexcept
                -> slice_masks<Block>
        {
                return slice_masks<Block>(m_start, m_size, m_stride);
        }

        [[nodiscard]] constexpr auto index() const noexcept
        {
                return [start = m_start, stride = m_stride](std::size_t j) noexcept -> std::size_t { return start + (j * stride); };
        }

public:
        [[nodiscard]] constexpr explicit slice_selection(xstd::bit_slice const& s) noexcept
                : m_start(s.start())
                , m_size(s.size())
                , m_stride(s.stride())
        {}

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                return m_size;
        }

        template<class Src, class Block>
        constexpr auto gather(Src const& src, borrowed_bits<Block>& dst) const noexcept
                -> void
        {
                if (m_stride == 1UZ) {
                        gather_run(src, m_start, m_size, dst);
                } else if (is_strided<Block>()) {
                        compress_blocks(src, masks<Block>(), dst);
                } else {
                        gather_indices(src, m_size, index(), dst);
                }
        }

        template<class Src, class Block, class F>
        constexpr auto expand(Src const& src, borrowed_bits<Block>& dst, F f) const noexcept
                -> void
        {
                if (m_stride == 1UZ) {
                        expand_run(src, m_start, m_size, dst, f);
                } else if (is_strided<Block>()) {
                        expand_blocks(src, masks<Block>(), dst, f);
                } else {
                        expand_indices(src, m_size, index(), dst, f);
                }
        }

        template<class Block, class F>
        constexpr auto fill(bool value, borrowed_bits<Block>& dst, F f) const noexcept
                -> void
        {
                if (m_stride == 1UZ) {
                        fill_run(value, m_start, m_size, dst, f);
                } else if (is_strided<Block>()) {
                        fill_blocks(value, masks<Block>(), dst, f);
                } else {
                        fill_indices(value, m_size, index(), dst, f);
                }
        }
};

// [class.gslice]: a bit_gslice's index, its words as masks where the positions ascend, one by one where they do not.
class gslice_selection
{
        xstd::bit_gslice m_gslice;

        [[nodiscard]] auto index() const noexcept
                -> gslice_index const&
        {
                return gslice_access::index(m_gslice);
        }

        [[nodiscard]] auto position() const noexcept
        {
                return [positions = std::span(index().positions)](std::size_t j) noexcept -> std::size_t { return positions[j]; };
        }

        template<class Block>
        [[nodiscard]] auto masks() const noexcept
                -> word_masks<Block>
        {
                return word_masks<Block>(index().words, index().width);
        }

public:
        [[nodiscard]] explicit gslice_selection(xstd::bit_gslice g) noexcept
                : m_gslice(std::move(g))
        {}

        [[nodiscard]] auto size() const noexcept
                -> std::size_t
        {
                return index().count;
        }

        [[nodiscard]] auto within(std::size_t n) const noexcept
                -> bool
        {
                return index().width <= n;
        }

        template<class Src, class Block>
        auto gather(Src const& src, borrowed_bits<Block>& dst) const noexcept
                -> void
        {
                if (index().is_masked()) {
                        compress_blocks(src, masks<Block>(), dst);
                } else {
                        gather_indices(src, size(), position(), dst);
                }
        }

        template<class Src, class Block, class F>
        auto expand(Src const& src, borrowed_bits<Block>& dst, F f) const noexcept
                -> void
        {
                if (index().is_masked()) {
                        expand_blocks(src, masks<Block>(), dst, f);
                } else {
                        expand_indices(src, size(), position(), dst, f);
                }
        }

        template<class Block, class F>
        auto fill(bool value, borrowed_bits<Block>& dst, F f) const noexcept
                -> void
        {
                if (index().is_masked()) {
                        fill_blocks(value, masks<Block>(), dst, f);
                } else {
                        fill_indices(value, size(), position(), dst, f);
                }
        }
};

// [template.mask.array]: the mask's own blocks, each compressed or expanded whole.
template<xstd::unsigned_integer Block>
class mask_selection
{
        std::span<Block const> m_masks;
        std::size_t m_size = 0UZ;

public:
        [[nodiscard]] constexpr explicit mask_selection(std::span<Block const> masks) noexcept
                : m_masks(masks)
        {
                for (auto const mask : m_masks) {
                        m_size += detail::popcount(mask);
                }
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                return m_size;
        }

        template<class Src>
        constexpr auto gather(Src const& src, borrowed_bits<Block>& dst) const noexcept
                -> void
        {
                compress_blocks(src, stored_masks<Block>(m_masks), dst);
        }

        template<class Src, class F>
        constexpr auto expand(Src const& src, borrowed_bits<Block>& dst, F f) const noexcept
                -> void
        {
                expand_blocks(src, stored_masks<Block>(m_masks), dst, f);
        }

        template<class F>
        constexpr auto fill(bool value, borrowed_bits<Block>& dst, F f) const noexcept
                -> void
        {
                fill_blocks(value, stored_masks<Block>(m_masks), dst, f);
        }
};

// [template.indirect.array]: the indices of the array it refers to, one by one; a write may name each position once.
class indirect_selection
{
        std::span<std::size_t const> m_indices;

        [[nodiscard]] constexpr auto index() const noexcept
        {
                return [indices = m_indices](std::size_t j) noexcept -> std::size_t { return indices[j]; };
        }

public:
        [[nodiscard]] explicit indirect_selection(std::valarray<std::size_t> const& indices) noexcept
                : m_indices(indices)
        {}

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                return m_indices.size();
        }

        [[nodiscard]] constexpr auto within(std::size_t n) const noexcept
                -> bool
        {
                return all_below(m_indices, n);
        }

        template<class Src, class Block>
        constexpr auto gather(Src const& src, borrowed_bits<Block>& dst) const noexcept
                -> void
        {
                gather_indices(src, size(), index(), dst);
        }

        template<class Src, class Block, class F>
        constexpr auto expand(Src const& src, borrowed_bits<Block>& dst, F f) const
                -> void
        {
                assert(names_each_once(m_indices, dst.size()));
                expand_indices(src, size(), index(), dst, f);
        }

        template<class Block, class F>
        constexpr auto fill(bool value, borrowed_bits<Block>& dst, F f) const
                -> void
        {
                assert(names_each_once(m_indices, dst.size()));
                fill_indices(value, size(), index(), dst, f);
        }
};

// The one door through which an owner makes a proxy, asks its size and gathers its elements, and a proxy another's.
struct proxy_access
{
        template<class Proxy, class... Args>
        [[nodiscard]] static constexpr auto make(Args&&... args)
                -> Proxy
        {
                return Proxy(std::forward<Args>(args)...);
        }

        template<class Proxy>
        [[nodiscard]] static constexpr auto size(Proxy const& p) noexcept
                -> std::size_t
        {
                return p.size();
        }

        template<class Proxy, class Block>
        static constexpr auto gather(Proxy const& p, borrowed_bits<Block>& dst) noexcept
                -> void
        {
                p.gather(dst);
        }
};

template<class Selection, xstd::unsigned_integer Block>
class valarray_proxy;

// One of the four proxies over blocks of type Block.
template<class P, class Block>
concept valarray_proxy_of =
        std::derived_from<P, valarray_proxy<slice_selection, Block>> or
        std::derived_from<P, valarray_proxy<gslice_selection, Block>> or
        std::derived_from<P, valarray_proxy<mask_selection<Block>, Block>> or
        std::derived_from<P, valarray_proxy<indirect_selection, Block>>;

// An array a proxy takes: an owner of the same blocks, or another proxy, which first gathers what it selects.
template<class A, class Block>
concept valarray_operand_of = valarray_owner_of<A, Block> or valarray_proxy_of<A, Block>;

// The owner's blocks and the positions selected in them; a write through a const proxy writes the owner.
template<class Selection, xstd::unsigned_integer Block>
class valarray_proxy
{
        std::span<Block> m_blocks;
        Selection m_selection;

        [[nodiscard]] constexpr auto bits() const noexcept
                -> borrowed_bits<Block>
        {
                return borrowed_bits<Block>(m_blocks);
        }

        template<class A, class F>
        constexpr auto scatter(A const& a, F f) const
                -> void
        {
                auto dst = bits();
                if constexpr (valarray_owner_of<A, Block>) {
                        assert(a.size() == size());
                        // Read through a borrowed view, so every owner over one block type runs one scatter.
                        auto const blocks = storage_access::bits(a).blocks();
                        auto const src    = borrowed_bits<Block>(std::span(const_cast<Block*>(blocks.data()), blocks.size())); // NOLINT(cppcoreguidelines-pro-type-const-cast): the view is only read.
                        m_selection.expand(src, dst, f);
                } else {
                        // Gathered first, so a proxy over the same owner reads every position before any is written.
                        auto const n = proxy_access::size(a);
                        assert(n == size());
                        auto blocks = std::vector<Block>(bit_block_container<std::vector<Block>>::blocks_for(n));
                        auto src    = borrowed_bits<Block>(std::span(blocks));
                        proxy_access::gather(a, src);
                        m_selection.expand(src, dst, f);
                }
        }

        template<class F>
        constexpr auto fill(bool value, F f) const
                -> void
        {
                auto dst = bits();
                m_selection.fill(value, dst, f);
        }

        friend struct proxy_access;

protected:
        [[nodiscard]] constexpr valarray_proxy(std::span<Block> blocks, Selection selection) noexcept
                : m_blocks(blocks)
                , m_selection(std::move(selection))
        {}

        [[nodiscard]] valarray_proxy(valarray_proxy const&) = default;

        ~valarray_proxy() = default;

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                return m_selection.size();
        }

        constexpr auto gather(borrowed_bits<Block>& dst) const noexcept
                -> void
        {
                auto const src = bits();
                m_selection.gather(src, dst);
        }

public:
        using value_type = bool;

        // Never a rebinding: each proxy assigns its elements through a const object of its own type.
        auto operator=(valarray_proxy const&) const -> valarray_proxy const& = delete;

        // [slice.arr.assign] and its three siblings: the array's elements, in order, at the selected positions.
        template<class A>
                requires valarray_operand_of<A, Block>
        constexpr auto operator=(A const& a) const // NOLINT(misc-unconventional-assign-operator, cppcoreguidelines-c-copy-assignment-signature): [valarray]'s proxies return void.
                -> void
        {
                scatter(a, scatter_assign);
        }

        // The compound assignments the owners have, against an array and against a scalar: their logical three.
        template<class A>
                requires valarray_operand_of<A, Block>
        constexpr auto operator&=(A const& a) const
                -> void
        {
                scatter(a, scatter_and);
        }

        template<class A>
                requires valarray_operand_of<A, Block>
        constexpr auto operator|=(A const& a) const
                -> void
        {
                scatter(a, scatter_or);
        }

        template<class A>
                requires valarray_operand_of<A, Block>
        constexpr auto operator^=(A const& a) const
                -> void
        {
                scatter(a, scatter_xor);
        }

        constexpr auto operator=(value_type const& value) const // NOLINT(misc-unconventional-assign-operator, cppcoreguidelines-c-copy-assignment-signature): [valarray]'s proxies return void.
                -> void
        {
                fill(value, scatter_assign);
        }

        constexpr auto operator&=(value_type const& value) const
                -> void
        {
                fill(value, scatter_and);
        }

        constexpr auto operator|=(value_type const& value) const
                -> void
        {
                fill(value, scatter_or);
        }

        constexpr auto operator^=(value_type const& value) const
                -> void
        {
                fill(value, scatter_xor);
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_VALARRAY_PROXY_HPP
