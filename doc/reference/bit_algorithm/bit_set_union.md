# `<xstd/bits/bit_algorithm/bit_set_union.hpp>`, `<xstd/bits/bit_algorithm/bit_set_intersection.hpp>`, `<xstd/bits/bit_algorithm/bit_set_difference.hpp>`, `<xstd/bits/bit_algorithm/bit_set_symmetric_difference.hpp>`

## `bit_set_union`, `bit_set_intersection`, `bit_set_difference`, `bit_set_symmetric_difference`

As `ranges::set_union`, `ranges::set_intersection`, `ranges::set_difference` and
`ranges::set_symmetric_difference` in [[set.union]](https://eel.is/c++draft/set.union),
[[set.intersection]](https://eel.is/c++draft/set.intersection),
[[set.difference]](https://eel.is/c++draft/set.difference) and
[[set.symmetric.difference]](https://eel.is/c++draft/set.symmetric.difference), returning the set rather than
writing through an output iterator, with:

- *Constraints:* `S` is a set of ours whose operator `|`, `&`, `-` or `^` respectively answers an `S`: an owner. A
  view, which has no result to hold, is turned away. `s2` converts to `S`, as for `bit_includes`.
- *Returns:* `s1 | s2`, `s1 & s2`, `s1 - s2` and `s1 ^ s2`: the set that
  [range-v3](https://github.com/ericniebler/range-v3)'s `views::set_union(s1, s2, s1.key_comp())` and its siblings
  would hold, of the first argument's type, whatever widths held the keys.
- *Complexity:* That of the operator.
- *Remarks:* The exception specification is `noexcept(declval<const S&>() | declval<const S&>())`, with the
  operator each one calls.
