# `<xstd/bits/bit_valarray/bit_inplace_valarray.hpp>`

## `basic_bit_inplace_valarray`, `bit_inplace_valarray`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
using basic_bit_inplace_valarray = basic_bit_valarray<see below, N>;

template<size_t N>
using bit_inplace_valarray = basic_bit_inplace_valarray<size_t, N>;
```

A [`basic_bit_valarray`](bit_valarray.md) at a size within a capacity of `N`, with:

- `bit_inplace_valarray<N>` holds up to `N` bools in `size_t` blocks, inline. The requires-clause admits `Block` as
  `basic_bit_inplace_vector`'s does, and the storage is that alias's.
- A size past `N` throws `bad_alloc`, and a refused `resize` leaves the array as it was.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined.
