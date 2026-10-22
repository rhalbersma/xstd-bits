# `<xstd/bits/bit_ranges/from_blocks.hpp>`

## `xstd::from_blocks_t`, `xstd::from_blocks`

```cpp
struct from_blocks_t { explicit from_blocks_t() = default; };
inline constexpr from_blocks_t from_blocks{};
```

A tag that says at the call site that an argument's blocks are read as bits, rather than as keys or elements:
`bit_fixed_set<32>(from_blocks, 5u)` is `{0, 2}`, the positions the value five has. Its default constructor is
`explicit`, so `{}` cannot stand in for it.

### Notes

- Every owner of ours takes the tag: a fixed width reads any blocks that hold at least its width, and a run-time width
  adopts its own block container. [`bit_constructible_from`](../bit_concepts/bit_constructible_from.md) names the
  blocks an owner takes as they are.
- The deduction guides of `basic_bit_array` and `basic_bit_fixed_set` deduce the block and the width
  [`bit_blocks_extent_v`](../bit_type_traits/bit_blocks_extent.md) names, from an unsigned integer, an `array` of
  them or a built-in array of them, as [[span.deduct]](https://eel.is/c++draft/span.deduct) takes an array's bound:
    - `basic_bit_array(from_blocks, uint16_t{...})` is `basic_bit_array<uint16_t, 16>`;
    - `basic_bit_fixed_set(from_blocks, array<uint8_t, 3>{...})` is `basic_bit_fixed_set<size_t, uint8_t, 24>`;
    - an empty `array` deduces width zero.
- The deduction guides of `basic_bit_vector` and `basic_bit_set` deduce the storage from the container they adopt:
  `basic_bit_vector(from_blocks, vector<uint8_t>{...})` is `basic_bit_vector<vector<uint8_t>>`, and over a storage
  of constant capacity the width is that capacity in bits.
- A signed integer, or an `array` of them, deduces nothing, and a run-time width takes no block of a fixed width:
  `bit_vector` is not constructible from `from_blocks` and a `uint64_t`.
- A type that has bit storage without being it, such as a `bitset<64>` or a `bit_array<64>`, is not read through the
  tag. It converts through [`bit_convert`](../bit/bit_convert.md).

### See also

| Entity | Relation |
| :----- | :------- |
| [`bit_constructible_from`](../bit_concepts/bit_constructible_from.md) | the blocks an owner takes through the tag as they are |
| [`bit_blocks_extent_v`](../bit_type_traits/bit_blocks_extent.md) | the width the deduction guides read off the blocks |
| [`bit_convert`](../bit/bit_convert.md) | the conversion for a type that has bit storage without being it |
| [`from_range_t`](https://en.cppreference.com/w/cpp/ranges/from_range) | the standard's tag for an argument's elements, whose role this tag takes for its blocks |
