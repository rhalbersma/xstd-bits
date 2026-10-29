# `<xstd/bits/ext/boost.hpp>`

```cpp
namespace xstd {
  // the sequence reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    using basic_bit_small_vector = basic_bit_vector<boost::container::small_vector<Block, see below, Alloc>>;
  template<size_t N>
    using bit_small_vector = basic_bit_small_vector<size_t, N>;

  // the valarray reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    using basic_bit_small_valarray = basic_bit_valarray<boost::container::small_vector<Block, see below, Alloc>>;
  template<size_t N>
    using bit_small_valarray = basic_bit_small_valarray<size_t, N>;

  // the set reading
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    using basic_bit_small_set =
      basic_bit_set<Key, boost::container::small_vector<Block, see below, Alloc>, dynamic_extent, KeyMapping, Compare>;
  template<size_t N>
    using bit_small_set = basic_bit_small_set<size_t, size_t, N>;

  // the storage: Boost's small_vector rebound
  template<class Block, size_t K, class Allocator>
    struct bit_storage_rebind<boost::container::small_vector<Block, K, Allocator>>;
}

// hash, is_range and is_tuple_like: basic_bit_vector's and basic_bit_set's, exported again
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/ext/boost/bit_small_vector.hpp>`](bit_small_vector.md) | `basic_bit_small_vector`, `bit_small_vector` |
| [`<xstd/bits/bit_vector/erase_if.hpp>`](../bit_vector/erase_if.md) | `erase`, `erase_if` |
| [`<xstd/bits/ext/boost/bit_small_valarray.hpp>`](bit_small_valarray.md) | `basic_bit_small_valarray`, `bit_small_valarray` |
| [`<xstd/bits/ext/boost/bit_small_set.hpp>`](bit_small_set.md) | `basic_bit_small_set`, `bit_small_set` |
| [`<xstd/bits/bit_set/erase_if.hpp>`](../bit_set/erase_if.md) | `erase_if` |

The three readings over the small storage, `boost::container::small_vector<Block, K, Alloc>` for `K = ceil(N / D)`:
the `K` blocks that hold `N` bits inline, and the heap past them. `<xstd/bits.hpp>` does not include this header, so the dependency
on Boost.Container's `small_vector` is opted into by name. It also exports the sequence reading's `erase` and
`erase_if` and the set reading's `erase_if`. `<xstd/bits/ext/boost/dynamic_bitset.hpp>`, which converts to and from
`boost::dynamic_bitset`, is outside this part.

Each owner throws only the standard exceptions, as its standard counterpart does, because it refuses a width before
Boost.Container is asked for it. Boost.Container's own exceptions, which derive from `std::exception` but not from
`std::bad_alloc` or `std::length_error`, reach only code that calls a `small_vector` or `static_vector` directly.
Defining `BOOST_CONTAINER_USE_STD_EXCEPTIONS` makes them the standard ones. It changes inline functions in Boost's
headers, so it is defined for the whole program, on the command line or in the build, never in one source file. The
library neither defines it nor needs it, and its tests build without it, so that a Boost exception escaping an owner
fails them.
