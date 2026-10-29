# `<xstd/bits/ext/boost/bit_small_set.hpp>`

## `basic_bit_small_set`, `bit_small_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
         see below Compare = less<Key>, class Alloc = boost::container::new_allocator<Block>>
  requires see below
using basic_bit_small_set = basic_bit_set<Key, boost::container::small_vector<Block, see below, Alloc>,
                                          dynamic_extent, KeyMapping, Compare>;

template<size_t N>
using bit_small_set = basic_bit_small_set<size_t, size_t, N>;
```

A [`basic_bit_set`](../bit_set/bit_set.md) over the small storage, with:

- `bit_small_set<N>` holds its keys in `size_t` blocks, the `ceil(N / D)` blocks that hold `N` bits inline.
- `allocator_type` and the allocator arguments are as for `basic_bit_small_vector`; the constructor from an
  allocator is `noexcept`, and the `from_blocks` constructor `noexcept` where the block container's move is.
- A key past the inline blocks moves them to the heap, keeping the keys and their count.
- The header also brings `std::hash` and Boost's `is_range` and `is_tuple_like` for it, which `basic_bit_set`'s
  headers declare.
