# `<xstd/bits/bit_type_traits/bit_align.hpp>`

## `bit_align`

- *Constraints:* `remove_cv_t<Owner>` is a resizable owner.
- *Result:* `bit_resize<K * D, Owner>`, where `N` is `Owner`'s width, `D` the digits of its block and `K` the
  ⌈`N`/`D`⌉ blocks that `N` bits take: the width rounded up to whole blocks, so that no block carries an unused
  tail. A width already whole, zero included, stays as it is.
- `bit_align<basic_bit_array<uint8_t, 9>>` is `basic_bit_array<uint8_t, 16>`, and `bit_align` of
  `basic_bit_array<uint8_t, 16>` and of `basic_bit_array<uint8_t, 0>` is the type itself. `bit_set` is turned away.
- The transformations compose, and the order matters: `bit_align<bit_least<bit_fixed_set<3>>>` is
  `basic_bit_fixed_set<size_t, uint8_t, 8>`, where `bit_least<bit_align<bit_fixed_set<3>>>` rounds to the digits
  of `size_t` first, and is `basic_bit_fixed_set<size_t, uint64_t, 64>` where `size_t` is 64 bits.
- *Remarks:* A fixed-width owner whose width fills its blocks is a structural type
  ([[temp.param]](https://eel.is/c++draft/temp.param)), so `bit_align<bit_array<N>>` and
  `bit_align<bit_fixed_set<N>>` are, at any `N`.
