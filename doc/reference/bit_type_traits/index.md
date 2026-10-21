# `<xstd/bits/bit_type_traits.hpp>`

```cpp
namespace xstd {
  // the width a storage type names
  template<bit_blocks Bits>
    constexpr size_t bit_blocks_extent_v = see below;

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
  `basic_bit_inplace_vector`, `basic_bit_vector`, `basic_bit_set`, `basic_bit_fixed_set`, `basic_bit_inplace_set`,
  `basic_bit_valarray`, `basic_bit_fixed_valarray`, `basic_bit_inplace_valarray`, or of the Boost extension's
  `basic_bit_small_vector`, `basic_bit_small_set` and `basic_bit_small_valarray`, the short names included.
- A *resizable owner* is a rebindable owner whose type carries its width `N`: every one but `basic_bit_vector`,
  `basic_bit_set` and `basic_bit_valarray`. For an inplace or a small owner `N` is its capacity.

Every transformation is cv-transparent, as `make_unsigned` is: an `Owner` that is cv-qualified is transformed as
`remove_cv_t<Owner>` is, and the type named carries `Owner`'s cv. A view, `bitset`, a block and a range of blocks
own no type to transform and are turned away by the constraint, not deep inside the alias.
