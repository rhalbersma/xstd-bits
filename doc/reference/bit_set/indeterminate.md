# `<xstd/bits/bit_set/indeterminate.hpp>`

## `xstd::indeterminate_t`, `xstd::indeterminate`, `xstd::indeterminate_array`, `XSTD_INDETERMINATE`

```cpp
#define XSTD_INDETERMINATE see below

struct indeterminate_t { explicit indeterminate_t() = default; };
inline constexpr indeterminate_t indeterminate{};

template<class T, size_t K>
    requires constructible_from<T, indeterminate_t>
  constexpr auto indeterminate_array() noexcept(is_nothrow_constructible_v<T, indeterminate_t>) -> array<T, K>;
```

A tag that asks an owner to leave its blocks default-initialized, the first step of a two-step initialization:
`auto s = bit_fixed_set<64>(indeterminate);` writes nothing, and `s = t;` then gives it a value. It is for a set built
in a hot path and assigned before anything reads it, such as a field of a struct that a later step fills. The fixed
arrays take it too: `bit_array<N>(indeterminate)` and `bit_fixed_valarray<N>(indeterminate)`. Its default
constructor is `explicit`, so `{}` cannot stand in for it.

`indeterminate_array<T, K>()` returns an `array<T, K>` whose every element is built from the tag. Each element is
initialized directly from the prvalue `T(indeterminate)`, so nothing is copied or zeroed. It fills an `array` member
in a default member initializer, as a braced `{indeterminate}` fills only the first element.

### Notes

- The owners of a fixed width take the tag: `basic_bit_fixed_set` and the sets that name it, `bit_enum_set` and
  `bit_flag_set`, `basic_bit_array` and `basic_bit_fixed_valarray`. A run-time width starts at zero and has nothing to
  leave unwritten. Until it is assigned, the owner's only valid operations are assignment and destruction: reading it
  in any other way, `size()`, `contains`, a subscript, a comparison, a copy, is undefined. A hardened build cannot
  check this; MemorySanitizer can.
- Only a whole write may follow, so the tag writes nothing at any width, the bits past `N` of a width that does not
  fill its blocks included: the assignment that follows copies them clear from an owner that has a value.
- Inserting a key after the tag, `a[0] = true`, or any other write of one bit, is undefined: a bit is written by
  reading the block that holds it, and that block has no value yet.
- A default-constructed set is empty, as [[container.reqmts]](https://eel.is/c++draft/container.reqmts) has `X u;`,
  and a default-constructed array is every position `false`, value-initialized or not: the tag is the only way to an
  indeterminate owner. `array<bool, N> a; a[0] = true;` is valid, and the packed `a[0] = true` reads the block that
  bit 1 shares, so a default-constructed `bit_array` starts clear to admit the same code.
- Under C++26, [P2795R5](https://wg21.link/p2795r5) gives an automatic variable's storage erroneous values, which an
  implementation may fill in whatever its constructors leave. A variable that must stay unwritten says so with
  `[[indeterminate]]`, which applies to the whole variable. `XSTD_INDETERMINATE` is that attribute's name where the
  language and the compiler have it, and empty elsewhere, so `[[XSTD_INDETERMINATE]] auto s =
  bit_fixed_set<64>(indeterminate);` compiles under C++23 as under C++26, as `[[XSTD_NO_UNIQUE_ADDRESS]]` does. A
  definition of it made before the header is included wins.

### See also

| Entity | Relation |
| :----- | :------- |
| [`basic_bit_fixed_set`](bit_fixed_set.md) | an owner that takes the tag, and empty by default |
| [`basic_bit_array`](../bit_array/bit_array.md) | an owner that takes the tag, and zeroed by default, so that writing one element after default construction is valid, as for `array<bool, N>` |
| [`basic_bit_fixed_valarray`](../bit_valarray/bit_fixed_valarray.md) | an owner that takes the tag, and zeroed by default, as `basic_bit_array` is |
| [`boost::container::default_init_t`](https://www.boost.org/doc/libs/release/doc/html/container/extensions.html) | Boost.Container's tag for default- rather than value-initialized elements |
| [`make_unique_for_overwrite`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique) | the standard's default-initializing counterpart of `make_unique`, from [P1020R1](https://wg21.link/p1020r1) |
