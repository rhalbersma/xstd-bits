# `<xstd/bits/ext/boost/bit_small_valarray.hpp>`

## `basic_bit_small_valarray`, `bit_small_valarray`

As `basic_bit_valarray`, over the small storage, with:

- `bit_small_valarray<N>` holds its bools in `size_t` blocks, inline up to `N` of them, and `Alloc` defaults to
  Boost's own. Past the inline capacity the blocks move to the heap, and the elements stay what they were.
- A `const` subscript gathers into a `basic_bit_small_valarray` of the same parameters.
