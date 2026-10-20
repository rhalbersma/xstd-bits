# `<xstd/bits/bit_type_traits/bit_resize.hpp>`

## `bit_resize`

- *Constraints:* `remove_cv_t<Owner>` is a resizable owner.
- *Result:* The same owner `N` bits wide, every other argument kept, an allocator included.
- `bit_resize<16, basic_bit_array<uint8_t, 9>>` is `basic_bit_array<uint8_t, 16>`, `bit_resize<64, bit_fixed_set<9>>`
  is `bit_fixed_set<64>`, `bit_resize<0, basic_bit_inplace_set<size_t, uint8_t, 9>>` is
  `basic_bit_inplace_set<size_t, uint8_t, 0>`, and `bit_resize<128, basic_bit_small_vector<uint64_t, 9>>` is
  `basic_bit_small_vector<uint64_t, 128, boost::container::new_allocator<uint64_t>>`.
- `bit_vector` and `bit_set`, whose widths are run-time values, have none to change and are turned away.
- *Remarks:* The form is that of `simd`'s `resize_t`, the new width first.
