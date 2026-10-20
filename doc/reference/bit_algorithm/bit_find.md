# `bit_find`, `bit_find_last`

As `ranges::find` and `ranges::find_last` in [[alg.find]](https://eel.is/c++draft/alg.find) and
[[alg.find.last]](https://eel.is/c++draft/alg.find.last), with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* For `bit_find`, `ranges::begin(r) + i`, where `i` is the first position holding `value`, or
  `ranges::size(r)` where none does. For `bit_find_last`, `{ranges::begin(r) + i, ranges::end(r)}`, where `i` is
  the last such position, or `{ranges::end(r), ranges::end(r)}` where none is.
- *Complexity:* O(`N`/`D`). `bit_find` reads up from the first block and `bit_find_last` down from the last, each
  stopping at the block that holds the answer. A search for `false` reads the blocks complemented, at the same cost.
