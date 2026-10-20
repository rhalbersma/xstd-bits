# `<xstd/bits/bit_algorithm/bit_copy.hpp>`, `<xstd/bits/bit_algorithm/bit_copy_backward.hpp>`, `<xstd/bits/bit_algorithm/bit_move.hpp>`

## `bit_copy`, `bit_copy_backward`, `bit_move`

As `ranges::copy` and `ranges::copy_backward` in [[alg.copy]](https://eel.is/c++draft/alg.copy), and `ranges::move`
in [[alg.move]](https://eel.is/c++draft/alg.move), with:

- *Constraints:* `R` is a sequence of ours, a window included, and `O` is an output iterator of ours.
- *Preconditions:* Where the destination lies in the blocks `r` reads, `out`, or for `bit_copy_backward` `out_last`,
  is not in `(ranges::begin(r), ranges::end(r))`. These are [[alg.copy]](https://eel.is/c++draft/alg.copy)'s,
  except that a destination exactly on its source is admitted.
- *Effects:* The `N` positions of `r` are written a block of the destination at a time: lowest first for `bit_copy`
  and `bit_move`, highest first for `bit_copy_backward`. Source and destination may sit at any offsets in storages
  of different block types; the source's bits are assembled from its own blocks where the two differ, and each
  destination block is written under the mask of the positions that remain, so no position outside the destination
  range changes. Moving a `bool` is copying it.
- *Complexity:* O(`N`/`D`), where `D` is the digits of the narrower of the two blocks.
