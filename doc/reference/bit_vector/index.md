# `<xstd/bits/bit_vector.hpp>`

```cpp
namespace xstd {
  // [vector.bool.pspc], partial class template specialization vector<bool, Allocator>
  template<unsigned_integer Block, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_vector;
  using bit_vector = basic_bit_vector<size_t>;

  // [vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

namespace std {
  template<class Block, class Allocator>
    struct hash<xstd::basic_bit_vector<Block, Allocator>>;
}

namespace boost::container_hash {
  template<class Block, class Allocator>
    struct is_range<xstd::basic_bit_vector<Block, Allocator>> : false_type {};
  template<class Block, class Allocator>
    struct is_tuple_like<xstd::basic_bit_vector<Block, Allocator>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| `<xstd/bits/bit_vector/bit_vector.hpp>` | `basic_bit_vector`, `bit_vector` |
| `<xstd/bits/bit_vector/erase_if.hpp>` | `erase`, `erase_if` |
| `<xstd/bits/bit_vector/hash.hpp>` | `hash` |
| `<xstd/bits/bit_vector/container_hash.hpp>` | `is_range`, `is_tuple_like` |
