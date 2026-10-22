# `<xstd/bits/ext/boost/bit_small_valarray.hpp>`

## `basic_bit_small_valarray`, `bit_small_valarray`

```cpp
template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
  requires see below
using basic_bit_small_valarray = basic_bit_valarray<boost::container::small_vector<Block, see below, Alloc>>;

template<size_t N>
using bit_small_valarray = basic_bit_small_valarray<size_t, N>;
```

A [`basic_bit_valarray`](../bit_valarray/bit_valarray.md) over the small storage, with:

- `bit_small_valarray<N>` holds its bools in `size_t` blocks, the `ceil(N / D)` blocks that hold `N` bits inline, and
  `Alloc` defaults to Boost's own. Past the inline blocks they move to the heap, and the elements stay what they were.
- A `const` subscript gathers into a valarray of the same type.
