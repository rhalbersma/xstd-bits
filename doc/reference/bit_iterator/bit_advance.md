# `bit_advance`

As `ranges::advance` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* `I` is an iterator of ours.
- *Preconditions:* As [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops)'s. For the first form over a set,
  the `n` keys are there to step over, checked by an `assert`.
- *Effects:* Over a sequence, `ranges::advance(it, n)` and `ranges::advance(it, n, bound)`. Over a set, moves `it`
  `n` keys on in the set's order, or `-n` back, skipping whole blocks by their popcount and selecting the key within
  the block that holds it, from the block's top where the walk goes down; the second form stops at `bound`.
- *Returns:* For the second form, the steps not taken, with the sign of `n`, as `ranges::advance(it, n, bound)`
  answers them.
- *Complexity:* Constant over a sequence. Over a set, linear in the number of blocks between `it` and where it
  stops, rather than one step per key.
- A view's iterators step over the keys it views, its owner's or lent blocks, and an empty set's `begin()` is its
  `end()`.
