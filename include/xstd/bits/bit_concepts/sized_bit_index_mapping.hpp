//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_SIZED_BIT_INDEX_MAPPING_HPP
#define XSTD_BITS_BIT_CONCEPTS_SIZED_BIT_INDEX_MAPPING_HPP

#include <xstd/bits/bit_concepts/bit_index_mapping.hpp> // bit_index_mapping
#include <concepts>                                     // convertible_to, same_as
#include <cstddef>                                      // size_t

namespace xstd {

// A mapping that closes its universe: exactly size keys, at positions 0 through size - 1, and is_key says which.
template<class Mapping, class Key>
concept sized_bit_index_mapping = bit_index_mapping<Mapping, Key> and requires (Key key) {
        { Mapping::size } -> std::convertible_to<std::size_t>;
        { Mapping::is_key(key) } -> std::same_as<bool>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_SIZED_BIT_INDEX_MAPPING_HPP
