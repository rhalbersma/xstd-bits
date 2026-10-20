# `<xstd/bits/bit_algorithm/bit_sort.hpp>`

## `bit_sort`

As `ranges::sort` in [[alg.sorting]](https://eel.is/c++draft/alg.sorting), `false` ordered before `true`, with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* Counts the `k` positions that are `true`, then sets the first `N - k` positions `false` and the last `k`
  `true`. Every `false` is like every other, so the result is `ranges::stable_sort`'s too.
- *Complexity:* O(`N`/`D`).
