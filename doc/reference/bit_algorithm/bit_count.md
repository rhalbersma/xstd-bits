# `bit_count`

As `ranges::count` in [[alg.count]](https://eel.is/c++draft/alg.count), with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* The number of positions of `r` holding `value`, as `ranges::range_difference_t<const R>`, signed as
  `ranges::count`'s result is: the ones popcounted, and the zeros as `ranges::size(r)` less the ones.
- *Complexity:* O(`N`/`D`) popcounts. The padding of the last block is never counted.
