//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_INDEX_MAPPING_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_INDEX_MAPPING_HPP

#include <concepts>    // same_as
#include <cstddef>     // size_t
#include <type_traits> // remove_cv_t

// What a set owner asks of the mapping that places its keys in bits.
namespace xstd {

// to_index preserves order, a < b exactly where to_index(a) < to_index(b), and from_index inverts it on the universe.
template<class Mapping, class Key>
concept bit_index_mapping = requires (std::remove_cv_t<Key> key, std::size_t index) {
        { Mapping::to_index(key) } -> std::same_as<std::size_t>;
        // Exactly Key: a mapping for a wider key takes a narrower one in, and hands back a value of the wrong type.
        { Mapping::from_index(index) } -> std::same_as<std::remove_cv_t<Key>>; // cv-transparent: asked of the bare key, as xstd-ints asks
};

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_INDEX_MAPPING_HPP
