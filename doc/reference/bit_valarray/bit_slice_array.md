# `<xstd/bits/bit_valarray/bit_slice_array.hpp>`, `<xstd/bits/bit_valarray/bit_gslice_array.hpp>`, `<xstd/bits/bit_valarray/bit_mask_array.hpp>`, `<xstd/bits/bit_valarray/bit_indirect_array.hpp>`

## `bit_slice_array`, `bit_gslice_array`, `bit_mask_array`, `bit_indirect_array`

```cpp
template<unsigned_integer Block>
class bit_slice_array {
public:
  using value_type = bool;

  bit_slice_array() = delete;
  bit_slice_array(const bit_slice_array&) = default;
  ~bit_slice_array() = default;

  constexpr auto operator=(const bit_slice_array& other) const -> const bit_slice_array&;
  template<class A>
      requires see below
    constexpr auto operator=(const A& a) const -> void;
  template<class A>
      requires see below
    constexpr auto operator&=(const A& a) const -> void;
  template<class A>
      requires see below
    constexpr auto operator|=(const A& a) const -> void;
  template<class A>
      requires see below
    constexpr auto operator^=(const A& a) const -> void;
  constexpr auto operator=(const bool& value) const -> void;
  constexpr auto operator&=(const bool& value) const -> void;
  constexpr auto operator|=(const bool& value) const -> void;
  constexpr auto operator^=(const bool& value) const -> void;
};
```

As [[template.slice.array]](https://eel.is/c++draft/template.slice.array),
[[template.gslice.array]](https://eel.is/c++draft/template.gslice.array),
[[template.mask.array]](https://eel.is/c++draft/template.mask.array) and
[[template.indirect.array]](https://eel.is/c++draft/template.indirect.array) at `T = bool`. The four have the members
shown for `bit_slice_array`, with:

- Only a non-const subscript of a valarray over `Block` makes one. It refers to the array's blocks, and a copy refers
  to the same elements; it must not outlive the array, or a mask or index array it was made from.
- Assigning one proxy to another assigns the elements, never the reference.
- *Constraints:* `A` is an owner of the valarray reading over `Block`, of any storage, or one of the four proxies
  over `Block`. *Preconditions:* `a`'s size is the number of positions selected.
- *Effects:* The elements of `a`, in order, are written to the selected positions, or combined with them by `&`,
  `|` or `^`. A proxy `a` gathers what it selects before any position is written, so `v[s] = v[t]` reads `v` first.
  A `bool` is written to, or combined with, every selected position.
- The compound assignments from a `bool` are additions; the arithmetic compound assignments and the shifts are
  absent.
- A slice of stride zero and a gslice that repeats a position write one position at a time, in order, so the last
  write to a position stands.
