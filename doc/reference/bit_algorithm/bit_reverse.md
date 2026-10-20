# `bit_reverse`

As `ranges::reverse` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* `R` is a whole sequence of ours whose storage is writable: an owner that is not `const`, or a whole
  view of bits that are not `const`. A window, which shares its end blocks with positions outside it, is turned
  away.
- *Effects:* Position `i` takes position `N - 1 - i`: the blocks in reverse order, each block's bits reversed, and
  the padding, which then lies at the bottom, shifted out. The padding stays clear.
- *Returns:* `ranges::end(r)`.
- *Complexity:* O(`N`/`D`), without allocating.
