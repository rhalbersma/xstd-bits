# `<xstd/bits/bit_iterator/bit_next.hpp>`, `<xstd/bits/bit_iterator/bit_prev.hpp>`

## `bit_next`, `bit_prev`

As `ranges::next` and `ranges::prev` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* `I` is an iterator of ours, and for `bit_prev` it models `bidirectional_iterator`, as
  `ranges::prev` asks.
- *Effects:* `bit_next` calls `bit_advance(it, n)`, or `bit_advance(it, n, bound)`, on its copy `it`; `bit_prev`
  calls them with `-n`.
- *Returns:* `it`.
- *Complexity:* As for `bit_advance`.
