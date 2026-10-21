# `<xstd/bits/bit_algorithm/bit_partition.hpp>`

## `bit_partition`

As `ranges::partition` in [[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* Counts the `k` positions that are `true`, then sets the first `k` positions `true` and the rest
  `false`.
- *Returns:* `{ranges::begin(r) + k, ranges::end(r)}`.
- *Complexity:* O(`N`/`D`).
