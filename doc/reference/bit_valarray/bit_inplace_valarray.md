# `<xstd/bits/bit_valarray/bit_inplace_valarray.hpp>`

## `basic_bit_inplace_valarray`, `bit_inplace_valarray`

As `basic_bit_valarray`, at a size within a capacity of `N`, with:

- `bit_inplace_valarray<N>` holds up to `N` bools in `size_t` blocks, inline. The requires-clause admits `Block` as
  `basic_bit_inplace_vector`'s does, and `block_container_type` is that class's.
- A size past `N` throws `bad_alloc`, and a refused `resize` leaves the array as it was.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined.
