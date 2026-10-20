# `<xstd/bits/bit.hpp>`

```cpp
namespace xstd {
  // [bit.cast], function template bit_cast, whose counterpart across bit storages is bit_convert
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit/bit_convert.hpp>`](bit_convert.md) | `bit_convert` |
