# `bit_shift_left`, `bit_shift_right`

As `ranges::shift_left` and `ranges::shift_right` in [[alg.shift]](https://eel.is/c++draft/alg.shift), with:

- *Constraints:* As for `bit_reverse`.
- *Preconditions:* `0 <= n`, checked by an `assert`.
- *Effects:* None where `n == 0` or `n >= N`. Otherwise `bit_shift_left` moves position `i + n` to `i`, and
  `bit_shift_right` position `i` to `i + n`, for every `i < N - n`. The `n` positions vacated are `false`, where
  [[alg.shift]](https://eel.is/c++draft/alg.shift) leaves them valid but unspecified.
- *Returns:* As [[alg.shift]](https://eel.is/c++draft/alg.shift): for `bit_shift_left`,
  `{ranges::begin(r), ranges::begin(r) + (N - n)}` where `n < N` and `{ranges::begin(r), ranges::begin(r)}`
  otherwise; for `bit_shift_right`, `{ranges::begin(r) + n, ranges::end(r)}` where `n < N` and
  `{ranges::end(r), ranges::end(r)}` otherwise.
- *Complexity:* O(`N`/`D`).
- *Remarks:* A sequence's front is a bit string's low end, so `bit_shift_left` is the storage's `>>=` and
  `bit_shift_right` its `<<=`.
