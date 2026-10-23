# `<xstd/bits/bit_algorithm/bit_find.hpp>`, `<xstd/bits/bit_algorithm/bit_find_if.hpp>`, `<xstd/bits/bit_algorithm/bit_find_last.hpp>`

## `bit_find`, `bit_find_if`, `bit_find_last`

As `ranges::find`, `ranges::find_if` and `ranges::find_last` in [[alg.find]](https://eel.is/c++draft/alg.find) and
[[alg.find.last]](https://eel.is/c++draft/alg.find.last), with:

- *Constraints:* For `bit_find` and `bit_find_last`, `R` is a sequence of ours, a window included. For
  `bit_find_if`, `R` is a sequence or a set of ours, a window included, and `predicate<Pred&,
  ranges::range_value_t<R>>` is modeled.
- *Returns:*
    - For `bit_find`, `ranges::begin(r) + i`, where `i` is the first position holding `value`, or `ranges::size(r)`
      where none does.
    - For `bit_find_if`, the iterator to the first element, a `bool` or a key, in the order `r` iterates, for which
      `pred` returns `true`, or `ranges::end(r)` where there is none.
    - For `bit_find_last`, `{ranges::begin(r) + i, ranges::end(r)}`, where `i` is the last such position, or
      `{ranges::end(r), ranges::end(r)}` where none is.
- *Complexity:* O(`N`/`D`) for `bit_find` and `bit_find_last`. `bit_find` reads up from the first block and
  `bit_find_last` down from the last, each stopping at the block that holds the answer. A search for `false` reads
  the blocks complemented, at the same cost. For `bit_find_if`:
    - over a set, one load per block up to the one holding the answer, and one application of `pred` per key before
      it and to it, the keys coming from each block lowest first or highest first, as the set orders them;
    - over a sequence, at most two applications of `pred`, which, being equality-preserving, answers every `bool`
      alike, and then `bit_find` for the one value it accepts. A sequence of one element applies `pred` once.
- `bit_find_if` hands `pred` each element by value. Over a set it is the early exit `bit_for_each` has none of, and
  walks the keys without the iterator, whose increment searches again from the position it holds.
