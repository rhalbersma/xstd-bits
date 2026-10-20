# `<xstd/bits/bit_ranges/from_blocks.hpp>`

## `from_blocks_t`, `from_blocks`

As `from_range_t` and `from_range` in [[ranges.syn]](https://eel.is/c++draft/ranges.syn), with:

- The tag says at the call site that an argument's blocks are read as bits, rather than as keys or elements:
  `bit_fixed_set<32>(from_blocks, 5u)` is `{0, 2}`, the positions the value five has.
- Its default constructor is `explicit`, so `{}` cannot stand in for the tag.
- Every owner of ours takes it: a fixed width reads any blocks that hold at least its width, and a run-time width
  adopts its own block container. `bit_constructible_from` names the blocks an owner takes as they are.
- The deduction guides of `basic_bit_array` and `basic_bit_fixed_set` deduce the block and the width
  `bit_blocks_extent_v` names, from an unsigned integer, an `array` of them or a built-in array of them, as
  [[span.deduct]](https://eel.is/c++draft/span.deduct) takes an array's bound: `basic_bit_array(from_blocks,
  uint16_t{...})` is `basic_bit_array<uint16_t, 16>`, and `basic_bit_fixed_set(from_blocks, array<uint8_t, 3>{...})`
  is `basic_bit_fixed_set<size_t, uint8_t, 24>`. An empty `array` deduces width zero.
- A signed integer or an `array` of them deduces nothing, and a run-time width takes no block of a fixed width:
  `bit_vector` is not constructible from `from_blocks` and a `uint64_t`.
- A type that has bit storage without being it, a `bitset<64>` or a `bit_array<64>`, is not read through the tag.
  It converts through `bit_convert`.
