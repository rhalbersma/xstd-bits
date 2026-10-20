# `bit_blocks_extent_v`

- *Constraints:* `Bits` models `bit_blocks`, so a `volatile` block or range of them names no extent.
- *Value:* For `B` = `remove_cv_t<Bits>` and `D` the `xstd::numeric_limits<Block>::digits` of its block type:
  - `D`, where `B` is a block;
  - `D * K`, where `B` is `array<Block, K>` or the built-in array `Block[K]`, the element's cv stripped;
  - `D * E`, where `B` is `span<Block, E>` and `E` is not `dynamic_extent`;
  - `dynamic_extent` otherwise, as for a `vector`, an `inplace_vector` or a `span` of dynamic extent.
- `bit_blocks_extent_v<uint8_t>` is 8, `bit_blocks_extent_v<const uint64_t>` 64, `bit_blocks_extent_v` of
  `array<uint16_t, 3>`, of `const array<uint16_t, 3>` and of `const uint16_t[3]` 48, of `span<const uint32_t, 2>`
  64, and of `uint64_t[4]` 256, as of `array<uint64_t, 4>`.
- *Remarks:* It is the default width of `bit_set_view` and `bit_span`, so `bit_set_view<span<uint32_t>>` is
  `bit_set_view<span<uint32_t>, dynamic_extent>`, and the width the `from_blocks` deduction guides deduce.
