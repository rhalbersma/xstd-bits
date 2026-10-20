# `<xstd/bits/bit_set/bit_flag_set.hpp>`

## `bit_flag_set`

- `bit_flag_set<Mask, N>` is the set of a bitmask type's one-bit values below `N`:
  `basic_bit_fixed_set<Mask, least_block_t<N>, N, bit_flag_mapping<Mask, N>, greater<Mask>>`.
- *Constraints:* `Mask` models `xstd::bit_mask`, is a signed integer, or is an enumeration whose `|` compiles,
  whatever it returns; and it is an enumeration, an integer or a `std::bitset`, as `bit_flag_mapping` takes.
  `N` defaults as `bit_flag_mapping`'s does: a signed type keeps its sign bit out, and an enumeration with no fixed
  underlying type spells its width.
- The mapping reads every value of `Mask` as a mask, so the set converts implicitly from and to it, and takes it in
  every mixed operator, as `basic_bit_fixed_set`'s mask members specify. Code written against
  `std::filesystem::perms` keeps its constants and changes only the variable's type.
- A braced list is the union of its values, and `contains(k)` is membership of one flag, false for a value of
  several bits.
- Iteration yields the one-bit values from the highest down, and `<=>` is the masks' numeric order.
- `xstd::ranges::bit_includes(p, m)`, `bit_intersects(p, m)` and `bit_disjoint(p, m)` take a mask as their second
  argument, and answer all-of, any-of and none-of. `~p` stays within `N`.
