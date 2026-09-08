//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_XSTD_BITSET_HPP
#define XSTD_BITS_EXT_XSTD_BITSET_HPP

// IWYU pragma: always_keep

#include <xstd/bits/set_adaptor.hpp>    // set_adaptor
#include <xstd/bits/bitset_adaptor.hpp> // IWYU pragma: export; bitset_adaptor
#include <xstd/bits/bitset.hpp>         // IWYU pragma: export; bitset
#include <iterator>                     // make_reverse_iterator
#include <ranges>                       // begin, end, rbegin, rend

// The set reading's iterators over a bitset, found by ADL: a set view over it is borrowed, so the iterators outlive
// the temporary that made them.
//
// Non-member on purpose, and not a leftover of the member-iterator refactor that gave every container a
// begin(this auto&& self). A bitset_adaptor must not become a range: xstd::bitset is a strict extension of
// std::bitset, and a range is the one addition a strict extension cannot make, because it changes what generic
// code already written against std::bitset does with it -- fmt would format it as a sequence, std::ranges::to
// would consume it. So iteration over the bitset reading is reachable by name and only by name, which is what
// a free function found by ADL is and what a member could never be. The two sibling headers adapt foreign
// bitsets and need none of this: they are bit_traits specializations, and the views carry the readings.
// [design.md#a-strict-extension]
namespace xstd {

template<class Bits, class Traits> [[nodiscard]] constexpr auto begin  (      bitset_adaptor<Bits, Traits>& c) noexcept { return set_adaptor(c).begin(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto begin  (const bitset_adaptor<Bits, Traits>& c) noexcept { return set_adaptor(c).begin(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto end    (      bitset_adaptor<Bits, Traits>& c) noexcept { return set_adaptor(c).end(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto end    (const bitset_adaptor<Bits, Traits>& c) noexcept { return set_adaptor(c).end(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto cbegin (const bitset_adaptor<Bits, Traits>& c) noexcept { return std::ranges::begin(c); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto cend   (const bitset_adaptor<Bits, Traits>& c) noexcept { return std::ranges::end  (c); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rbegin (      bitset_adaptor<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::end(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rbegin (const bitset_adaptor<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::end(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rend   (      bitset_adaptor<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::begin(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rend   (const bitset_adaptor<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::begin(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto crbegin(const bitset_adaptor<Bits, Traits>& c) noexcept { return std::ranges::rbegin(c); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto crend  (const bitset_adaptor<Bits, Traits>& c) noexcept { return std::ranges::rend  (c); }

}       // namespace xstd

#endif // XSTD_BITS_EXT_XSTD_BITSET_HPP
