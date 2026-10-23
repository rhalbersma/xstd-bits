# `<xstd/bits/bit_valarray.hpp>`

```cpp
namespace xstd {
  // [template.valarray], class template valarray, beside it the fixed and inplace columns
  template<bit_storage Blocks = vector<size_t>, size_t N = dynamic_extent>
      requires see below
    class basic_bit_valarray;
  using bit_valarray = basic_bit_valarray<>;
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_fixed_valarray;
  template<size_t N>
    using bit_fixed_valarray = basic_bit_fixed_valarray<size_t, N>;
  template<unsigned_integer Block, size_t N>
      requires see below
    using basic_bit_inplace_valarray = basic_bit_valarray<see below, N>;
  template<size_t N>
    using bit_inplace_valarray = basic_bit_inplace_valarray<size_t, N>;

  // [class.slice], class slice
  class bit_slice;
  // [template.slice.array], class template slice_array
  template<unsigned_integer Block>
    class bit_slice_array;
  // [class.gslice], class gslice
  class bit_gslice;
  // [template.gslice.array], class template gslice_array
  template<unsigned_integer Block>
    class bit_gslice_array;
  // [template.mask.array], class template mask_array
  template<unsigned_integer Block>
    class bit_mask_array;
  // [template.indirect.array], class template indirect_array
  template<unsigned_integer Block>
    class bit_indirect_array;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_valarray/bit_valarray.hpp>`](bit_valarray.md) | `basic_bit_valarray`, `bit_valarray` |
| [`<xstd/bits/bit_valarray/bit_fixed_valarray.hpp>`](bit_fixed_valarray.md) | `basic_bit_fixed_valarray`, `bit_fixed_valarray` |
| [`<xstd/bits/bit_valarray/bit_inplace_valarray.hpp>`](bit_inplace_valarray.md) | `basic_bit_inplace_valarray`, `bit_inplace_valarray` |
| [`<xstd/bits/bit_valarray/bit_slice.hpp>`](bit_slice.md) | `bit_slice` |
| [`<xstd/bits/bit_valarray/bit_slice_array.hpp>`](bit_slice_array.md) | `bit_slice_array` |
| [`<xstd/bits/bit_valarray/bit_gslice.hpp>`](bit_gslice.md) | `bit_gslice` |
| [`<xstd/bits/bit_valarray/bit_gslice_array.hpp>`](bit_slice_array.md) | `bit_gslice_array` |
| [`<xstd/bits/bit_valarray/bit_mask_array.hpp>`](bit_slice_array.md) | `bit_mask_array` |
| [`<xstd/bits/bit_valarray/bit_indirect_array.hpp>`](bit_slice_array.md) | `bit_indirect_array` |
