# `<xstd/bits/bit.hpp>`

```cpp
namespace xstd {
  // [bit.cast], function template bit_cast, whose counterpart across bit storages is bit_convert
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;

  // [bit.pow.two], integral powers of 2, over one owner as well as over one unsigned integer
  template<class T>
      requires see below
    constexpr auto has_single_bit(const T& x) noexcept -> bool;

  // [bit.rotate], rotating, beside which P3793R2's shl and shr shift a set's keys by any count
  template<class X, integer S>
      requires see below
    constexpr auto shl(X x, S s) noexcept(see below) -> X;
  template<class X, integer S>
      requires see below
    constexpr auto shr(X x, S s) noexcept(see below) -> X;

  // [bit.permute], permutation, over two owners of one type as well as over one unsigned integer
  template<class T>
      requires see below
    constexpr auto bit_compress(const T& x, const T& m) -> T;
  template<unsigned_integer U, class T>
      requires see below
    constexpr auto bit_compress(const T& x, const T& m) -> U;
  template<class T>
      requires see below
    constexpr auto bit_expand(const T& x, const T& m) -> T;
  template<unsigned_integer U, class T>
      requires see below
    constexpr auto bit_expand(U x, const T& m) -> T;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit/bit_convert.hpp>`](bit_convert.md) | `bit_convert` |
| [`<xstd/bits/bit/has_single_bit.hpp>`](has_single_bit.md) | `has_single_bit` |
| [`<xstd/bits/bit/shl.hpp>`](shl.md) | `shl` |
| [`<xstd/bits/bit/shr.hpp>`](shl.md) | `shr` |
| [`<xstd/bits/bit/bit_compress.hpp>`](bit_compress.md) | `bit_compress` |
| [`<xstd/bits/bit/bit_expand.hpp>`](bit_expand.md) | `bit_expand` |
