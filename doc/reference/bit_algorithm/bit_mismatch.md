# `<xstd/bits/bit_algorithm/bit_mismatch.hpp>`

## `bit_mismatch`

As `ranges::mismatch` in [[mismatch]](https://eel.is/c++draft/mismatch), with:

- *Constraints:* `R1` and `R2` are sequences of ours, of any two types: `bit_array<8>` beside `bit_array<9>`, a
  `bit_vector` beside a window over a `bit_array`.
- *Effects:* Two whole sequences of one type whose storage compares its blocks are compared through that storage, at
  two run-time sizes as well. Any other two are compared a block of `r1` at a time, each read at its offset and the
  other's bits assembled to match. Either way the comparison stops at the shorter's end, so set bits in the longer
  where the shorter has padding do not count.
- *Complexity:* O(min(`N1`, `N2`)/`D1`), for `D1` the digits of `r1`'s block.
