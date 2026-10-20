# `bit_lexicographical_compare_three_way`

As `lexicographical_compare_three_way` in [[alg.three.way]](https://eel.is/c++draft/alg.three.way), over two ranges
rather than two iterator pairs, the shape [P2022R3](https://wg21.link/P2022R3) proposes for
`ranges::lexicographical_compare_three_way`, with:

- *Constraints:* As for `bit_equal`.
- *Returns:* `lexicographical_compare_three_way(ranges::begin(r1), ranges::end(r1), ranges::begin(r2),
  ranges::end(r2))`, a `strong_ordering`. Over two sequences, `false` precedes `true` and a prefix precedes the
  longer sequence. Over two sets it is `set`'s order, the keys compared lexicographically in iteration order:
  ascending keys position by position, and descending ones as their masks compared as numbers.
- *Complexity:* As for `bit_equal`.
- *Remarks:* `lexicographical_compare(r1, r2)` is `bit_lexicographical_compare_three_way(r1, r2) < 0`.
