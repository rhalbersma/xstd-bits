# `bit_index_mapping`

```cpp
template<class Mapping, class Key>
concept bit_index_mapping = requires (remove_cv_t<Key> key, size_t index) {
  { Mapping::to_index(key) } -> same_as<size_t>;
  { Mapping::from_index(index) } -> same_as<remove_cv_t<Key>>;
};
```

- A type models it if `to_index` preserves order, `a < b` exactly where `to_index(a) < to_index(b)`, and
  `from_index` inverts `to_index` on the keys the mapping places.
- `bit_key_mapping<Key>` models it for every unsigned block type `Key`.
- `bit_range_mapping<float_round_style, round_indeterminate, 5>`, `bit_find_mapping<int, Keys>` over a sorted list
  with gaps, `bit_flag_mapping<launch>` and `bit_flag_mapping<byte>` model it for their keys, and so does a
  program's specialization of `bit_key_mapping` for a strong index type.
- A mapping does not model it for a key of another type, however alike in width, signedness or header:
  `bit_key_mapping<size_t>` for `unsigned char`, `bit_range_mapping<int, -50, 100>` for `long`,
  `bit_flag_mapping<byte>` for `uint8_t`, and `bit_flag_mapping<filesystem::perms>` for `filesystem::perm_options`.
- `from_index` must answer exactly `Key`: a mapping for a wider key takes a narrower one in through `to_index`, and
  hands back a value of the wrong type.
- `bit_key_mapping<size_t>` models it for `const size_t` and `volatile size_t`, and `bit_flag_mapping<launch>` for
  `const volatile launch`, but `bit_key_mapping<size_t>` does not for `const unsigned char`.
