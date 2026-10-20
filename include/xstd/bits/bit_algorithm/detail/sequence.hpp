//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_SEQUENCE_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_SEQUENCE_HPP

#include <xstd/bits/detail/ownership.hpp> // reads_bools, storage_access
#include <cstddef>                        // size_t
#include <type_traits>                    // remove_cvref_t
#include <utility>                        // declval

namespace xstd::bits::detail {

// A sequence reading all of its storage: a subspan's window shares its end blocks with positions outside it.
template<class R>
concept whole_sequence = reads_bools<R> and not std::remove_cvref_t<R>::is_windowed;

// Where position zero of a sequence sits in its storage: a window's offset, and zero for everything else.
template<reads_bools R>
[[nodiscard]] constexpr auto first_position(R const& r [[maybe_unused]]) noexcept
        -> std::size_t
{
        if constexpr (whole_sequence<R>) {
                return 0UZ;
        } else {
                return storage_access::offset(r);
        }
}

// The storage under one of our sequences, as the algorithms that write it reach it.
template<class R>
using bits_t = std::remove_cvref_t<decltype(storage_access::bits(std::declval<R&>()))>;

// A sequence whose bools the blit can write: its storage reached as writable, a window's or a whole sequence's.
template<class R>
concept writable_bools = reads_bools<R> and requires (R& r, bits_t<R>::block_type block) { storage_access::bits(r).block_at(0UZ, block, block); };

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_SEQUENCE_HPP
