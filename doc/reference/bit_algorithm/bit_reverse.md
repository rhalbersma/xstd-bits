# `<xstd/bits/bit_algorithm/bit_reverse.hpp>`

## `bit_reverse`

As `ranges::reverse` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* `R` is a sequence of ours whose storage is writable: an owner that is not `const`, or a view of
  bits that are not `const`, a window included.
- *Effects:* Position `i` takes position `N - 1 - i`. A whole sequence whose storage reverses itself reverses the
  blocks in order, each block's bits too, and shifts out the padding, which then lies at the bottom; the padding stays
  clear. A window trades a block from each end inward, each reversed, and reverses what remains in the middle; its end
  blocks' positions outside it keep their values.
- *Returns:* `ranges::end(r)`.
- *Complexity:* O(`N`/`D`), without allocating.
