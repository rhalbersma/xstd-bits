# `<xstd/bits/bit_type_traits/bit_rebind.hpp>`

## `bit_rebind`

- *Constraints:* `remove_cv_t<Owner>` is a rebindable owner. `Block` models `unsigned_integer`, so `bool` and `int`
  are turned away.
- *Result:* The same owner over `remove_cv_t<Block>`, every other argument kept: the width, the key, the mapping
  and the comparison. An allocator `A` becomes `allocator_traits<A>::rebind_alloc<remove_cv_t<Block>>`.
- `bit_rebind<uint8_t, basic_bit_array<uint64_t, 9>>` is `basic_bit_array<uint8_t, 9>`, and
  `bit_rebind<uint32_t, basic_bit_fixed_set<size_t, uint8_t, 9, bit_key_mapping<size_t>, greater<size_t>>>` is
  `basic_bit_fixed_set<size_t, uint32_t, 9, bit_key_mapping<size_t>, greater<size_t>>`.
- `bit_rebind<uint8_t, bit_vector>` is `basic_bit_vector<uint8_t, allocator<uint8_t>>`, and
  `bit_rebind<uint16_t, bit_small_vector<9>>` is
  `basic_bit_small_vector<uint16_t, 9, boost::container::new_allocator<uint16_t>>`.
- The owner rebound holds the same bits: `bit_rebind<uint8_t, bit_vector>` constructed from a `bit_vector`'s
  iterators has its size and its elements, in `vector<uint8_t>` blocks.
- `bit_rebind<const volatile uint8_t, bit_vector>` is `bit_rebind<uint8_t, bit_vector>`: a block is held by value.
- *Remarks:* The form is that of `simd`'s `rebind_t`, the new block first.
