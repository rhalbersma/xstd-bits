# `basic_bit_valarray`, `bit_valarray`

```cpp
template<unsigned_integer Block, class Allocator>
  requires see below
class basic_bit_valarray {
public:
  using value_type           = bool;
  using reference            = see below;
  using size_type            = size_t;
  using block_container_type = vector<Block, Allocator>;

  // [valarray.cons]
  basic_bit_valarray() = default;
  constexpr explicit basic_bit_valarray(size_t n);
  constexpr basic_bit_valarray(const bool& value, size_t n);
  constexpr basic_bit_valarray(const bool* p, size_type n);
  constexpr basic_bit_valarray(initializer_list<bool> il);
  constexpr basic_bit_valarray(const bit_slice_array<Block>& a);
  constexpr basic_bit_valarray(const bit_gslice_array<Block>& a);
  constexpr basic_bit_valarray(const bit_mask_array<Block>& a);
  constexpr basic_bit_valarray(const bit_indirect_array<Block>& a);
  constexpr basic_bit_valarray(from_blocks_t, block_container_type blocks) noexcept(see below);

  // [valarray.assign]
  constexpr auto operator=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator=(initializer_list<bool> il) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_slice_array<Block>& a) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_gslice_array<Block>& a) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_mask_array<Block>& a) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_indirect_array<Block>& a) -> basic_bit_valarray&;

  // [valarray.access]
  constexpr auto operator[](size_type n) noexcept -> reference;
  constexpr auto operator[](size_type n) const noexcept -> bool;

  // [valarray.sub]
  auto operator[](slice s) const;
  auto operator[](slice s) -> bit_slice_array<Block>;
  constexpr auto operator[](bit_slice s) const;
  constexpr auto operator[](bit_slice s) -> bit_slice_array<Block>;
  auto operator[](const gslice& s) const;
  auto operator[](const gslice& s) -> bit_gslice_array<Block>;
  auto operator[](const bit_gslice& s) const;
  auto operator[](const bit_gslice& s) -> bit_gslice_array<Block>;
  template<class Mask>
      requires see below
    constexpr auto operator[](const Mask& mask) const;
  template<class Mask>
      requires see below
    constexpr auto operator[](const Mask& mask) -> bit_mask_array<Block>;
  auto operator[](const valarray<size_t>& indices) const;
  auto operator[](const valarray<size_t>& indices) -> bit_indirect_array<Block>;

  // [valarray.unary]
  constexpr auto operator!() const -> basic_bit_valarray;
  constexpr auto operator~() const -> basic_bit_valarray;

  // [valarray.cassign]
  constexpr auto operator&=(const basic_bit_valarray& other) noexcept -> basic_bit_valarray&;
  constexpr auto operator|=(const basic_bit_valarray& other) noexcept -> basic_bit_valarray&;
  constexpr auto operator^=(const basic_bit_valarray& other) noexcept -> basic_bit_valarray&;
  constexpr auto operator&=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator|=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator^=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator<<=(size_type n) noexcept -> basic_bit_valarray&;
  constexpr auto operator>>=(size_type n) noexcept -> basic_bit_valarray&;

  // [valarray.members]
  constexpr auto swap(basic_bit_valarray& other) noexcept(see below) -> void;
  constexpr auto size() const noexcept -> size_type;
  constexpr auto sum() const noexcept -> bool;
  constexpr auto min() const noexcept -> bool;
  constexpr auto max() const noexcept -> bool;
  constexpr auto shift(int n) const -> basic_bit_valarray;
  constexpr auto cshift(int n) const -> basic_bit_valarray;
  constexpr auto apply(bool func(bool)) const -> basic_bit_valarray;
  constexpr auto apply(bool func(const bool&)) const -> basic_bit_valarray;
  constexpr auto resize(size_type n, bool value = false) -> void;

  // [valarray.binary], [valarray.comparison]: for each @ of & | ^ && || == != < > <= >=
  friend constexpr auto operator@(const basic_bit_valarray& x, const basic_bit_valarray& y) -> basic_bit_valarray;
  friend constexpr auto operator@(const basic_bit_valarray& x, const bool& y) -> basic_bit_valarray;
  friend constexpr auto operator@(const bool& x, const basic_bit_valarray& y) -> basic_bit_valarray;
  friend constexpr auto operator<<(const basic_bit_valarray& x, size_type n) -> basic_bit_valarray;
  friend constexpr auto operator>>(const basic_bit_valarray& x, size_type n) -> basic_bit_valarray;

  // [valarray.special], [valarray.range]
  friend constexpr auto swap(basic_bit_valarray& x, basic_bit_valarray& y) noexcept(see below) -> void;
  friend constexpr auto begin(basic_bit_valarray& v) noexcept;
  friend constexpr auto begin(const basic_bit_valarray& v) noexcept;
  friend constexpr auto end(basic_bit_valarray& v) noexcept;
  friend constexpr auto end(const basic_bit_valarray& v) noexcept;
};
```

