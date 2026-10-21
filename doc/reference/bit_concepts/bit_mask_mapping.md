# `<xstd/bits/bit_concepts/bit_mask_mapping.hpp>`

## `bit_mask_mapping`

```cpp
template<class Mapping, class Key>
concept bit_mask_mapping =
  sized_bit_index_mapping<Mapping, Key> and
  see below and
  requires (remove_cv_t<Key> mask, Mapping::block_type block) {
    { Mapping::to_block(mask) } -> same_as<typename Mapping::block_type>;
    { Mapping::from_block(block) } -> same_as<remove_cv_t<Key>>;
  };
```

- `see below` is that `typename Mapping::block_type` models `bit_blocks` and `bit_blocks_extent_v` of it is not
  `dynamic_extent`: one unsigned block, or a fixed number of them. It is never the signed type under an
  enumeration such as `launch`.
- A type models it if `to_block(mask)` holds every bit of `mask` at its position and `from_block(block)` is the
  mask with those bits, so that every value of `Key`, one bit or several, is a set of its one-bit keys.
- `bit_flag_mapping<byte>`, `bit_flag_mapping<byte, 2>`, `bit_flag_mapping<launch>` and
  `bit_flag_mapping<bitset<16>>` model it: `bit_flag_mapping<byte>::to_block(byte{0x02})` is `0x02`, and
  `from_block(0x03)` is `byte{0x03}`.
- `bit_key_mapping<size_t>`, `bit_range_mapping<uint8_t, 0, 8>` and `bit_key_mapping` of a listed enumeration do
  not: each ranks its values rather than reading one as a mask.
- `bit_flag_mapping<byte>` does not for `uint8_t`, the same bits under another type, nor for `const uint8_t`; it
  does for `const byte`, and `bit_flag_mapping<launch>` for `const volatile launch`.
- *Remarks:* A fixed set whose mapping models it reads every value of its key as a mask and converts to and from
  it; `bit_flag_set` is the alias over such a set.
