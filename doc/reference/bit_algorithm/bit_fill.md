# `<xstd/bits/bit_algorithm/bit_fill.hpp>`

## `bit_fill`

As `ranges::fill` in [[alg.fill]](https://eel.is/c++draft/alg.fill), with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* A whole sequence fills every block of its storage; a window sets its own positions, its end blocks
  masked, and leaves every other position as it was.
- *Complexity:* O(`N`/`D`).
