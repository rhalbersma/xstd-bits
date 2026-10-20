# `bit_all_of`, `bit_any_of`, `bit_none_of`

As `ranges::all_of`, `ranges::any_of` and `ranges::none_of` in
[[alg.all.of]](https://eel.is/c++draft/alg.all.of), [[alg.any.of]](https://eel.is/c++draft/alg.any.of) and
[[alg.none.of]](https://eel.is/c++draft/alg.none.of), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* Whether every, some or no position of `r` is `true`: `true`, `false` and `true` on an empty sequence.
- *Complexity:* O(`N`/`D`). A whole sequence asks its storage; a window asks the blocks it covers, its end blocks
  masked to its own positions.
