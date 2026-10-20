# `bit_rotate_copy`

As `ranges::rotate_copy` in [[alg.rotate]](https://eel.is/c++draft/alg.rotate), with:

- *Constraints:* As for `bit_copy`.
- *Preconditions:* `middle` is in `[ranges::begin(r), ranges::end(r)]`, checked by an `assert`.
- *Effects:* Copies `[middle, ranges::end(r))` and then `[ranges::begin(r), middle)`, each as `bit_copy` does.
- *Complexity:* As for `bit_copy`.
