# `<xstd/bits/bit.hpp>`

```cpp
namespace xstd {
  // [bit.cast], function template bit_cast, whose counterpart across bit storages is bit_convert
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;

  // [bit.rotate], rotating, beside which P3793R2's shl and shr shift a set's keys by any count
  template<class X, integer S>
      requires see below
    constexpr auto shl(X x, S s) noexcept(see below) -> X;
  template<class X, integer S>
      requires see below
    constexpr auto shr(X x, S s) noexcept(see below) -> X;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit/bit_convert.hpp>`](bit_convert.md) | `bit_convert` |
| [`<xstd/bits/bit/shl.hpp>`](shl.md) | `shl` |
| [`<xstd/bits/bit/shr.hpp>`](shl.md) | `shr` |
