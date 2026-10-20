# `<xstd/bits/ext/boost/bit_small_set.hpp>`

## `basic_bit_small_set`, `bit_small_set`

As `basic_bit_set`, over the small storage, with:

- `bit_small_set<N>` holds its keys in `size_t` blocks, inline below `N`. *Mandates:* as `basic_bit_fixed_set`'s.
- `allocator_type` and the allocator arguments are as for `basic_bit_small_vector`; the constructor from an
  allocator is `noexcept`, and the `from_blocks` constructor `noexcept` where the block container's move is.
- A key past the inline capacity moves the blocks to the heap, keeping the keys and their count. There are no
  deduction guides.
