# `bit_enum_set`

- `bit_enum_set<Enum>` is the fixed set of an enumeration's keys, one bit each in the smallest block that holds them:
  `basic_bit_fixed_set<Enum, least_block_t<S>, S, bit_key_mapping<Enum>>` for `S = bit_key_mapping<Enum>::size`.
- *Constraints:* `Enum` is an enumeration whose default mapping names a size: its values listed in `enum_traits`,
  ended by a `count` sentinel, or a `bit_key_mapping` specialized directly.
- `basic_bit_fixed_set{E::a, E::b}` deduces the same type. Another block is
  `basic_bit_fixed_set<Enum, Block, S>`'s to spell.
- An enumerator is the one-element set holding it, on either side of `|`, `&`, `^` and `-`, as for `basic_bit_set`.
