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
- A width that is not a whole number of blocks keeps the bits past `N` zero, which `size()`, `==` and hashing rely
  on, so its blocks are zeroed through the tag as well. The tag leaves blocks unwritten where every bit is a key.
- A default-constructed set is empty, as [[container.reqmts]](https://eel.is/c++draft/container.reqmts) has `X u;`,
  and so is a value-initialized one: the tag is the only way to an indeterminate set.
- Under C++26, [P2795R5](https://wg21.link/p2795r5) gives an automatic variable without an initializer an erroneous
  value, which an implementation may initialize. A variable that must stay unwritten says so with
  `[[indeterminate]]`, as `[[indeterminate]] auto s = bit_fixed_set<64>(indeterminate);` does.
- `bit_array` needs no tag: like `array<bool, N>`, `bit_array<N> a;` default-initializes and `bit_array<N> a{};` is
  all `false`.

### See also

| Entity | Relation |
| :----- | :------- |
| [`basic_bit_fixed_set`](bit_fixed_set.md) | the owner that takes the tag |
| [`basic_bit_array`](../bit_array/bit_array.md) | default-initializes as `array<bool, N>` does, without a tag |
| [`boost::container::default_init_t`](https://www.boost.org/doc/libs/release/doc/html/container/extensions.html) | Boost.Container's tag for default- rather than value-initialized elements |
| [`make_unique_for_overwrite`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique) | the standard's default-initializing counterpart of `make_unique`, from [P1020R1](https://wg21.link/p1020r1) |
