//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_FIND_IF_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_FIND_IF_HPP

#include <xstd/bits/bit_algorithm/bit_find.hpp>       // bit_find
#include <xstd/bits/bit_algorithm/detail/find_if.hpp> // find_key_if
#include <xstd/bits/detail/ownership.hpp>             // reads_bools, reads_keys
#include <concepts>                                   // predicate
#include <functional>                                 // invoke
#include <ranges>                                     // begin, borrowed_iterator_t, end, range_value_t, size

namespace xstd::ranges {

// std::ranges::find_if(r, pred), a block at a time: the first key or bool, in the order r iterates, that pred accepts.
template<class R, class Pred>
        requires (bits::detail::reads_keys<R> or bits::detail::reads_bools<R>) and std::predicate<Pred&, std::ranges::range_value_t<R>>
[[nodiscard]] constexpr auto bit_find_if(R&& r, Pred pred)
        -> std::ranges::borrowed_iterator_t<R>
{
        using result_type = std::ranges::borrowed_iterator_t<R>;
        if constexpr (bits::detail::reads_keys<R>) {
                auto const key = bits::detail::find_key_if(r, pred);
                return key ? result_type(r.find(*key)) : result_type(std::ranges::end(r));
        } else {
                // An equality-preserving pred answers each bool alike, so two calls decide what one per element would.
                auto const n = std::ranges::size(r);
                if (n < 2UZ) {
                        return n == 1UZ and std::invoke(pred, static_cast<bool>(*std::ranges::begin(r))) ? result_type(std::ranges::begin(r)) : result_type(std::ranges::end(r));
                }
                auto const on_false = static_cast<bool>(std::invoke(pred, false));
                auto const on_true  = static_cast<bool>(std::invoke(pred, true));
                if (on_false == on_true) {
                        return on_true ? result_type(std::ranges::begin(r)) : result_type(std::ranges::end(r));
                }
                return result_type(xstd::ranges::bit_find(r, on_true));
        }
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_FIND_IF_HPP
