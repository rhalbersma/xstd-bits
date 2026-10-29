//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_WALK_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_WALK_HPP

#include <xstd/bits/bit_algorithm/detail/sequence.hpp> // first_position
#include <xstd/bits/detail/key_order.hpp>              // descends
#include <xstd/bits/detail/ownership.hpp>              // reads_keys, storage_access
#include <xstd/bits/detail/shift.hpp>                  // shr
#include <xstd/bits/detail/walk_block.hpp>             // walk_block_down, walk_block_up
#include <xstd/misc/concepts/specialization_of.hpp>    // specialization_of
#include <algorithm>                                   // min
#include <cstddef>                                     // size_t
#include <ranges>                                      // iota, ref_view, reverse_view, size
#include <type_traits>                                 // remove_cvref_t
#include <utility>                                     // declval

namespace xstd::bits::detail {

// A prvalue from a named parameter: MSVC 17 has no auto(x), which is [P0849R8]'s spelling of this.
template<class T>
[[nodiscard]] constexpr auto decay_copy(T value) noexcept
        -> T
{
        return value;
}

// A set's keys, lowest position first, a block at a time; arms for one or two blocks compile to this same loop.
template<class Key, class KeyMapping, class Bits, class F>
constexpr auto walk_keys_up(Bits const& c, F& f)
        -> void
{
        constexpr auto digits = Bits::bits_per_block;
        auto key              = [&](std::size_t n) -> void { f(decay_copy<Key>(KeyMapping::from_index(n))); };
        for (auto const index : std::views::iota(0UZ, c.num_blocks())) {
                walk_block_up(c[index], digits * index, key);
        }
}

// The mirror, highest position first.
template<class Key, class KeyMapping, class Bits, class F>
constexpr auto walk_keys_down(Bits const& c, F& f)
        -> void
{
        constexpr auto digits = Bits::bits_per_block;
        auto key              = [&](std::size_t n) -> void { f(decay_copy<Key>(KeyMapping::from_index(n))); };
        auto const blocks     = c.num_blocks();
        for (auto index = blocks - 1UZ; index < blocks; --index) {
                walk_block_down(c[index], digits * index, key);
        }
}

// A sequence's len bools from position n, a block at a time: the reload is the inner loop's exit test.
template<class Bits, class F>
constexpr auto walk_bools_up(Bits const& c, std::size_t n, std::size_t len, F& f)
        -> void
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;
        for (auto k = 0UZ; k < len; k += digits) {
                auto const block = c.block_at(n + k);
                for (auto const i : std::views::iota(0UZ, std::ranges::min(digits, len - k))) {
                        f(decay_copy((shr(block, i) & block_type{1}) != block_type{}));
                }
        }
}

// The mirror, the last bool first, its block's bits from the top.
template<class Bits, class F>
constexpr auto walk_bools_down(Bits const& c, std::size_t n, std::size_t len, F& f)
        -> void
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;
        auto const chunks     = (len + digits - 1UZ) / digits;
        for (auto j = chunks - 1UZ; j < chunks; --j) {
                auto const block = c.block_at(n + (j * digits));
                auto const count = std::ranges::min(digits, len - (j * digits));
                for (auto i = count - 1UZ; i < count; --i) {
                        f(decay_copy((shr(block, i) & block_type{1}) != block_type{}));
                }
        }
}

// Every element of r handed to f by value, in the order r iterates or, Reverse, against it.
template<bool Reverse, class R, class F>
constexpr auto walk(R const& r, F& f)
        -> void
{
        auto const& c = storage_access::bits(r);
        if constexpr (reads_keys<R>) {
                using key_type     = std::remove_cvref_t<R>::key_type;
                using mapping_type = std::remove_cvref_t<R>::key_mapping_type;
                if constexpr (Reverse != descends<R>) {
                        walk_keys_down<key_type, mapping_type>(c, f);
                } else {
                        walk_keys_up<key_type, mapping_type>(c, f);
                }
        } else if constexpr (Reverse) {
                walk_bools_down(c, first_position(r), std::ranges::size(r), f);
        } else {
                walk_bools_up(c, first_position(r), std::ranges::size(r), f);
        }
}

// What std::views::reverse was applied to, where that is a view of ours: the view itself.
template<class V>
struct reversed_base
{
        using type = V;

        [[nodiscard]] static constexpr auto get(std::ranges::reverse_view<V> const& r) noexcept
                -> V
        {
                return r.base();
        }
};

// Or a container of ours, which std::views::reverse holds through a ref_view.
template<class S>
struct reversed_base<std::ranges::ref_view<S>>
{
        using type = S&;

        [[nodiscard]] static constexpr auto get(std::ranges::reverse_view<std::ranges::ref_view<S>> const& r) noexcept
                -> S&
        {
                return r.base().base();
        }
};

template<class R>
using reversed_base_t = reversed_base<decltype(std::declval<std::remove_cvref_t<R> const&>().base())>;

// A std::views::reverse over one of ours, held by reference or as a view, which a walk takes against its order.
template<class R>
concept reversed_reading = xstd::specialization_of<std::remove_cvref_t<R>, std::ranges::reverse_view> and (reads_keys<typename reversed_base_t<R>::type> or reads_bools<typename reversed_base_t<R>::type>);

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_WALK_HPP
