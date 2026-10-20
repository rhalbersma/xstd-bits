# `<xstd/bits/bit_set.hpp>`

```cpp
namespace xstd {
  // [set], class template set
  template<class Key, unsigned_integer Block, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_set;
  using bit_set = basic_bit_set<size_t, size_t>;

  // the fixed-width and inplace-capacity sets
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>>
      requires see below
    class basic_bit_fixed_set;
  template<size_t N>
    using bit_fixed_set = basic_bit_fixed_set<size_t, size_t, N>;
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>>
      requires see below
    class basic_bit_inplace_set;
  template<size_t N>
    using bit_inplace_set = basic_bit_inplace_set<size_t, size_t, N>;

  // the set over borrowed blocks
  template<bit_blocks Blocks, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_set_view;

  // the key mappings
  template<class Enum>
    struct enum_traits {};
  template<class Key, Key First, size_t N>
      requires see below
    struct bit_range_mapping;
  template<class Key, const auto& Keys>
      requires see below
    struct bit_find_mapping;
  template<class Key>
    struct bit_key_mapping;
  template<class Key, size_t N = see below>
      requires see below
    struct bit_flag_mapping;

  // the fixed sets an enumeration or a bitmask type names
  template<class Enum>
      requires is_enum_v<Enum> and sized_bit_index_mapping<bit_key_mapping<Enum>, Enum>
    using bit_enum_set =
      bit_least<basic_bit_fixed_set<Enum, size_t, bit_key_mapping<Enum>::size, bit_key_mapping<Enum>>>;
  template<class Mask, size_t N = see below>
      requires see below
    using bit_flag_set =
      bit_least<basic_bit_fixed_set<Mask, size_t, N, bit_flag_mapping<Mask, N>, greater<Mask>>>;

  // [set.erasure], erasure
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

namespace std {
  template<class Key, class Block, class KeyMapping, class Compare, class Allocator>
    struct hash<xstd::basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct hash<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct hash<xstd::basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>>;
  template<class Blocks, size_t N>
    struct hash<xstd::bit_set_view<Blocks, N>>;
}

namespace boost::container_hash {
  template<class Key, class Block, class KeyMapping, class Compare, class Allocator>
    struct is_range<xstd::basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>> : false_type {};
  template<class Key, class Block, class KeyMapping, class Compare, class Allocator>
    struct is_tuple_like<xstd::basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_range<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_tuple_like<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_range<xstd::basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_tuple_like<xstd::basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Blocks, size_t N>
    struct is_range<xstd::bit_set_view<Blocks, N>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| `<xstd/bits/bit_set/bit_set.hpp>` | `basic_bit_set`, `bit_set` |
| `<xstd/bits/bit_set/bit_fixed_set.hpp>` | `basic_bit_fixed_set`, `bit_fixed_set` |
| `<xstd/bits/bit_set/bit_inplace_set.hpp>` | `basic_bit_inplace_set`, `bit_inplace_set` |
| `<xstd/bits/bit_set/bit_set_view.hpp>` | `bit_set_view` |
| `<xstd/bits/bit_set/bit_key_mapping.hpp>` | `bit_find_mapping`, `bit_key_mapping`, `bit_range_mapping`, `enum_traits` |
| `<xstd/bits/bit_set/bit_flag_mapping.hpp>` | `bit_flag_mapping` |
| `<xstd/bits/bit_set/bit_enum_set.hpp>` | `bit_enum_set` |
| `<xstd/bits/bit_set/bit_flag_set.hpp>` | `bit_flag_set` |
| `<xstd/bits/bit_set/erase_if.hpp>` | `erase_if` |
| `<xstd/bits/bit_set/hash.hpp>` | `hash` |
| `<xstd/bits/bit_set/container_hash.hpp>` | `is_range`, `is_tuple_like` |
