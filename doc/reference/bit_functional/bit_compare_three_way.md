# `bit_compare_three_way`

As `compare_three_way` in [[comparisons.three.way]](https://eel.is/c++draft/comparisons.three.way), of the same shape
as `bit_less`, with:

- *Constraints:* As for `bit_less`.
- *Returns:* `xstd::ranges::bit_lexicographical_compare_three_way(x, y)`, a `strong_ordering`.
- It answers `strong_ordering::equal` exactly where `bit_equal_to` answers `true`.
