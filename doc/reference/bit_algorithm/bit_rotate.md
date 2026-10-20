# `bit_rotate`

As `ranges::rotate` in [[alg.rotate]](https://eel.is/c++draft/alg.rotate), with:

- *Constraints:* As for `bit_reverse`.
- *Preconditions:* `middle` is in `[ranges::begin(r), ranges::end(r)]`, checked by an `assert`.
- *Effects:* Position `i` takes position (`i` + `m`) mod `N`, for `m = middle - ranges::begin(r)`, so a whole turn
  is no turn. The blocks rotate by whole blocks, then a funnel shift moves the remaining bits across each boundary.
- *Returns:* `{ranges::begin(r) + (ranges::end(r) - middle), ranges::end(r)}`.
- *Complexity:* O(`N`/`D`), without allocating.
- *Remarks:* It moves no proxy through the standard library, so it is exact over proxy references where libstdc++'s
  `ranges::rotate` before version 16 is not ([GCC PR 121913](https://gcc.gnu.org/PR121913)). The `rotr(n)` of
  [P3103R2](https://wg21.link/P3103R2) is `bit_rotate(v, v.begin() + n)`, and its `rotl(n)` is
  `bit_rotate(v, v.end() - n)`.
