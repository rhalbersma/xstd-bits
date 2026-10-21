# `<xstd/bits/bit_set/bit_flag_mapping.hpp>`

## `bit_flag_mapping`

```cpp
template<class Key, size_t N = see below>
  requires see below
struct bit_flag_mapping {
  using block_type = see below;
  static constexpr size_t size = N;
  static constexpr auto is_key(const Key& key) noexcept -> bool;
  static constexpr auto to_index(const Key& key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
  static constexpr auto to_block(const Key& mask) noexcept -> block_type;
  static constexpr auto from_block(const block_type& block) noexcept -> Key;
};
```

A bitmask type keyed on its own one-bit values.

- `Key` is an enumeration, an integer type other than `bool` and the character types, or a `std::bitset<M>` with
  `M > 0`. `block_type` is the unsigned word a mask is read and written as: an enumeration's or an integer's unsigned
  counterpart, or for a bitset the least block holding `M`, or past the widest block an array of them.
- `N` defaults to every bit of that word but a sign bit, and for a bitset to `M`. An enumeration with no fixed
  underlying type has no default, its values stopping at the bit-field its enumerators need. *Mandates:* `N` is no
  more than that default width.
- `is_key(k)` holds where `k` has exactly one bit set, below `N`; zero and a value of several bits are no key.
- `to_index(k)` is the position of `k`'s one bit. *Preconditions:* `k` has exactly one bit set.
- `from_index(i)` is the value with bit `i` alone. *Preconditions:* `i < N`.
- `to_block` and `from_block` read any value of `Key`, one-bit or not, as its block and back, which makes the
  mapping model `bit_mask_mapping<bit_flag_mapping<Key, N>, Key>`.
