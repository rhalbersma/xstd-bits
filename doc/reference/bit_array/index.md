# `<xstd/bits/bit_array.hpp>`

```cpp
namespace xstd {
  // [array], class template array
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_array;
  template<size_t N>
    using bit_array = basic_bit_array<size_t, N>;

  // [array.creation], array creation functions
  template<class T, size_t N>
    constexpr auto to_bit_array(T (&a)[N]) -> bit_array<N>;
  template<class T, size_t N>
    constexpr auto to_bit_array(T (&&a)[N]) -> bit_array<N>;
}

namespace std {
  // [array.tuple], tuple interface
  template<size_t I, class Block, size_t N>
    struct tuple_element<I, xstd::basic_bit_array<Block, N>>;
  template<size_t I, class Block, size_t N>
    struct tuple_element<I, const xstd::basic_bit_array<Block, N>>;
  template<class Block, size_t N>
    struct tuple_size<xstd::basic_bit_array<Block, N>>;

  // hash support
  template<class Block, size_t N>
    struct hash<xstd::basic_bit_array<Block, N>>;
}

namespace boost::container_hash {
  template<class Block, size_t N>
    struct is_range<xstd::basic_bit_array<Block, N>> : false_type {};
  template<class Block, size_t N>
    struct is_tuple_like<xstd::basic_bit_array<Block, N>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_array/bit_array.hpp>`](bit_array.md) | `basic_bit_array`, `bit_array` |
| [`<xstd/bits/bit_array/to_array.hpp>`](to_array.md) | `to_bit_array` |
| [`<xstd/bits/bit_array/tuple_element.hpp>`](tuple_element.md) | `tuple_element`, `tuple_size` |
| [`<xstd/bits/bit_array/hash.hpp>`](hash.md) | `hash` |
| [`<xstd/bits/bit_array/container_hash.hpp>`](hash.md) | `is_range`, `is_tuple_like` |
