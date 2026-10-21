# `<xstd/bits/bit_inplace_vector.hpp>`

```cpp
namespace xstd {
  // [inplace.vector], class template inplace_vector
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_inplace_vector;
  template<size_t N>
    using bit_inplace_vector = basic_bit_inplace_vector<size_t, N>;

  // [inplace.vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

#define XSTD_BITS_HAS_CONSTEXPR_INPLACE 1                                     // see below

namespace std {
  template<class Block, size_t N>
    struct hash<xstd::basic_bit_inplace_vector<Block, N>>;
}

namespace boost::container_hash {
  template<class Block, size_t N>
    struct is_range<xstd::basic_bit_inplace_vector<Block, N>> : false_type {};
  template<class Block, size_t N>
    struct is_tuple_like<xstd::basic_bit_inplace_vector<Block, N>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_inplace_vector/bit_inplace_vector.hpp>`](bit_inplace_vector.md) | `basic_bit_inplace_vector`, `bit_inplace_vector` |
| [`<xstd/bits/bit_inplace_vector/erase_if.hpp>`](erase_if.md) | `erase`, `erase_if` |
| [`<xstd/bits/bit_inplace_vector/hash.hpp>`](hash.md) | `hash` |
| [`<xstd/bits/bit_inplace_vector/container_hash.hpp>`](hash.md) | `is_range`, `is_tuple_like` |
