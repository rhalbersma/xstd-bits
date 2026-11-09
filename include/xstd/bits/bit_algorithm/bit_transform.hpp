//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_TRANSFORM_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_TRANSFORM_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // binary_table, bool_function, read_block, unary_table, write_up
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator, bool_output_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position
#include <algorithm>                                          // binary_transform_result, min, unary_transform_result
#include <cstddef>                                            // ptrdiff_t, size_t
#include <ranges>                                             // begin, borrowed_iterator_t, end, size
#include <type_traits>                                        // remove_cvref_t
#include <utility>                                            // move

namespace xstd::ranges {

// std::ranges::transform(r, out, f), f asked of false and of true once each and applied a block at a time.
template<bits::detail::reads_bools R, bits::detail::bool_output_iterator O, bits::detail::bool_function<bool> F>
constexpr auto bit_transform(R&& r, O out, F f)
        -> std::ranges::unary_transform_result<std::ranges::borrowed_iterator_t<R>, O>
{
        using bits::detail::storage_access;
        auto&& dst       = storage_access::iterated_bits(out);
        using block_type = std::remove_cvref_t<decltype(dst)>::block_type;
        auto const op    = bits::detail::unary_table<block_type>(f);
        auto const& src  = storage_access::bits(r);
        auto const s     = bits::detail::first_position(r);
        auto const n     = std::ranges::size(r);
        bits::detail::write_up(dst, storage_access::position(out), n, [&](std::size_t k, std::size_t count) -> block_type { return op(bits::detail::read_block<block_type>(src, s + k, count)); });
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), out + static_cast<std::ptrdiff_t>(n)};
}

// std::ranges::transform(r1, r2, out, f), f asked of the four pairs once each, as far as the shorter goes.
template<bits::detail::reads_bools R1, bits::detail::reads_bools R2, bits::detail::bool_output_iterator O, bits::detail::bool_function<bool, bool> F>
constexpr auto bit_transform(R1&& r1, R2&& r2, O out, F f)
        -> std::ranges::binary_transform_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>, O>
{
        using bits::detail::storage_access;
        auto&& dst       = storage_access::iterated_bits(out);
        using block_type = std::remove_cvref_t<decltype(dst)>::block_type;
        auto const op    = bits::detail::binary_table<block_type>(f);
        auto const& x    = storage_access::bits(r1);
        auto const& y    = storage_access::bits(r2);
        auto const xs    = bits::detail::first_position(r1);
        auto const ys    = bits::detail::first_position(r2);
        auto const n     = std::ranges::min(std::ranges::size(r1), std::ranges::size(r2));
        bits::detail::write_up(dst, storage_access::position(out), n, [&](std::size_t k, std::size_t count) -> block_type { return op(bits::detail::read_block<block_type>(x, xs + k, count), bits::detail::read_block<block_type>(y, ys + k, count)); });
        auto const d = static_cast<std::ptrdiff_t>(n);
        return {std::ranges::borrowed_iterator_t<R1>(std::ranges::begin(r1) + d), std::ranges::borrowed_iterator_t<R2>(std::ranges::begin(r2) + d), out + d};
}

// std::ranges::transform(first, last, out, f), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S, bits::detail::bool_output_iterator O, bits::detail::bool_function<bool> F>
constexpr auto bit_transform(I first, S last, O out, F f)
        -> std::ranges::unary_transform_result<I, O>
{
        auto w               = bits::detail::iterator_window(first, last);
        auto const [in, end] = xstd::ranges::bit_transform(w, out, std::move(f));
        return {bits::detail::rebased(first, in), end};
}

// std::ranges::transform(first1, last1, first2, last2, out, f), over both pairs' positions as windows.
template<bits::detail::bool_iterator I1, bits::detail::bool_sentinel_for<I1> S1, bits::detail::bool_iterator I2, bits::detail::bool_sentinel_for<I2> S2, bits::detail::bool_output_iterator O, bits::detail::bool_function<bool, bool> F>
constexpr auto bit_transform(I1 first1, S1 last1, I2 first2, S2 last2, O out, F f)
        -> std::ranges::binary_transform_result<I1, I2, O>
{
        auto w1                    = bits::detail::iterator_window(first1, last1);
        auto w2                    = bits::detail::iterator_window(first2, last2);
        auto const [in1, in2, end] = xstd::ranges::bit_transform(w1, w2, out, std::move(f));
        return {bits::detail::rebased(first1, in1), bits::detail::rebased(first2, in2), end};
}

} // namespace xstd::ranges

namespace xstd {

// std::transform(first, last, out, f): the end of what was written.
template<bits::detail::bool_iterator I, bits::detail::bool_output_iterator O, bits::detail::bool_function<bool> F>
constexpr auto bit_transform(I first, I last, O out, F f)
        -> O
{
        return xstd::ranges::bit_transform(first, last, out, std::move(f)).out;
}

// std::transform(first1, last1, first2, out, f): the end of what was written.
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2, bits::detail::bool_output_iterator O, bits::detail::bool_function<bool, bool> F>
constexpr auto bit_transform(I1 first1, I1 last1, I2 first2, O out, F f)
        -> O
{
        return xstd::ranges::bit_transform(first1, last1, first2, first2 + (last1 - first1), out, std::move(f)).out;
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_TRANSFORM_HPP
