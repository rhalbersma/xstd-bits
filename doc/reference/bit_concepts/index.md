# `<xstd/bits/bit_concepts.hpp>`

```cpp
namespace xstd {
  // what holds the bits: one block, or a sized contiguous range of them
  template<class Bits>
    concept bit_blocks = see below;

  // what a set owner asks of the mapping that places its keys in bits
  template<class Mapping, class Key>
    concept bit_index_mapping = see below;
  template<class Mapping, class Key>
    concept sized_bit_index_mapping = see below;
  template<class Mapping, class Key>
    concept bit_mask_mapping = see below;

  // blocks an owner takes as they are, and positions converted between any two widths
  template<class To, class Blocks>
    concept bit_constructible_from = see below;
  template<class From, class To>
    concept bit_convertible_to = see below;
}
```

| Header | Declares |
| :----- | :------- |
| `<xstd/bits/bit_concepts/bit_blocks.hpp>` | `bit_blocks` |
| `<xstd/bits/bit_concepts/bit_index_mapping.hpp>` | `bit_index_mapping` |
| `<xstd/bits/bit_concepts/sized_bit_index_mapping.hpp>` | `sized_bit_index_mapping` |
| `<xstd/bits/bit_concepts/bit_mask_mapping.hpp>` | `bit_mask_mapping` |
| `<xstd/bits/bit_concepts/bit_constructible_from.hpp>` | `bit_constructible_from` |
| `<xstd/bits/bit_concepts/bit_convertible_to.hpp>` | `bit_convertible_to` |

The umbrella exports each header below, in the three groups its comments mark. A concept whose definition names an
exposition-only concept of the library's `detail` namespace writes `see below` there, and the bullets under it say
what that concept asks. The three mapping concepts are cv-transparent in `Key`, as xstd-ints's concepts are: a
cv-qualified key is asked about as the bare one.
