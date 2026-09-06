//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_XSTD_BITSET_HPP
#define XSTD_BITS_EXT_XSTD_BITSET_HPP

// IWYU pragma: always_keep

#include <xstd/bits/basic_bit_set.hpp>             // basic_bit_set
#include <xstd/bits/basic_bitset.hpp>              // IWYU pragma: export; basic_bitset
#include <xstd/bits/bitset.hpp>                    // IWYU pragma: export; bitset
#include <iterator>                                // make_reverse_iterator
#include <ranges>                                  // begin, end, rbegin, rend

// The set reading's iterators over a bitset, found by ADL: a set view over it is borrowed, so the iterators outlive the temporary that made them.
namespace xstd {

template<class Bits, class Traits> [[nodiscard]] constexpr auto begin  (      basic_bitset<Bits, Traits>& c) noexcept { return basic_bit_set(c).begin(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto begin  (const basic_bitset<Bits, Traits>& c) noexcept { return basic_bit_set(c).begin(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto end    (      basic_bitset<Bits, Traits>& c) noexcept { return basic_bit_set(c).end(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto end    (const basic_bitset<Bits, Traits>& c) noexcept { return basic_bit_set(c).end(); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto cbegin (const basic_bitset<Bits, Traits>& c) noexcept { return std::ranges::begin(c); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto cend   (const basic_bitset<Bits, Traits>& c) noexcept { return std::ranges::end  (c); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rbegin (      basic_bitset<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::end(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rbegin (const basic_bitset<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::end(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rend   (      basic_bitset<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::begin(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto rend   (const basic_bitset<Bits, Traits>& c) noexcept { return std::make_reverse_iterator(std::ranges::begin(c)); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto crbegin(const basic_bitset<Bits, Traits>& c) noexcept { return std::ranges::rbegin(c); }
template<class Bits, class Traits> [[nodiscard]] constexpr auto crend  (const basic_bitset<Bits, Traits>& c) noexcept { return std::ranges::rend  (c); }

}       // namespace xstd

#endif // XSTD_BITS_EXT_XSTD_BITSET_HPP
