# `<xstd/bits/bit_type_traits/bit_storage_rebind.hpp>`

## `bit_storage_rebind`

```cpp
template<class Blocks>
struct bit_storage_rebind {};

template<template<class, class> class Storage, class Block, class Allocator>
  requires simple_allocator<Allocator>
struct bit_storage_rebind<Storage<Block, Allocator>> {
  template<class OtherBlock>
    using with_block = Storage<OtherBlock, typename allocator_traits<Allocator>::template rebind_alloc<OtherBlock>>;
};

template<template<class, size_t> class Storage, class Block, size_t K>
struct bit_storage_rebind<Storage<Block, K>> {
  static constexpr size_t block_count = K;

  template<class OtherBlock>
    using with_block = Storage<OtherBlock, K>;
  template<size_t L>
    using with_block_count = Storage<Block, L>;
};
```

How a [`bit_storage`](../bit_concepts/bit_storage.md) changes its block, and, where its type carries a count of
blocks, that count. `bit_rebind`, `bit_resize`, `bit_least`, `bit_fast`, `bit_align` and `bit_underlying` reach an
owner's storage through it.

- *`with_block<OtherBlock>`:* The same storage over `OtherBlock`. A storage over an allocator takes the allocator
  along, rebound as `allocator_traits` rebinds it: `vector<uint64_t>` becomes `vector<uint8_t>`, and
  `pmr::vector<uint64_t>` becomes `pmr::vector<uint8_t>`.
- *`block_count`, `with_block_count<L>`:* The count of blocks in the type, and the same storage over `L` of them:
  `inplace_vector<uint64_t, 2>` and `boost::container::small_vector<uint16_t, 3>` have them, and a heap of blocks has
  neither. An owner over a counted storage has a width to resize, and another block recounts the blocks that hold it.
- The library specializes it for Boost's `static_vector` and `small_vector`, whose extra parameters the primary forms
  do not match, and for the empty storage `bit_inplace_vector<0>` holds. A storage of another shape is specialized by
  its user, beside the storage, with the same members.
- Without a specialization a storage still holds an owner's blocks; `bit_rebind` and `bit_resize` then name no type
  for it.
