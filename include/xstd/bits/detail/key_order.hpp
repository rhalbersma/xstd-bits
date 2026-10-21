//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_KEY_ORDER_HPP
#define XSTD_BITS_DETAIL_KEY_ORDER_HPP

#include <xstd/bits/detail/ownership.hpp> // reads_keys
#include <concepts>                       // same_as
#include <functional>                     // greater
#include <type_traits>                    // remove_cvref_t

namespace xstd::bits::detail {

// A set whose comparator is std::greater, so its iteration order is its positions from the top down.
template<class S>
concept descends = std::same_as<typename std::remove_cvref_t<S>::key_compare, std::greater<typename std::remove_cvref_t<S>::key_type>> or std::same_as<typename std::remove_cvref_t<S>::key_compare, std::greater<>>;

// Two sets whose keys mean the same positions in the same order, whatever blocks and storage hold them.
template<class S1, class S2>
concept same_keys =
        reads_keys<S1> and reads_keys<S2> and
        std::same_as<typename std::remove_cvref_t<S1>::key_type, typename std::remove_cvref_t<S2>::key_type> and
        std::same_as<typename std::remove_cvref_t<S1>::key_mapping_type, typename std::remove_cvref_t<S2>::key_mapping_type> and
        std::same_as<typename std::remove_cvref_t<S1>::key_compare, typename std::remove_cvref_t<S2>::key_compare>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_KEY_ORDER_HPP
