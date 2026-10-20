# `bit_blocks`

```cpp
template<class Bits>
concept bit_blocks = see below;
```

- `Bits` models it where it is either of:
  - a *block*: a type that models xstd-ints's `unsigned_integer`, `const` or not, and is not `volatile`;
  - a *range of blocks*: `Bits` models `ranges::sized_range` and `ranges::contiguous_range`,
    `remove_reference_t<ranges::range_reference_t<Bits>>` is a block, and `blocks[n]` is valid for an lvalue
    `blocks` of type `Bits` and an `n` of type `ranges::range_size_t<Bits>`. The block is asked of the reference
    type, as `ranges::range_value_t` drops the cv.
- A block models it: `uint8_t`, `uint64_t`, `const uint64_t`.
- A range of blocks models it: `array<uint16_t, 3>`, `vector<size_t>`, `span<uint32_t>`, `span<const uint32_t, 2>`,
  `inplace_vector<uint16_t, 3>` where the library provides it, and the built-in arrays `uint64_t[4]` and
  `const uint16_t[3]`.
- `vector<bool>` does not, nor do `deque<uint32_t>` and `list<uint32_t>`, which are not contiguous.
- `bitset<64>`, `bit_set` and `bit_array<64>` do not: each *has* bit storage and is not bit storage.
- `volatile uint32_t`, `const volatile uint32_t`, `span<volatile uint32_t>` and `volatile uint32_t[2]` are turned
  away. A register's accesses must each stay one volatile access, and the block algorithms copy and merge them.
- *Remarks:* It is the constraint on the storage `bit_set_view` and `bit_span` view and on `bit_blocks_extent_v`,
  so a register array is turned away at the view's name rather than inside a block algorithm. An owner holds its
  blocks by value and takes only a cv-unqualified block: `basic_bit_array<const uint32_t, 32>` is not a type.
