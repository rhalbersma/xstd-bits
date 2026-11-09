//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_FOR_EACH_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_FOR_EACH_HPP

#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/bit_algorithm/detail/walk.hpp>            // reversed_base_t, reversed_reading, walk
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, reads_keys
#include <algorithm>                                          // for_each_result
#include <concepts>                                           // invocable
#include <ranges>                                             // borrowed_iterator_t, end, range_value_t
#include <utility>                                            // move

namespace xstd::ranges {

// std::ranges::for_each(r, f), a block at a time: f takes each key or bool by value, in the order r iterates.
template<class R, class F>
        requires (bits::detail::reads_keys<R> or bits::detail::reads_bools<R>) and std::invocable<F&, std::ranges::range_value_t<R>>
constexpr auto bit_for_each(R&& r, F f)
        -> std::ranges::for_each_result<std::ranges::borrowed_iterator_t<R>, F>
{
        bits::detail::walk<false>(r, f);
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), std::move(f)};
}

// std::ranges::for_each(std::views::reverse(r), f), the same walk against the order r iterates.
template<class R, class F>
        requires bits::detail::reversed_reading<R> and std::invocable<F&, std::ranges::range_value_t<R>>
constexpr auto bit_for_each(R&& r, F f)
        -> std::ranges::for_each_result<std::ranges::borrowed_iterator_t<R>, F>
{
        bits::detail::walk<true>(bits::detail::reversed_base_t<R>::get(r), f);
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), std::move(f)};
}

// std::ranges::for_each(first, last, f), over the positions [first, last) as a window: f takes each bool by value.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S, class F>
        requires std::invocable<F&, bool>
constexpr auto bit_for_each(I first, S last, F f)
        -> std::ranges::for_each_result<I, F>
{
        auto w         = bits::detail::iterator_window(first, last);
        auto [in, fun] = xstd::ranges::bit_for_each(w, std::move(f));
        return {bits::detail::rebased(first, in), std::move(fun)};
}

} // namespace xstd::ranges

namespace xstd {

// std::for_each(first, last, f): f, as it stands after the walk.
template<bits::detail::bool_iterator I, class F>
        requires std::invocable<F&, bool>
constexpr auto bit_for_each(I first, I last, F f)
        -> F
{
        return xstd::ranges::bit_for_each(first, last, std::move(f)).fun;
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_FOR_EACH_HPP
