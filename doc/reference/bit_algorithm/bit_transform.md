# `bit_transform`

As `ranges::transform` in [[alg.transform]](https://eel.is/c++draft/alg.transform), with:

- *Constraints:* `R`, or `R1` and `R2`, are sequences of ours, windows included; `O` is an output iterator of ours;
  `invocable<F&, bool>`, or `invocable<F&, bool, bool>`, is modeled and its result converts to `bool`. A function
  taking `bool&`, or answering a `string`, is turned away.
- *Effects:* The unary form calls `f(false)` and `f(true)` once each; the binary form calls `f` once at each of the
  four pairs. The answers make a truth table that is applied as one block expression, a block of the destination at
  a time as `bit_copy` writes it: for the unary form `(x & T) | (~x & U)`, where `T` and `U` are all ones or all
  zeros as `f(true)` and `f(false)` answer. The binary form goes as far as the shorter input. `out` may be
  `ranges::begin(r1)`, as [[alg.transform]](https://eel.is/c++draft/alg.transform) allows.
- *Complexity:* Exactly two applications of `f` for the unary form and four for the binary, whatever `N`, an empty
  input included; O(`N`/`D`) block operations.
- *Remarks:* The positions written are those `ranges::transform` writes; a function with side effects sees two or
  four calls where `ranges::transform` makes one per element.
