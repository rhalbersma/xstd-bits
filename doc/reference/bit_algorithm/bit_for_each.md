# `<xstd/bits/bit_algorithm/bit_for_each.hpp>`

## `bit_for_each`

As `ranges::for_each` in [[alg.foreach]](https://eel.is/c++draft/alg.foreach), with:

- *Constraints:* `invocable<F&, ranges::range_value_t<R>>` is modeled, and:
  - for the first form, `R` is a sequence or a set of ours, a window included;
  - for the second, `remove_cvref_t<R>` is `ranges::reverse_view<V>`, where `V` is a view of ours or a
    `ranges::ref_view` of an owner of ours: what `views::reverse` makes of a view or of an lvalue owner.
- *Effects:* Calls `f` with each element by value, a `bool` or a key, in the order `r` iterates. The second form
  walks the range under the `reverse_view` against its order, so `bit_for_each(views::reverse(r), f)` is
  `ranges::for_each(views::reverse(r), f)`.
- *Returns:* `{ranges::end(r), std::move(f)}`.
- *Complexity:* Exactly `ranges::distance(r)` applications of `f`, and one load per block. A set's keys come from
  each block lowest first or highest first, as the set and the direction order them.
- A functor that takes the element as a non-`const` lvalue reference is turned away, and of a functor overloaded on
  value category the overload taking an rvalue is the one called.
- A functor returning `bool` is called on every element, its answer ignored, as `ranges::for_each` calls it.
