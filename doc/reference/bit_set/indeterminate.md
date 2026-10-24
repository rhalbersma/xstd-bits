# `<xstd/bits/bit_set/indeterminate.hpp>`

## `xstd::indeterminate_t`, `xstd::indeterminate`, `xstd::indeterminate_array`

```cpp
struct indeterminate_t { explicit indeterminate_t() = default; };
inline constexpr indeterminate_t indeterminate{};

template<class T, size_t K>
    requires constructible_from<T, indeterminate_t>
  constexpr auto indeterminate_array() noexcept(is_nothrow_constructible_v<T, indeterminate_t>) -> array<T, K>;
```

A tag that asks an owner to leave its blocks default-initialized, the first step of a two-step initialization:
`auto s = bit_fixed_set<64>(indeterminate);` writes nothing, and `s = t;` then gives it a value. It is for a set built
in a hot path and assigned before anything reads it, such as a field of a struct that a later step fills. Its default
constructor is `explicit`, so `{}` cannot stand in for it.

`indeterminate_array<T, K>()` returns an `array<T, K>` whose every element is built from the tag. Each element is
initialized directly from the prvalue `T(indeterminate)`, so nothing is copied or zeroed. It fills an `array` member
in a default member initializer, as a braced `{indeterminate}` fills only the first element.

### Notes

- `basic_bit_fixed_set` and the sets that name it, `bit_enum_set` and `bit_flag_set`, take the tag. Until it is
  assigned, the set's only valid operations are assignment and destruction: reading it in any other way, `size()`,
  `contains`, a comparison, a copy, is undefined. A hardened build cannot check this; MemorySanitizer can.
- Only a whole write may follow, so the tag writes nothing at any width, the bits past `N` of a width that does not
  fill its blocks included: the assignment that follows copies them clear from a set that has a value.
- Inserting a key after the tag, or any other write of one bit, is undefined: a bit is written by reading the block
  that holds it, and that block has no value yet.
- A default-constructed set is empty, as [[container.reqmts]](https://eel.is/c++draft/container.reqmts) has `X u;`,
  and so is a value-initialized one: the tag is the only way to an indeterminate set.
- Under C++26, [P2795R5](https://wg21.link/p2795r5) gives an automatic variable without an initializer an erroneous
  value, which an implementation may initialize. A variable that must stay unwritten says so with
  `[[indeterminate]]`, as `[[indeterminate]] auto s = bit_fixed_set<64>(indeterminate);` does.
- `bit_array` takes no tag. `array<bool, N> a; a[0] = true;` is valid, and the packed `a[0] = true` reads the block
  that bit 1 shares, so a default-constructed `bit_array` must start clear to admit the same code.

### See also

| Entity | Relation |
| :----- | :------- |
| [`basic_bit_fixed_set`](bit_fixed_set.md) | the owner that takes the tag |
| [`basic_bit_array`](../bit_array/bit_array.md) | zeroed by default, so that writing one element after default construction is valid, as for `array<bool, N>` |
| [`boost::container::default_init_t`](https://www.boost.org/doc/libs/release/doc/html/container/extensions.html) | Boost.Container's tag for default- rather than value-initialized elements |
| [`make_unique_for_overwrite`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique) | the standard's default-initializing counterpart of `make_unique`, from [P1020R1](https://wg21.link/p1020r1) |
