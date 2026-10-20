# `bit_is_sorted`, `bit_is_partitioned`

As `ranges::is_sorted` in [[alg.sorting]](https://eel.is/c++draft/alg.sorting) and `ranges::is_partitioned` in
[[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* For `bit_is_sorted`, whether no `false` follows the first `true`; for `bit_is_partitioned`, whether no
  `true` follows the first `false`.
- *Complexity:* O(`N`/`D`): two searches, the second from where the first stopped.
