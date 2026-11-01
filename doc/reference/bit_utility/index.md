# `<xstd/bits/bit_utility.hpp>`

```cpp
namespace xstd {
  // [utility.syn], the in-place construction tags, beside which indeterminate leaves a fixed owner's blocks unwritten
  #define XSTD_INDETERMINATE see below
  struct indeterminate_t { explicit indeterminate_t() = default; };
  inline constexpr indeterminate_t indeterminate{};
  template<class T, size_t K>
      requires constructible_from<T, indeterminate_t>
    constexpr auto indeterminate_array() noexcept(is_nothrow_constructible_v<T, indeterminate_t>) -> array<T, K>;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_utility/indeterminate.hpp>`](indeterminate.md) | `indeterminate`, `indeterminate_array`, `indeterminate_t`, `XSTD_INDETERMINATE` |
