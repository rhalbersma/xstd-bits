# `<xstd/bits/bit_functional.hpp>`

```cpp
namespace xstd {
  // [comparisons], comparisons
  struct bit_equal_to;
  struct bit_not_equal_to;
  struct bit_greater;
  struct bit_less;
  struct bit_greater_equal;
  struct bit_less_equal;

  // [comparisons.three.way], class compare_three_way
  struct bit_compare_three_way;

  // [unord.hash], class template hash, beside it the digest of the same bits
  template<hash_algorithm Hash, class Flavor, class T>
      requires see below
    constexpr auto bit_hash_append(Hash& h, const Flavor& f, const T& x) -> void;
  template<hash_algorithm Hash, class Flavor, class R>
      requires see below
    constexpr auto bit_hash_append(Hash& h, const Flavor& f, const R& r) -> void;
  template<class T, hash_algorithm H = boost::hash2::xxhash_64, class Flavor = boost::hash2::default_flavor>
      requires see below
    class bit_hash;
  template<class T, hash_algorithm H = boost::hash2::xxh3_128, class Flavor = boost::hash2::little_endian_flavor>
      requires see below
    class bit_digest;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_functional/bit_equal_to.hpp>`](bit_equal_to.md) | `bit_equal_to` |
| [`<xstd/bits/bit_functional/bit_not_equal_to.hpp>`](bit_equal_to.md) | `bit_not_equal_to` |
| [`<xstd/bits/bit_functional/bit_greater.hpp>`](bit_greater.md) | `bit_greater` |
| [`<xstd/bits/bit_functional/bit_less.hpp>`](bit_greater.md) | `bit_less` |
| [`<xstd/bits/bit_functional/bit_greater_equal.hpp>`](bit_greater.md) | `bit_greater_equal` |
| [`<xstd/bits/bit_functional/bit_less_equal.hpp>`](bit_greater.md) | `bit_less_equal` |
| [`<xstd/bits/bit_functional/bit_compare_three_way.hpp>`](bit_compare_three_way.md) | `bit_compare_three_way` |
| [`<xstd/bits/bit_functional/bit_hash.hpp>`](bit_hash.md) | `bit_hash`, `bit_hash_append` |
| [`<xstd/bits/bit_functional/bit_digest.hpp>`](bit_digest.md) | `bit_digest` |

Each comparison and the hash has a header of its own under `<xstd/bits/bit_functional/>`, and the umbrella exports
them in [[comparisons]](https://eel.is/c++draft/comparisons)'s order, then
[[comparisons.three.way]](https://eel.is/c++draft/comparisons.three.way), then
[[unord.hash]](https://eel.is/c++draft/unord.hash) with the digest beside it. `hash_algorithm` is xstd-misc's. The
comparisons are transparent and agree with the hash: what `bit_equal_to` calls equal, `bit_hash` hashes alike, and
the orderings order equivalent exactly what it calls equal. So a `std::unordered_set<K, bit_hash<K>, bit_equal_to>`
or a `std::set<K, bit_less>` finds a key through a view of it or through an owner over narrower blocks.
