# `<xstd/bits/bit.hpp>`

```cpp
namespace xstd {
  // [bit.cast], function template bit_cast, whose counterpart across bit storages is bit_convert
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;

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
| [`<xstd/bits/bit/bit_compress.hpp>`](bit_compress.md) | `bit_compress` |
| [`<xstd/bits/bit/bit_expand.hpp>`](bit_expand.md) | `bit_expand` |
