# `bit_reverse_copy`

As `ranges::reverse_copy` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* As for `bit_copy`.
- *Effects:* Each block of the destination is read from the top of `r`, its bits reversed, and written as `bit_copy`
  writes it.
- *Complexity:* As for `bit_copy`.
