# `<xstd/bits/ext/boost.hpp>`

```cpp
namespace xstd {
  // the sequence reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_vector;
  template<size_t N>
    using bit_small_vector = basic_bit_small_vector<size_t, N>;

  // the valarray reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_valarray;
  template<size_t N>
    using bit_small_valarray = basic_bit_small_valarray<size_t, N>;

  // the set reading
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_set;
  template<size_t N>
    using bit_small_set = basic_bit_small_set<size_t, size_t, N>;
}

namespace std {
  template<class Block, size_t N, class Alloc>
    struct hash<xstd::basic_bit_small_vector<Block, N, Alloc>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct hash<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>>;
}

namespace boost::container_hash {
  template<class Block, size_t N, class Alloc>
    struct is_range<xstd::basic_bit_small_vector<Block, N, Alloc>> : false_type {};
  template<class Block, size_t N, class Alloc>
    struct is_tuple_like<xstd::basic_bit_small_vector<Block, N, Alloc>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct is_range<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct is_tuple_like<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| `<xstd/bits/ext/boost/bit_small_vector.hpp>` | `basic_bit_small_vector`, `bit_small_vector` |
| `<xstd/bits/bit_vector/erase_if.hpp>` | `erase`, `erase_if` |
| `<xstd/bits/ext/boost/bit_small_valarray.hpp>` | `basic_bit_small_valarray`, `bit_small_valarray` |
| `<xstd/bits/ext/boost/bit_small_set.hpp>` | `basic_bit_small_set`, `bit_small_set` |
| `<xstd/bits/bit_set/erase_if.hpp>` | `erase_if` |

The three readings over the small storage, `boost::container::small_vector<Block, K, Alloc>` for `K = ceil(N / D)`:
inline up to `N` bits, and on the heap past them. `<xstd/bits.hpp>` does not include this header, so the dependency
on Boost.Container's `small_vector` is opted into by name. It also exports the sequence reading's `erase` and
`erase_if` and the set reading's `erase_if`. `<xstd/bits/ext/boost/dynamic_bitset.hpp>`, which converts to and from
`boost::dynamic_bitset`, is outside this part.
