//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_ITERATOR_WINDOW_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_ITERATOR_WINDOW_HPP

#include <xstd/bits/detail/iterator_concepts.hpp> // bool_iterator
#include <xstd/bits/detail/ownership.hpp>         // storage_access
#include <cassert>                                // assert
#include <iterator>                               // iter_reference_t, sentinel_for
#include <type_traits>                            // remove_reference_t
#include <utility>                                // declval

namespace xstd::bits::detail {

// The end of a range of our sequence iterators: one of them itself, so the positions it spans are known.
template<class S, class I>
concept bool_sentinel_for = bool_iterator<S> and std::sentinel_for<S, I>;

// The window over the storage I steps over, through Bits const where I is a const_iterator, named by I's own adaptor.
template<bool_iterator I>
using iterator_window_t = std::iter_reference_t<I>::adaptor_type::template window_over_t<std::remove_reference_t<decltype(storage_access::iterated_bits(std::declval<I const&>()))>>;

// The positions [first, last) as a window, which every range form takes as it takes a subspan.
template<bool_iterator I, bool_sentinel_for<I> S>
[[nodiscard]] constexpr auto iterator_window(I const& first, S const& last) noexcept
        -> iterator_window_t<I>
{
        decltype(auto) bits = storage_access::iterated_bits(first);
        auto const offset   = storage_access::position(first);
        assert(offset <= storage_access::position(last));
        return storage_access::window<iterator_window_t<I>>(&bits, offset, storage_access::position(last) - offset);
}

// An iterator of type I at the position where it, an iterator into the same storage, stands.
template<bool_iterator I, bool_iterator J>
[[nodiscard]] constexpr auto rebased(I i, J const& j) noexcept
        -> I
{
        storage_access::reposition(i, storage_access::position(j));
        return i;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_ITERATOR_WINDOW_HPP
