# `<xstd/bits/bit_vector.hpp>`

```cpp
namespace xstd {
  // [vector.bool.pspc], partial class template specialization vector<bool, Allocator>
  template<bit_storage Blocks = vector<size_t>, size_t N = dynamic_extent>
      requires see below
    class basic_bit_vector;
  using bit_vector = basic_bit_vector<>;

  // [vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

namespace std {
  template<class Blocks, size_t N>
    struct hash<xstd::basic_bit_vector<Blocks, N>>;
}

namespace boost::container_hash {
  template<class Blocks, size_t N>
    struct is_range<xstd::basic_bit_vector<Blocks, N>> : false_type {};
  template<class Blocks, size_t N>
    struct is_tuple_like<xstd::basic_bit_vector<Blocks, N>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_vector/bit_vector.hpp>`](bit_vector.md) | `basic_bit_vector`, `bit_vector` |
| [`<xstd/bits/bit_vector/erase_if.hpp>`](erase_if.md) | `erase`, `erase_if` |
| [`<xstd/bits/bit_vector/hash.hpp>`](hash.md) | `hash` |
| [`<xstd/bits/bit_vector/container_hash.hpp>`](hash.md) | `is_range`, `is_tuple_like` |
