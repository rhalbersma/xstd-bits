# `<xstd/bits/ext/boost/bit_small_vector.hpp>`

## `basic_bit_small_vector`, `bit_small_vector`

```cpp
template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
  requires see below
using basic_bit_small_vector = basic_bit_vector<boost::container::small_vector<Block, see below, Alloc>>;

template<size_t N>
using bit_small_vector = basic_bit_small_vector<size_t, N>;
```

A [`basic_bit_vector`](../bit_vector/bit_vector.md) over `boost::container::small_vector`, with:

- `bit_small_vector<N>` holds its bools in `size_t` blocks, the `ceil(N / D)` blocks that hold `N` bits inline and the
  rest on the heap. The requires-clause admits `Block` as `basic_bit_vector`'s does, and `Alloc` defaults to Boost's
  own, as `small_vector`'s does.
- The inline part is whole blocks, so `bit_small_vector<N>` and `bit_small_vector<M>` are one type wherever
  `ceil(N / D) == ceil(M / D)`, and `bit_rebind` and `bit_resize` keep the bits the inline blocks hold.
- `allocator_type` is the small vector's, which wraps `Alloc`, and every allocator argument is of that type.
  Constructing from an allocator does not throw, inline blocks allocating nothing.
- `capacity()`, `reserve` and `shrink_to_fit` are the small vector's.
- The header also brings `std::hash` and Boost's `is_range` and `is_tuple_like` for it, which `basic_bit_vector`'s
  headers declare. The Hash2 hook's model is `vector<bool>`.
