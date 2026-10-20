# `<xstd/bits/bit_algorithm/bit_mismatch.hpp>`

## `bit_mismatch`

As `ranges::mismatch` in [[mismatch]](https://eel.is/c++draft/mismatch), with:

- *Constraints:* `R1` is a sequence of ours, and `remove_cvref_t<R1>` and `remove_cvref_t<R2>` are the same type:
  `bit_array<8>` and `bit_array<9>` are turned away.
- *Effects:* Two whole sequences whose storage compares its blocks are compared a block at a time, at two run-time
  sizes as well, stopping at the shorter's end, so set bits in the longer where the shorter has padding do not
  count. Two windows are compared by `ranges::mismatch(r1, r2)` itself.
- *Complexity:* O(min(`N1`, `N2`)/`D`) for two whole sequences; for two windows, `ranges::mismatch`'s.
