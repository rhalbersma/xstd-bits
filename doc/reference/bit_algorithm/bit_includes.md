# `bit_includes`, `bit_intersects`, `bit_disjoint`

As `ranges::includes` in [[includes]](https://eel.is/c++draft/includes), and as whether `ranges::set_intersection`
in [[set.intersection]](https://eel.is/c++draft/set.intersection) would write a key, or none, with:

- *Constraints:* `S` is a set of ours, a view included. The second parameter is `const type_identity_t<S>&`, so `S`
  deduces from `s1` alone and `s2` converts to it: a flag set takes a flag second,
  `bit_includes(perms, fs::perms::owner_read)`, and `bit_fixed_set<8>` with `bit_fixed_set<9>` is turned away.
- *Returns:*
  - `bit_includes`: whether every key of `s2` is a key of `s1`, as `ranges::includes(s1, s2, s1.key_comp())`;
  - `bit_intersects`: whether some key is a key of both;
  - `bit_disjoint`: `not bit_intersects(s1, s2)`.
- *Complexity:* O(`K`), where `K` is the number of blocks of the wider. The blocks both storages hold are compared
  pairwise, and `bit_includes` also asks whether `s2` holds a key in the blocks above `s1`'s. A width of zero
  fixed in the type is answered without a block: `true` for `bit_includes`, `false` for `bit_intersects`.
- They answer at any two run-time widths.
- *Remarks:* `x` is a subset of `y` where `bit_includes(y, x)`, and a proper one where `not bit_includes(x, y)` as
  well; every other relation between two sets is spelled from these three.
