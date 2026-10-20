# `sized_bit_index_mapping`

```cpp
template<class Mapping, class Key>
concept sized_bit_index_mapping = bit_index_mapping<Mapping, Key> and requires (remove_cv_t<Key> key) {
  { Mapping::size } -> convertible_to<size_t>;
  { Mapping::is_key(key) } -> same_as<bool>;
};
```

- A type models it if it models `bit_index_mapping<Mapping, Key>`, places exactly `Mapping::size` keys at the
  positions `0` to `size - 1`, and `is_key(k)` is whether `k` is one of them.
- `bit_range_mapping<int, -50, 100>` and `bit_range_mapping<float_round_style, round_indeterminate, 5>` model it,
  as do `bit_find_mapping<int, Keys>`, `bit_key_mapping` of an enumeration listed in `enum_traits`, dense or with
  gaps, `bit_flag_mapping<launch>` and `bit_flag_mapping<byte>`, and a program's mapping that offsets a strong
  index into a closed range.
- `bit_key_mapping<Key>` for an unsigned `Key` does not: the identity leaves the universe open, though it says
  which values are keys. Neither does a program's `bit_key_mapping` for a strong index that names no size.
- A size alone does not make every value below it a key: a list of keys with gaps leaves values between them,
  which `is_key` turns away.
- *Remarks:* A set whose mapping models it closes its universe at `Mapping::size` positions.
