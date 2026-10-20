# `bit_swap_ranges`

As `ranges::swap_ranges` in [[alg.swap]](https://eel.is/c++draft/alg.swap), with:

- *Constraints:* `R1` and `R2` are writable sequences of ours, windows included.
- *Effects:* Exchanges the first `M` = min(`ranges::size(r1)`, `ranges::size(r2)`) positions of the two, a block of
  the narrower block type at a time, at any two offsets.
- *Complexity:* O(`M`/`D`), where `D` is the digits of the narrower block.
