# `<xstd/bits/bit_algorithm/bit_partition_point.hpp>`

## `bit_partition_point`

As `ranges::partition_point` in [[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Preconditions:* `r` is partitioned, as [[alg.partitions]](https://eel.is/c++draft/alg.partitions) requires.
- *Returns:* `ranges::begin(r) + i`, where `i` is the first position that is `false`, or `ranges::size(r)`.
- *Complexity:* O(`i`/`D`): a search up to the block holding the first `false`, where
  [[alg.partitions]](https://eel.is/c++draft/alg.partitions) bisects.
