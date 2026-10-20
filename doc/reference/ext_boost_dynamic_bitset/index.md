# `<xstd/bits/ext/boost/dynamic_bitset.hpp>`

```cpp
namespace xstd {
  // exported, and valid with boost::dynamic_bitset<Block, AllocatorOrContainer> at either end
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;
}
```

`<xstd/bits.hpp>` and `<xstd/bits/ext/boost.hpp>` do not include this header, so the dependency on
[Boost.DynamicBitset](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/) is opted into by name. Including
it is what makes the conversions valid; it declares no name of its own and exports `bit_convert`.
