# `bit_distance`

As `ranges::distance` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* For the first form, `I` is an iterator of ours. For the second, `R` models `ranges::common_range`
  and its iterator is an iterator of ours.
- *Preconditions:* `[first, last)` denotes a range.
- *Returns:* For the first form, `ranges::distance(first, last)` over a sequence, and over a set the number of keys
  in `[first, last)`. For the second, `ranges::distance(r)` where `R` models `ranges::sized_range`, as a set does,
  and otherwise `bit_distance(ranges::begin(r), ranges::end(r))`: so
  `bit_distance(subrange(s.lower_bound(a), s.lower_bound(b)))` counts the keys in `[a, b)`.
- *Complexity:* Constant over a sequence and over a sized range. Over a set's iterators, linear in the number of
  blocks between them: a popcount of the whole blocks and of the masked ends.
- *Remarks:* Counting a set's keys is a distance and not a count: `ranges::count(s, k)` asks how often `k` occurs,
  which `s.contains(k)` answers.
