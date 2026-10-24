# `<xstd/bits/bit_array/bit_array.hpp>`

## `basic_bit_array`, `bit_array`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_array {
public:
  // types
  using value_type             = bool;
  using pointer                = void;
  using const_pointer          = void;
  using reference              = see below;
  using const_reference        = see below;
  using size_type              = size_t;
  using difference_type        = ptrdiff_t;
  using iterator               = see below;
  using const_iterator         = see below;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using block_container_type   = array<Block, see below>;

  // construct
  constexpr basic_bit_array() noexcept;
  constexpr basic_bit_array(initializer_list<bool> il);
  template<size_t M>
      requires (M == N)
    constexpr explicit basic_bit_array(const bool (&a)[M]) noexcept;
  template<class Bits>
      requires see below
    constexpr basic_bit_array(from_blocks_t, const Bits& b) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> size     = {};
  static constexpr bool_constant<N == 0>        empty    = {};
  static constexpr integral_constant<size_t, N> max_size = {};

  friend constexpr auto swap(basic_bit_array& x, basic_bit_array& y) noexcept(noexcept(x.swap(y))) -> void;
};

template<unsigned_integer Block>
  basic_bit_array(from_blocks_t, Block) -> basic_bit_array<Block, bit_blocks_extent_v<Block>>;
template<unsigned_integer Block, size_t K>
  basic_bit_array(from_blocks_t, array<Block, K>) -> basic_bit_array<Block, bit_blocks_extent_v<array<Block, K>>>;
template<unsigned_integer Block, size_t K>
  basic_bit_array(from_blocks_t, const Block (&)[K]) -> basic_bit_array<Block, bit_blocks_extent_v<array<Block, K>>>;
```

As [[array]](https://eel.is/c++draft/array) for `array<bool, N>`, with:

- `bit_array<N>` holds `N` bits in `size_t` blocks, and `basic_bit_array<Block, N>` in `Block`s, `ceil(N / D)` of
  them, none at `N == 0`. The requires-clause admits a cv-unqualified `Block` whose digits are a power of two and at
  least `unsigned char`'s, at any `N`, and any other cv-unqualified `Block` where `N` fits in one of it.
- **A proxy reference.** A bit has no address, so `reference` is a proxy class, `pointer` and `const_pointer` are
  `void`, `data()` is absent, and `iterator` is random access rather than contiguous. `const_reference` is the same
  proxy over const storage, and reads without assigning.
- The class is no aggregate. Its constructors are what `array`'s initialization does:
    - The default constructor makes every position `false`. `array<bool, N> a; a[0] = true;` is valid, and writing a
      packed bit reads the block that holds it, so the blocks start clear rather than default-initialized; reading a
      position before writing it, which `array` leaves undefined, answers `false`.
    - The `initializer_list` constructor sets the leading positions to the listed values and leaves the rest `false`.
      *Preconditions:* `il.size() <= N`.
    - The built-in array constructor copies `a` position by position, and only an array of `N` elements converts.
- *`from_blocks` constructor:* *Constraints:* `Bits` models `bit_blocks`, and its bits have a layout this width
  reads: a block, a `std::array` or a built-in array of blocks holding at least `N` bits, or, on a little-endian
  target, a trivially copyable object with room for `N` bits, read as `bit_cast` reads it. *Effects:* Position `i`
  takes bit `i` of `b`, for every `i < N`. The guides deduce the width the blocks name.
- *Capacity:* `size`, `empty` and `max_size` are constants of the type. `a.size()` and `A::size()` call
  `integral_constant`'s `operator()`, so each answers a `size_type`, `noexcept`, in a constant expression;
  `A::size` and `a.size` name the constant itself, and `auto n = A::size;` deduces the `integral_constant`.
- `fill`, `swap`, `==` and `<=>` are as [[array]](https://eel.is/c++draft/array); `<=>` answers `strong_ordering`,
  the order `lexicographical_compare_three_way` gives the two sequences of bools, and is `noexcept`.
- It is a structural type exactly where `N` fills its blocks, at `N == 0` included, as `bit_align<bit_array<N>>`
  does at any `N`. At another width the last block's unused bits must stay clear, so the blocks are not public.
- Every member is usable in a constant expression, as [P3372R3](https://wg21.link/P3372R3) makes the standard
  containers' members.
- *Out of range:* `at(n)` throws `out_of_range` where `n >= size()`; `operator[]`, `front` and `back` have
  [[sequence.reqmts]](https://eel.is/c++draft/sequence.reqmts)'s preconditions, checked by an `assert`.

**`reference`, shared by every owner and view of the sequence reading.**

```cpp
class reference {
public:
  using value_type = bool;
  using iterator   = see below;

  reference(const reference&) = default;

  constexpr auto operator&() const noexcept -> iterator;
  constexpr operator bool() const noexcept;
  constexpr auto operator=(bool x) const noexcept -> const reference&;
  constexpr auto operator=(const reference& x) const noexcept -> const reference&;

  friend constexpr auto swap(reference x, reference y) noexcept -> void;
  friend constexpr auto swap(reference x, bool& y) noexcept -> void;
  friend constexpr auto swap(bool& x, reference y) noexcept -> void;
};
```

As `vector<bool>::reference` in [[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc), with:

- The assignments are `const`-qualified and return `const reference&`, the proxy shape of
  [P2321R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2321r2.html). Each writes the bit, never
  rebinds the proxy, and exists only where the storage is not const.
- The three `swap`s are hidden friends, after
  [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html). There is no static
  `swap(reference, reference)` on any container, the form
  [[depr.vector.bool.swap]](https://eel.is/c++draft/depr.vector.bool.swap) deprecates.
- There is no `flip()`: `v[i] = not v[i]` negates one element, and the valarray reading's `!` negates them all.
- `&ref` answers an iterator, so `&*it == it` holds in iterator arithmetic. `bool` is the one conversion, so
  comparisons are the built-in ones through it.
- `formatter<reference, charT>` formats it as `bool`, as [[vector.bool.fmt]](https://eel.is/c++draft/vector.bool.fmt)
  formats `vector<bool>::reference`, and a sequence formats as a range, in brackets.

**`iterator`, shared by every owner and view of the sequence reading.**

- `iterator_category` is `random_access_iterator_tag`, `value_type` is `bool`, `difference_type` is `ptrdiff_t`,
  `pointer` is `void`, and `reference` is the proxy. A mutable iterator converts implicitly to `const_iterator`.
- `iter_move(it)` answers the `bool`, and `iter_swap` exchanges the two bits, so the `ranges` algorithms run over
  the proxies unchanged.
- *Preconditions:* Two iterators compared or subtracted refer to the same storage.
- *Remarks:* An iterator is the storage object and a position, not a pointer into the blocks, so growth that
  reallocates the blocks invalidates no iterator.
