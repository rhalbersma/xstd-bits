# `<xstd/bits/bit_type_traits.hpp>`

```cpp
namespace xstd {
  // the width a storage type names, and how a storage rebinds
  template<bit_blocks Bits>
    constexpr size_t bit_blocks_extent_v = see below;
  template<class Blocks>
    struct bit_storage_rebind;

  // the pair that rebinds an owner's block or width
  template<unsigned_integer Block, class Owner>
      requires see below
    using bit_rebind = see below;
  template<size_t N, class Owner>
      requires see below
    using bit_resize = see below;

  // the choices made through them
  template<size_t N>
    using least_block_t = see below;
  template<class Owner>
      requires see below
    using bit_least = bit_rebind<least_block_t<see below>, Owner>;
  template<size_t N>
    using fast_block_t = see below;
  template<class Owner>
      requires see below
    using bit_fast = bit_rebind<fast_block_t<see below>, Owner>;
  template<class Owner>
      requires see below
    using bit_align = bit_resize<see below, Owner>;
  template<class Key>
      requires see below
    using underlying_block_t = see below;
  template<class Owner>
      requires see below
    using bit_underlying = bit_rebind<underlying_block_t<typename Owner::key_type>, Owner>;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_type_traits/bit_blocks_extent.hpp>`](bit_blocks_extent.md) | `bit_blocks_extent_v` |
| [`<xstd/bits/bit_type_traits/bit_storage_rebind.hpp>`](bit_storage_rebind.md) | `bit_storage_rebind` |
| [`<xstd/bits/bit_type_traits/bit_rebind.hpp>`](bit_rebind.md) | `bit_rebind` |
| [`<xstd/bits/bit_type_traits/bit_resize.hpp>`](bit_resize.md) | `bit_resize` |
| [`<xstd/bits/bit_type_traits/bit_least.hpp>`](bit_least.md) | `bit_least`, `least_block_t` |
| [`<xstd/bits/bit_type_traits/bit_fast.hpp>`](bit_fast.md) | `bit_fast`, `fast_block_t` |
| [`<xstd/bits/bit_type_traits/bit_align.hpp>`](bit_align.md) | `bit_align` |
| [`<xstd/bits/bit_type_traits/bit_underlying.hpp>`](bit_underlying.md) | `bit_underlying`, `underlying_block_t` |

The width a storage type names, the two transformations that rebind an owner's block or width, then the choices
made through them, in the umbrella's order. Each transformation is an alias template and is specified as
[[meta.trans]](https://eel.is/c++draft/meta.trans) specifies one: its *Constraints*, then the type it names.
Two phrases name what the constraints admit:

- A *rebindable owner* is an owner of ours that names its block: a specialization of `basic_bit_array`,
  `basic_bit_fixed_set` or `basic_bit_fixed_valarray`, or of `basic_bit_vector`, `basic_bit_set` or
  `basic_bit_valarray` over a storage with a `bit_storage_rebind`, the inplace and small aliases and the short names
  included.
- A *resizable owner* is a rebindable owner whose type carries its width: a fixed width, a cap `N`, or a storage whose
  `bit_storage_rebind` counts its blocks, the width then the bits they hold. For an inplace owner the width is its
  capacity, and for a small one the bits its inline blocks hold.

Every transformation is cv-transparent, as `make_unsigned` is: an `Owner` that is cv-qualified is transformed as
`remove_cv_t<Owner>` is, and the type named carries `Owner`'s cv. A view, `bitset`, a block and a range of blocks
own no type to transform and are turned away by the constraint, not deep inside the alias.
