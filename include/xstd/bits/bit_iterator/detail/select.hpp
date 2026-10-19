//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_DETAIL_SELECT_HPP
#define XSTD_BITS_BIT_ITERATOR_DETAIL_SELECT_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // descends
#include <xstd/bits/detail/intrinsic.hpp>               // popcount
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <xstd/bits/detail/shift.hpp>                   // partial_block_mask
#include <xstd/ints/bit/bit_select.hpp>                 // bit_select
#include <cassert>                                      // assert
#include <cstddef>                                      // ptrdiff_t, size_t
#include <iterator>                                     // iter_reference_t
#include <ranges>                                       // iota

namespace xstd::bits::detail {

// Where a search for the n-th set position stopped: there, or, missing that many short, at the range's end.
struct select_result
{
        std::size_t position;
        std::size_t missing;
};

// The block [first, last) touches at index, with what lies outside it cleared.
template<class Bits>
[[nodiscard]] constexpr auto masked_block(Bits const& c, std::size_t index, std::size_t first, std::size_t last) noexcept
        -> Bits::block_type
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;
        auto block            = c[index];
        if (index == first / digits) {
                block = static_cast<block_type>(block & static_cast<block_type>(~partial_block_mask<block_type>(first % digits)));
        }
        if (index == (last - 1UZ) / digits) {
                block = static_cast<block_type>(block & partial_block_mask<block_type>(((last - 1UZ) % digits) + 1UZ));
        }
        return block;
}

// The n-th set position of [first, last), counting up from one: whole blocks skipped by popcount, the last selected in.
template<class Bits>
[[nodiscard]] constexpr auto select_up(Bits const& c, std::size_t first, std::size_t last, std::size_t n) noexcept
        -> select_result
{
        assert(0UZ < n and first <= last and last <= c.size());
        if (first == last) {
                return {.position = last, .missing = n};
        }
        constexpr auto digits = Bits::bits_per_block;
        for (auto const index : std::views::iota(first / digits, ((last - 1UZ) / digits) + 1UZ)) {
                auto const block = masked_block(c, index, first, last);
                auto const count = popcount(block);
                if (n <= count) {
                        return {.position = (index * digits) + static_cast<std::size_t>(xstd::bit_select(block, static_cast<int>(n - 1UZ))), .missing = 0UZ};
                }
                n -= count;
        }
        return {.position = last, .missing = n};
}

// The n-th set position of [first, last), counting down from one, a block's own selected from its top.
template<class Bits>
[[nodiscard]] constexpr auto select_down(Bits const& c, std::size_t first, std::size_t last, std::size_t n) noexcept
        -> select_result
{
        assert(0UZ < n and first <= last and last <= c.size());
        if (first == last) {
                return {.position = first, .missing = n};
        }
        constexpr auto digits = Bits::bits_per_block;
        auto const lo         = first / digits;
        for (auto index = (last - 1UZ) / digits, N = index + 1UZ - lo; index - lo < N; --index) {
                auto const block = masked_block(c, index, first, last);
                auto const count = popcount(block);
                if (n <= count) {
                        return {.position = (index * digits) + static_cast<std::size_t>(xstd::bit_select(block, static_cast<int>(count - n))), .missing = 0UZ};
                }
                n -= count;
        }
        return {.position = first, .missing = n};
}

// it moved n keys, but not past the key or end at bound, and the steps it fell short by, signed as n is.
template<class I>
[[nodiscard]] constexpr auto advance_keys(I& it, std::ptrdiff_t n, std::size_t bound) noexcept
        -> std::ptrdiff_t
{
        auto&& c     = storage_access::iterated_bits(it);
        auto const p = storage_access::position(it);
        if (n == 0 or p == bound) {
                return n;
        }
        auto const size = c.size();
        auto const m    = static_cast<std::size_t>(n < 0 ? -n : n);
        // Up the positions is forward in an ascending set and backward in a descending one, whose end sits below 0.
        auto const result = ((0 < n) != descends<typename std::iter_reference_t<I>::adaptor_type>)
                                    ? select_up(c, p == size ? 0UZ : p + 1UZ, bound, m)
                                    : select_down(c, bound == size ? 0UZ : bound + 1UZ, p, m);
        if (result.missing == 0UZ) {
                storage_access::reposition(it, result.position);
                return 0;
        }
        storage_access::reposition(it, bound);
        auto const rest = static_cast<std::ptrdiff_t>(result.missing - 1UZ);
        return n < 0 ? -rest : rest;
}

// Where begin() of it's set stands: the lowest key ascending, the highest descending, and the end where it is empty.
template<class I>
[[nodiscard]] constexpr auto first_key_position(I const& it) noexcept
        -> std::size_t
{
        auto&& c = storage_access::iterated_bits(it);
        if constexpr (descends<typename std::iter_reference_t<I>::adaptor_type>) {
                return c.total_find_prev(c.size());
        } else {
                return c.find_first();
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ITERATOR_DETAIL_SELECT_HPP
