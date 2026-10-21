# `<xstd/bits/bit_type_traits/bit_least.hpp>`

## `least_block_t`, `bit_least`

- `least_block_t<N>` is the narrowest fixed-width block holding `N` bits in one: `uint8_t` where `N <= 8`, `uint16_t`
  where `N <= 16`, `uint32_t` where `N <= 32`, and `uint64_t` otherwise, of which `N` bits then take several.
  `least_block_t<0>` is `uint8_t`.
- `bit_least<Owner>`:
  - *Constraints:* `remove_cv_t<Owner>` is a resizable owner, whose width picks the block.
  - *Result:* `bit_rebind<least_block_t<N>, Owner>`, where `N` is `Owner`'s width.
  - `bit_least<basic_bit_array<uint64_t, 9>>` is `basic_bit_array<uint16_t, 9>`, and `bit_least<bit_fixed_set<9>>`
    is `basic_bit_fixed_set<size_t, uint16_t, 9>`. `bit_set` is turned away.
- *Remarks:* The name is `<cstdint>`'s: `uint_least8_t` is the smallest type of at least 8 bits. `bit_enum_set` and
  `bit_flag_set` are `bit_least` over a fixed set.