As [[template.valarray]](https://eel.is/c++draft/template.valarray) for `valarray<bool>`, with:

- `bit_valarray` holds its bools in `size_t` blocks, and `basic_bit_valarray<Block, Allocator>` in `Block`s, in a
  `vector<Block, Allocator>`. The requires-clause admits `Block` as `basic_bit_vector`'s does. There is no
  allocator argument or `get_allocator`.
- `reference` is the sequence reading's proxy, standing in for `T&`; the `const` subscript returns a `bool`. The
  iterators `begin` and `end` return are the sequence reading's, so it is a random-access range with no member
  `begin` or `end`, and `bit_span` views it as `span<bool>` views a `valarray<bool>`.
- The constructor from a value and a count takes the value first, as
  [[valarray.cons]](https://eel.is/c++draft/valarray.cons) orders them. The constructors from a proxy are
  implicit. `resize(n, value)` assigns `value` to every element, as
  [[valarray.members]](https://eel.is/c++draft/valarray.members) says. A size past what `difference_type` can count
  throws `length_error` before anything is allocated.
- **`~` is logical not**, the same as `!`, where `valarray<bool>::operator~` promotes and answers all `true`.
- **The shifts are `std::bitset`'s.** `v <<= n` moves position `i` to `i + n` and `v >>= n` position `i + n` to `i`,
  zeros filling in, each emptying the array where `n >= size()`; `<<` and `>>` return the copy. `shift(n)` keeps
  [[valarray.members]](https://eel.is/c++draft/valarray.members)'s sense, element `i` taking element `i + n`, so
  `v << n` is `v.shift(-n)`. `cshift(n)` takes element `(i + n) mod size()`, and turns an empty array not at all.
- **Empty reductions are defined.** `sum()` and `max()` are the disjunction of the elements, `false` for an empty
  array, and `min()` their conjunction, `true` for an empty array.
- `apply(func)` calls `func(false)` and `func(true)` once each, and applies the answer a block at a time: a constant,
  the identity or the negation.
- **No arithmetic.** `+`, `-`, `*`, `/`, `%`, unary `+` and `-`, their compound forms and the `<cmath>` functions are
  absent.
- **Comparisons answer arrays.** `==`, `!=`, `<`, `>`, `<=` and `>=` return the array of elementwise answers, so the
  type is not `equality_comparable`, and has no `<=>` and no `hash`, as `valarray` has none. `&&` and `||`
  evaluate both operands, and are `&` and `|`.
- *Preconditions:* The two arrays of a binary or compound operator have one size; each subscript's positions lie
  inside the array, and a mask has the array's size; an index array names no position twice for a write. Each is
  checked by an `assert`.
- **The subscripts.** On a `const` array each of the four gathers into a valarray of its own; on a non-const one it
  returns the proxy over the same block type, which refers to the array. A mask is any owner of the valarray reading
  over `Block` of the array's size, held by reference. `v[slice]` forwards to `v[bit_slice]` and `v[gslice]` to
  `v[bit_gslice]`. A slice or a mask gathers and scatters a block at a time, through
  [P3104](https://wg21.link/P3104)'s `bit_compress` and `bit_expand`.
- *`from_blocks` constructor:* As `basic_bit_vector`'s. A valarray has no `replace` or `extract`.
- Each member that returns a valarray is `noexcept` where copying the blocks is, and the `from_blocks` constructor
  where moving them is.
- Every member but the subscripts by `slice`, `gslice`, `bit_gslice` and `valarray<size_t>` is `constexpr`; the
  heap array is usable in a constant expression as `vector` is.
