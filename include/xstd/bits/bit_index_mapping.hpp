//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INDEX_MAPPING_HPP
#define XSTD_BITS_BIT_INDEX_MAPPING_HPP

#include <concepts> // convertible_to, same_as
#include <cstddef>  // size_t

// What a set owner asks of the mapping that places its keys in bits.
namespace xstd {

// to_index preserves order, a < b exactly where to_index(a) < to_index(b), and from_index inverts it on the universe.
template<class Mapping, class Key>
concept bit_index_mapping = requires (Key key, std::size_t index) {
        { Mapping::to_index(key) } -> std::same_as<std::size_t>;
        { Mapping::from_index(index) } -> std::same_as<Key>;
};

// A mapping that closes its universe: exactly size keys, at positions 0 through size - 1.
template<class Mapping, class Key>
concept sized_bit_index_mapping = bit_index_mapping<Mapping, Key> and requires {
        { Mapping::size } -> std::convertible_to<std::size_t>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_INDEX_MAPPING_HPP
