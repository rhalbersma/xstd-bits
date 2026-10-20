# Reference

Each public entity of xstd-bits is specified here the way the standard specifies the library: a synopsis per
header, then for each entity the elements of
[[structure.specifications]](https://eel.is/c++draft/structure.specifications) that apply. The library compiles as
[C++23](https://wg21.link/N4950), but its interfaces are measured against the current
[working draft](https://eel.is/c++draft/), and every clause cited here is the draft's. A name with no namespace is in
`std`.

The synopses leave out `[[nodiscard]]`, as the standard's own have since [P2422R1](https://wg21.link/P2422R1), which
made it a recommended practice rather than part of a library's specification. The code carries it on every function
that returns a value, but for a reference returned for chaining, as `operator&=` returns one, and for a by-product a
caller may ignore: postfix `++` and `--`, `insert`, `insert_range`, `emplace`, `emplace_hint`, `erase`, the free
`erase` and `erase_if`, the reference `push_back` and `emplace_back` return, and the `try_` and `unchecked_` doors.
It carries it on every constructor but `basic_bit_array`'s from an `initializer_list`.

This page covers the containers, the views, the algorithms and the iterator operations. The library's own terms
are used throughout:

- A *block* is an unsigned integer holding bits. Position `i` is bit `i % D` of block `i / D`, where `D` is the
  block's `numeric_limits<Block>::digits`.
- The *storage* holds the blocks: *fixed*, the width the type's; *inplace*, a run-time width within a capacity the
  type carries, the blocks held inline; *small*, inline up to a capacity and on the heap past it; or *dynamic*, on
  the heap.
- An *owner* holds its storage, and a *view* refers to storage another object holds.
- A *reading* is what the bits mean: a *sequence* of `bool`, a *set* of keys, or a *valarray*, the sequence read
  elementwise.

Each container answers a standard counterpart's synopsis, and is specified as that counterpart *with* what the
packing changes. Members named `adaptor_type`, `adapted_type`, `derived_type`, `reads_as`, `owns_storage`,
`is_windowed` and `static_extent` answer the library's own traits and are not specified here.

## `<xstd/bits/bit_array.hpp>`

```cpp
namespace xstd {
  // [array], class template array
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_array;
  template<size_t N>
    using bit_array = basic_bit_array<size_t, N>;

  // [array.creation], array creation functions
  template<class T, size_t N>
    constexpr auto to_bit_array(T (&a)[N]) -> bit_array<N>;
  template<class T, size_t N>
    constexpr auto to_bit_array(T (&&a)[N]) -> bit_array<N>;
}

namespace std {
  // [array.tuple], tuple interface
  template<size_t I, class Block, size_t N>
    struct tuple_element<I, xstd::basic_bit_array<Block, N>>;
  template<size_t I, class Block, size_t N>
    struct tuple_element<I, const xstd::basic_bit_array<Block, N>>;
  template<class Block, size_t N>
    struct tuple_size<xstd::basic_bit_array<Block, N>>;

  // hash support
  template<class Block, size_t N>
    struct hash<xstd::basic_bit_array<Block, N>>;
}

namespace boost::container_hash {
  template<class Block, size_t N>
    struct is_range<xstd::basic_bit_array<Block, N>> : false_type {};
  template<class Block, size_t N>
    struct is_tuple_like<xstd::basic_bit_array<Block, N>> : false_type {};
}
```

### `basic_bit_array`, `bit_array`

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
  basic_bit_array() = default;
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
  - The default constructor makes every position `false`.
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

### `to_bit_array`

- *Mandates:* `remove_cv_t<T>` is `bool`.
- *Returns:* `bit_array<N>(a)`, the second form from the moved array. A built-in array names no block type, so the
  result takes the default one.

### `tuple_size`, `tuple_element`, `get`

- As [[array.tuple]](https://eel.is/c++draft/array.tuple), with:
- `tuple_size<basic_bit_array<Block, N>>` is `integral_constant<size_t, N>`.
- `tuple_element<I, basic_bit_array<Block, N>>::type` is the proxy `reference`, and for a `const` array the proxy
  `const_reference`, so a structured binding over the array writes through to it. *Mandates:* `I < N`.
- `get<I>(a)`, for an lvalue, a `const` lvalue, an rvalue and a `const` rvalue, returns `a[I]`. *Constraints:*
  `I < N`. It is found by argument-dependent lookup, not as a member of namespace `xstd`.

### `hash`, `is_range`, `is_tuple_like`

- `hash<basic_bit_array<Block, N>>`: `operator()` is `noexcept` and returns
  `xstd::bit_hash<T, boost::hash2::xxhash_64>()(x)`, the hash of the bits. `array<bool, N>` has no hash; this one
  does, and equal values hash equal.
- A Boost.Hash2 `hash_append` hook appends exactly the message Hash2 writes for the `array<bool, N>` of the same
  value, so `xstd::hash<T, H, Flavor>()(x) == xstd::hash<array<bool, N>, H, Flavor>()(m)` for that `m`.
- `is_range` and `is_tuple_like` are `false_type`, so Boost.ContainerHash and Hash2 take the hook rather than
  hashing the array as a range or as a tuple.

## `<xstd/bits/bit_inplace_vector.hpp>`

```cpp
namespace xstd {
  // [inplace.vector], class template inplace_vector
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_inplace_vector;
  template<size_t N>
    using bit_inplace_vector = basic_bit_inplace_vector<size_t, N>;

  // [inplace.vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

#define XSTD_BITS_HAS_CONSTEXPR_INPLACE 1                                     // see below

namespace std {
  template<class Block, size_t N>
    struct hash<xstd::basic_bit_inplace_vector<Block, N>>;
}

namespace boost::container_hash {
  template<class Block, size_t N>
    struct is_range<xstd::basic_bit_inplace_vector<Block, N>> : false_type {};
  template<class Block, size_t N>
    struct is_tuple_like<xstd::basic_bit_inplace_vector<Block, N>> : false_type {};
}
```

### `basic_bit_inplace_vector`, `bit_inplace_vector`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_inplace_vector {
public:
  // types as basic_bit_array's, with
  using block_container_type = see below;

  // construct: [inplace.vector.cons]'s, with
  constexpr basic_bit_inplace_vector(from_blocks_t, block_container_type blocks) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
  static constexpr integral_constant<size_t, N> capacity = {};
  static constexpr auto reserve(size_type n) -> void;
  static constexpr auto shrink_to_fit() noexcept -> void;

  // block exchange
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  friend constexpr auto swap(basic_bit_inplace_vector& x, basic_bit_inplace_vector& y)
    noexcept(noexcept(x.swap(y))) -> void;
};

template<unsigned_integer Block, size_t K>
  basic_bit_inplace_vector(from_blocks_t, see below) -> basic_bit_inplace_vector<Block, bit_blocks_extent_v<Block> * K>;
```

As [[inplace.vector]](https://eel.is/c++draft/inplace.vector) for `inplace_vector<bool, N>`, with:

- `bit_inplace_vector<N>` holds up to `N` bits in `size_t` blocks, and `basic_bit_inplace_vector<Block, N>` in
  `Block`s. The requires-clause admits a cv-unqualified `Block` whose digits are a power of two and at least
  `unsigned char`'s. `N` is the capacity exactly, even where the last block has room for more.
- `block_container_type` is `inplace_vector<Block, K>` where the standard library has `<inplace_vector>`, and
  `boost::container::static_vector<Block, K>` elsewhere, for `K = ceil(N / D)`. At `K == 0`, where that type is not
  empty, it is an empty type of the library's own, so `bit_inplace_vector<0>` is an empty type, as
  [[inplace.vector.overview]](https://eel.is/c++draft/inplace.vector.overview) makes `inplace_vector<T, 0>`.
- The reference, the iterator and the departures they force are `basic_bit_array`'s: no `data()`, `pointer` names
  `void`.
- *Capacity:* `capacity` and `max_size` are constants of the type, called as
  [[inplace.vector.capacity]](https://eel.is/c++draft/inplace.vector.capacity) calls them; `size()` and `empty()`
  are functions. `reserve(n)` throws `bad_alloc` where `n > N` and does nothing otherwise; `shrink_to_fit()` does
  nothing.
- *Modifiers:* As [[inplace.vector.modifiers]](https://eel.is/c++draft/inplace.vector.modifiers): `push_back` and
  `emplace_back` return the proxy for the new element, `try_push_back` and `try_emplace_back` return
  `optional<reference>`, `nullopt` when full, and `unchecked_push_back` and `unchecked_emplace_back` have
  `size() < N` as their precondition. There is no `try_append_range`, which
  [P4022R0](https://wg21.link/P4022R0) takes out of C++26.
- Growth past `N` throws `bad_alloc` before anything is written. `append_range` from a range with no size undoes
  what it appended before the exception leaves, so it has no effects, as
  [[inplace.vector.modifiers]](https://eel.is/c++draft/inplace.vector.modifiers) asks.
- *`from_blocks` constructor:* *Effects:* Moves `blocks` in. Every bit is a position, so `size()` is
  `blocks.size() * D`. *Preconditions:* `blocks.size() * D <= N`. The guide deduces the capacity the blocks hold.
- *`replace`:* *Effects:* Moves `blocks` in, `size()` becoming `blocks.size() * D`. *Preconditions:* as the
  constructor's.
- *`extract`:* *Returns:* the blocks, moved out, the unused bits of the last one clear. *Postconditions:*
  `size() == 0`. A width short of whole blocks comes back padded with `false`, which `resize` trims.
- `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined exactly where the blocks are an `inplace_vector`, and there alone
  the owner is usable in a constant expression. Over `static_vector` its copies are not `noexcept`, as
  `static_vector`'s are not; its `swap` is `noexcept` over either.
- It is not a structural type at any width.

### `erase`, `erase_if`

- As [[inplace.vector.erasure]](https://eel.is/c++draft/inplace.vector.erasure), with:
- *Constraints:* `c` is an owner of the sequence reading whose width grows, so `c.erase(c.cbegin(), c.cend())` is
  well-formed: `basic_bit_inplace_vector`, `basic_bit_vector` and `basic_bit_small_vector`.
- *Returns:* the number of elements removed.
- They are declared in a namespace of the library's adaptor and found by argument-dependent lookup, not as
  `xstd::erase` and `xstd::erase_if`.
- A `bit_vector` erases as a `vector<bool>` of the same bools does, counting what it removes alike.

### `hash`, `is_range`, `is_tuple_like`

As for `basic_bit_array`, the model being `inplace_vector<bool, N>`.

## `<xstd/bits/bit_vector.hpp>`

```cpp
namespace xstd {
  // [vector.bool.pspc], partial class template specialization vector<bool, Allocator>
  template<unsigned_integer Block, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_vector;
  using bit_vector = basic_bit_vector<size_t>;

  // [vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

namespace std {
  template<class Block, class Allocator>
    struct hash<xstd::basic_bit_vector<Block, Allocator>>;
}

namespace boost::container_hash {
  template<class Block, class Allocator>
    struct is_range<xstd::basic_bit_vector<Block, Allocator>> : false_type {};
  template<class Block, class Allocator>
    struct is_tuple_like<xstd::basic_bit_vector<Block, Allocator>> : false_type {};
}
```

### `basic_bit_vector`, `bit_vector`

```cpp
template<unsigned_integer Block, class Allocator>
  requires see below
class basic_bit_vector {
public:
  // types as basic_bit_array's, with
  using allocator_type       = Allocator;
  using block_container_type = vector<Block, Allocator>;

  // construct: [vector.bool.pspc]'s, with
  constexpr basic_bit_vector(from_blocks_t, vector<Block, Allocator> blocks) noexcept;
  constexpr basic_bit_vector(from_blocks_t, vector<Block, Allocator> blocks, const Allocator& a);

  // block exchange
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  friend constexpr auto swap(basic_bit_vector& x, basic_bit_vector& y) noexcept(noexcept(x.swap(y))) -> void;
};

template<input_iterator InputIterator, class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(InputIterator, InputIterator, Allocator = Allocator())
    -> basic_bit_vector<typename Allocator::value_type, Allocator>;
template<ranges::input_range R, class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(from_range_t, R&&, Allocator = Allocator())
    -> basic_bit_vector<typename Allocator::value_type, Allocator>;
template<class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(initializer_list<bool>, Allocator = Allocator())
    -> basic_bit_vector<typename Allocator::value_type, Allocator>;
template<unsigned_integer Block, class Allocator>
  basic_bit_vector(from_blocks_t, vector<Block, Allocator>) -> basic_bit_vector<Block, Allocator>;
template<unsigned_integer Block, class Allocator>
  basic_bit_vector(from_blocks_t, vector<Block, Allocator>, Allocator) -> basic_bit_vector<Block, Allocator>;
```

As [[vector.bool]](https://eel.is/c++draft/vector.bool), with:

- `bit_vector` holds its bits in `size_t` blocks, and `basic_bit_vector<Block, Allocator>` in `Block`s, in a
  `vector<Block, Allocator>`. The requires-clause admits a cv-unqualified `Block` whose digits are a power of two
  and at least `unsigned char`'s.
- The reference and the iterator are `basic_bit_array`'s. [[vector.bool]](https://eel.is/c++draft/vector.bool)'s
  extras are left out: `flip()` on the container and on the proxy, and the static `swap(reference, reference)`.
  `const_reference` is a read-only proxy rather than `bool`.
- `push_back` returns `void`, as [[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) has it. Growth goes
  through the blocks' own `resize`, so `vector`'s growth policy picks the capacity: `n` appends at the end make
  O(log n) allocations.
- `max_size()` answers the widest whole number of blocks' bits the blocks allow and that `difference_type` can
  count. A size past it throws `length_error` before anything is allocated.
- An insertion whose count is known shifts the tail in place; one from a single-pass range with no size, or from a
  window of the sequence itself, rebuilds aside and moves in, with the strong guarantee. A range that throws while it
  is read leaves the sequence as it was.
- The guides deduce the block from the allocator where one is given, and `size_t` where none is; the element is
  `bool`. The `from_blocks` guides deduce the block and the allocator both.
- *`from_blocks` constructors:* *Effects:* Move `blocks` in, the second under `a`. Every bit is a position, so
  `size()` is `blocks.size() * D`.
- *`replace`, `extract`:* As for `basic_bit_inplace_vector`, without a capacity to respect.
- `get_allocator()` returns the blocks' allocator, a construction argument and never part of the value.

### `erase`, `erase_if`

As for `<xstd/bits/bit_inplace_vector.hpp>`, after [[vector.erasure]](https://eel.is/c++draft/vector.erasure). The
two headers export the same function templates.

### `hash`, `is_range`, `is_tuple_like`

As for `basic_bit_array`, the model being `vector<bool>`, as
[[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) hashes it: equal values hash equal, whatever capacity
or allocation is behind them.

## `<xstd/bits/bit_set.hpp>`

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

### `basic_bit_set`, `bit_set`

```cpp
template<class Key, unsigned_integer Block, bit_index_mapping<Key> KeyMapping, see below Compare, class Allocator>
  requires see below
class basic_bit_set {
public:
  // types
  using key_type               = Key;
  using key_compare            = Compare;
  using value_type             = Key;
  using value_compare          = Compare;
  using allocator_type         = Allocator;
  using pointer                = void;
  using const_pointer          = void;
  using reference              = see below;
  using const_reference        = reference;
  using size_type              = size_t;
  using difference_type        = ptrdiff_t;
  using iterator               = see below;
  using const_iterator         = iterator;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using key_mapping_type       = KeyMapping;
  using block_container_type   = vector<Block, Allocator>;

  // construct: [set.cons]'s, with
  constexpr basic_bit_set(from_blocks_t, vector<Block, Allocator> blocks) noexcept;
  constexpr basic_bit_set(from_blocks_t, vector<Block, Allocator> blocks, const Allocator& a);

  // modifiers: [set.modifiers]'s, with
  constexpr auto insert(value_type x) -> pair<iterator, bool>;
  constexpr auto insert(const_iterator position, value_type x) -> iterator;
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  // set algebra
  constexpr auto operator&=(const basic_bit_set& other) noexcept -> basic_bit_set&;
  constexpr auto operator|=(const basic_bit_set& other) -> basic_bit_set&;
  constexpr auto operator^=(const basic_bit_set& other) -> basic_bit_set&;
  constexpr auto operator-=(const basic_bit_set& other) noexcept -> basic_bit_set&;
  constexpr auto operator<<=(size_t n) -> basic_bit_set&;
  constexpr auto operator>>=(size_t n) noexcept -> basic_bit_set&;

  friend constexpr auto operator==(const basic_bit_set& x, const basic_bit_set& y) noexcept -> bool;
  friend constexpr auto operator<=>(const basic_bit_set& x, const basic_bit_set& y) noexcept -> strong_ordering;
  friend constexpr auto swap(basic_bit_set& x, basic_bit_set& y) noexcept(noexcept(x.swap(y))) -> void;
};

constexpr auto operator&(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator|(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator^(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator-(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator<<(const basic_bit_set& x, size_t n) -> basic_bit_set;
constexpr auto operator>>(const basic_bit_set& x, size_t n) -> basic_bit_set;
```

As [[set]](https://eel.is/c++draft/set) for `set<Key, Compare, Allocator>`, with:

- `bit_set` is a set of `size_t` in `size_t` blocks, and `basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>`
  a set of `Key` whose positions `KeyMapping` names, in a `vector<Block, Allocator>`. The requires-clause admits a
  cv-unqualified `Block` whose digits are a power of two and at least `unsigned char`'s.
- `KeyMapping` takes a key to its position by `to_index` and back by `from_index`, preserving order. A mapping that
  models `sized_bit_index_mapping` closes the universe at `KeyMapping::size` keys.
- `Compare` is `less<Key>`, `less<>`, `greater<Key>` or `greater<>`: the order of the positions is fixed, so the
  comparator picks only a direction. Each holds no state, so a `comp` argument is taken and dropped, and `key_comp()`
  and `value_comp()` return `Compare()`. With `less<>` or `greater<>` the heterogeneous `contains`, `count`, `find`,
  `lower_bound`, `upper_bound`, `equal_range` and `erase` take a key of another type, over the positions whose keys
  are equivalent to it.
- **No nodes.** A key is a bit in a block, so `node_type`, `insert_return_type`, `extract` of an element,
  `insert(node_type&&)` and `merge` are absent, as `flat_set` drops them. The name `extract` is the block exchange
  below.
- **A proxy reference.** `reference` is a read-only proxy: copyable, not assignable, converting to `Key`, and `&ref`
  answers the iterator. `pointer` names `void`. It formats as `Key` does, and the set formats in braces.
- `iterator` is bidirectional, its `value_type` `Key`, its `reference` the proxy; `iterator` and `const_iterator`
  are one type. Iteration visits the keys in `Compare`'s order. An iterator is the storage object and a position, so
  growth that reallocates the blocks invalidates none.
- `insert` takes its key by value, one overload where [[set]](https://eel.is/c++draft/set) has two. `emplace` and
  `emplace_hint` take at most one argument, from which a `Key` is made.
- `size()` counts the keys, and runs in constant time, the count being kept beside the blocks. A key past the width
  grows the width; the width is capacity and never value.
- `max_size()` answers the keys the storage could hold, and no more than `KeyMapping::size` where the mapping names
  one.
- **Out of range.** `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range` and `erase(const
  key_type&)` are total, as on `set`: a key outside the universe, or a value that is no key at all, is absent, and
  where `Key` is totally ordered the bounds place it among the keys by that order. `insert`, `emplace`, the ranged
  forms and an enumerator's `|=` and `^=` throw `out_of_range` for a value the mapping ranks at or past
  `KeyMapping::size`; a key of an open universe throws `length_error` past what the storage can count.
- *Set algebra:* The compound operators combine with a set of the same type at any two widths, a block at a time:
  `&=` keeps the common keys, `|=` adds the other's, `^=` keeps those in one alone, `-=` removes the other's. `|=`
  and `^=` grow the width to the other's highest key. `<<= n` adds `n` to every key, growing the width towards
  `max_size()` and dropping the keys that land at or past it; `>>= n` subtracts `n` and drops the keys below `n`. The
  non-members return the copy the compound form makes of `x`, and are the owner's alone. They are the shorthand of
  `xstd::ranges::bit_set_intersection`, `bit_set_union`, `bit_set_difference` and `bit_set_symmetric_difference`.
- `==` holds where the two sets hold the same keys, whatever their widths. `<=>` is
  [[associative.reqmts]](https://eel.is/c++draft/associative.reqmts)'s lexicographic order of the keys; over
  descending keys that is the order of the two masks as unsigned numbers.
- *`from_blocks` constructors:* *Effects:* Move `blocks` in, the second under `a`. Every bit is a position, so the
  set's keys are those whose positions are set. Under an allocator equal to theirs the second keeps their storage,
  and under an unequal one it copies them into `a`'s. The guides deduce the block and the allocator.
- *`replace`:* *Effects:* Moves `blocks` in, the keys becoming those whose positions are set. *`extract`:*
  *Returns:* the blocks, moved out. *Postconditions:* `empty()`. A set loses nothing in the round trip, its width
  being capacity.
- The guides of [[set.overview]](https://eel.is/c++draft/set.overview) deduce `Key` as `size_t` and the block from
  the allocator, `size_t` where none is given.
- **Enumerator operands.** Where `Key` is an enumeration and `KeyMapping` does not read it as a mask, an enumerator
  is the one-element set holding it: `&=`, `|=`, `^=` and `-=` take one on the right, and on an owner `&`, `|`, `^`
  and `-` take one on either side. The enumeration itself gains no operator. `|=` and `^=` throw as `insert` does.
- The Boost.Hash2 hook appends the keys in iteration order, then their count, exactly as Hash2 writes the
  `set<Key, Compare>` of the same keys.

### `basic_bit_fixed_set`, `bit_fixed_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping, see below Compare>
  requires see below
class basic_bit_fixed_set {
public:
  // types as basic_bit_set's, without allocator_type, with
  using block_container_type = array<Block, see below>;

  // construct: [set.cons]'s without the allocator forms, with
  template<class Bits>
      requires see below
    constexpr basic_bit_fixed_set(from_blocks_t, const Bits& b) noexcept;
  constexpr basic_bit_fixed_set(const Key& mask) noexcept;                     // see below
  constexpr operator Key() const noexcept;                                      // see below

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
};

constexpr auto operator~(const basic_bit_fixed_set& x) noexcept -> basic_bit_fixed_set;

template<unsigned_integer Block>
  basic_bit_fixed_set(from_blocks_t, Block) -> basic_bit_fixed_set<size_t, Block, bit_blocks_extent_v<Block>>;
template<unsigned_integer Block, size_t K>
  basic_bit_fixed_set(from_blocks_t, array<Block, K>)
    -> basic_bit_fixed_set<size_t, Block, bit_blocks_extent_v<array<Block, K>>>;
template<unsigned_integer Block, size_t K>
  basic_bit_fixed_set(from_blocks_t, const Block (&)[K])
    -> basic_bit_fixed_set<size_t, Block, bit_blocks_extent_v<array<Block, K>>>;
template<class Enum>
    requires is_enum_v<Enum> and sized_bit_index_mapping<bit_key_mapping<Enum>, Enum>
  basic_bit_fixed_set(initializer_list<Enum>)
    -> basic_bit_fixed_set<Enum, least_block_t<bit_key_mapping<Enum>::size>, bit_key_mapping<Enum>::size,
                           bit_key_mapping<Enum>>;
```

As `basic_bit_set`, with:

- `bit_fixed_set<N>` holds the keys `0` to `N - 1` in `size_t` blocks. The requires-clause admits `Block` as
  `basic_bit_array`'s does. *Mandates:* where `KeyMapping` models `sized_bit_index_mapping<KeyMapping, Key>`,
  `KeyMapping::size == N`.
- There is no allocator, and neither `allocator_type` nor `get_allocator`. The default constructor is `noexcept`.
- `max_size` is a constant of the type, `N`; `size()` and `empty()` are functions that count the keys.
- `insert`, `emplace`, the ranged forms and an enumerator's `|=` and `^=` throw `out_of_range` for a key whose
  position is `N` or more.
- `~x` complements every key below `N`. `<<= n` keeps the keys that land below `N`, and `>>= n` the keys at or
  above `n`, both emptying the set where `n >= N`. Every operator over two sets is `noexcept`.
- *`from_blocks` constructor:* As `basic_bit_array`'s: position `i` is a key where bit `i` of `b` is set. The guides
  deduce the width the blocks name, `Key` being `size_t`. `xstd::bit_convert` goes the other way, from the set to a
  block, an array of blocks or a `std::bitset`, position `i` staying position `i`.
- The `initializer_list` guide deduces, from a list of enumerators whose `bit_key_mapping` names a size, the type
  `bit_enum_set<Enum>` names.
- It is a structural type exactly where `N` fills its blocks, as `bit_align<bit_fixed_set<N>>` does at any `N`.
- **The mask members**, present only where `bit_mask_mapping<KeyMapping, Key>` holds, which reads every value of
  `Key` as the block of its one-bit keys:
  - The constructor from a mask is implicit and sets the keys whose bits the mask has. *Preconditions:* the mask has
    no bit at or above `N`.
  - The conversion to `Key` is implicit and returns the mask with this set's keys.
  - The `initializer_list` constructor is the union of its values, and `noexcept`.
  - `&`, `|`, `^` and `-` take a mask on either side, and `&=`, `|=`, `^=` and `-=` one on the right, each over
    the mask's exact type. `&` and `-` with the mask on the right are total, the mask's bits at or above `N` meeting
    nothing; the others have the constructor's precondition. `set == mask` is total, and false for a mask with a bit
    at or above `N`.

### `basic_bit_inplace_set`, `bit_inplace_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping, see below Compare>
  requires see below
class basic_bit_inplace_set {
public:
  // types as basic_bit_set's, without allocator_type, with
  using block_container_type = see below;

  // construct: [set.cons]'s without the allocator forms, with
  constexpr basic_bit_inplace_set(from_blocks_t, block_container_type blocks) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
};

template<unsigned_integer Block, size_t K>
  basic_bit_inplace_set(from_blocks_t, see below) -> basic_bit_inplace_set<size_t, Block, bit_blocks_extent_v<Block> * K>;
```

As `basic_bit_set`, with:

- `bit_inplace_set<N>` holds keys below a capacity of `N` in `size_t` blocks, inline. The requires-clause admits
  `Block` as `basic_bit_set`'s does. *Mandates:* as `basic_bit_fixed_set`'s.
- `block_container_type` is `basic_bit_inplace_vector`'s, for `K = ceil(N / D)`. There is no allocator.
- `max_size` is a constant of the type, `N`; `size()` and `empty()` count the keys.
- A key past the width grows the width up to `N`. A key whose position is `N` or more throws `bad_alloc`, as
  `inplace_vector`'s growth does, where the mapping leaves the universe open, and `out_of_range` where the mapping
  closes it.
- `<<= n` keeps the keys that land below `N`, asks for no storage past it, and is `noexcept`.
- *`from_blocks` constructor, `replace`, `extract`:* As `basic_bit_set`'s, with the capacity as a precondition. The
  guide deduces the capacity the blocks hold.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined, and is not a
  structural type.

### `bit_set_view`

```cpp
template<bit_blocks Blocks, size_t N>
class bit_set_view {
public:
  // types, lookups, iterators and set algebra as basic_bit_set's, with
  template<class W>
      requires see below
    constexpr explicit bit_set_view(W&& blocks) noexcept;
  template<class Owner>
      requires see below
    constexpr bit_set_view(Owner& c) noexcept;
};

template<class Owner>
    requires see below
  bit_set_view(Owner&) -> bit_set_view<see below>;
template<class W>
    requires see below
  bit_set_view(W&&) -> bit_set_view<see below>;

namespace std::ranges {
  template<class Blocks, size_t N>
    constexpr bool enable_view<xstd::bit_set_view<Blocks, N>> = true;
  template<class Blocks, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_set_view<Blocks, N>> = true;
}
```

The set reading of bits another object owns, as `basic_bit_set` over `size_t` keys, with:

- `Blocks` names the blocks the view refers to: a block, a `span` of blocks, or the block container of the owner it
  views, `const` where they are; `N` is their width in bits, or `dynamic_extent`.
- The constructor from blocks is `explicit`. It takes an lvalue block, or a range of blocks that is an lvalue or a
  borrowed range, and its guide deduces a block as itself and a range as the `span` that lends it, so
  `bit_set_view(board)` over a `uint64_t` is a `bit_set_view<uint64_t>`.
- The constructor from an owner is implicit, and takes an lvalue owner of the set reading; a `const` owner gives a
  view of `const` bits. A view of mutable bits converts implicitly to a view of the same bits as `const`.
- It has no default constructor, owns nothing, and is a `ranges::view` and a borrowed range.
- Its modifiers write the bits it refers to, and exist only where those bits are not `const`. `insert` over a
  run-time width grows the owner's storage. The compound operators take any set over the same storage and keys,
  owner or view; the non-member operators, which copy, are the owner's alone.
- `==` and `<=>` compare it with whatever converts to it, its owner and lent blocks included, as `string_view`
  compares.
- It hashes what it presents: `hash<bit_set_view<Blocks, N>>` and the Hash2 hook answer as for its owner.

### `bit_key_mapping`, `bit_range_mapping`, `bit_find_mapping`, `enum_traits`

```cpp
template<class Key, Key First, size_t N>
  requires see below
struct bit_range_mapping {
  static constexpr size_t size = N;
  static constexpr auto is_key(Key key) noexcept -> bool;
  static constexpr auto to_index(Key key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
};

template<class Key, const auto& Keys>
  requires see below
struct bit_find_mapping {
  static constexpr size_t size = ranges::size(Keys);
  static constexpr auto is_key(Key key) noexcept -> bool;
  static constexpr auto to_index(Key key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
};
```

A key's mapping is an order-preserving bijection onto the positions `0` to `size - 1`: `a < b` exactly where
`to_index(a) < to_index(b)`, and `from_index` inverts `to_index`.

- `bit_range_mapping<Key, First, N>` maps the `N` consecutive keys from `First`, of an integer or an enumeration,
  onto `0` to `N - 1`. `is_key(k)` holds for `First <= k < First + N`, compared as an unsigned distance from
  `First`, so no bound overflows `Key`. A key outside the range ranks at `N` or above.
- `bit_find_mapping<Key, Keys>` maps each key of the list `Keys` to its rank there, found by binary search, so a gap
  costs no position. *Mandates:* `Keys` ascends strictly. `is_key(k)` holds for a listed key, and any other ranks at
  `size`.
- `bit_key_mapping<Key>` is the mapping a set takes by default. Its primary template is declared and not defined,
  and it is specialized as `char_traits` is:
  - for every unsigned integer `Key`, the identity, with no `size`, leaving the universe open; `is_key(k)` holds
    where `k` survives the round trip through `size_t`;
  - for an enumeration whose `enum_traits<Key>::values` is declared, `bit_range_mapping` where the values run without
    a gap and `bit_find_mapping` over the list otherwise;
  - for an enumeration that declares no list and has an enumerator `count`, `bit_range_mapping` from `Key{}` to
    `count`, the sentinel itself no key. *Mandates:* `count` is not negative.
  - A program may specialize it for a key type of its own, such as a strong index.
- `enum_traits<Enum>` is empty. A program specializes it beside an enumeration with a `static constexpr`
  `std::array` named `values`, listing the enumerators ascending, each once.

### `bit_flag_mapping`

```cpp
template<class Key, size_t N = see below>
  requires see below
struct bit_flag_mapping {
  using block_type = see below;
  static constexpr size_t size = N;
  static constexpr auto is_key(const Key& key) noexcept -> bool;
  static constexpr auto to_index(const Key& key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
  static constexpr auto to_block(const Key& mask) noexcept -> block_type;
  static constexpr auto from_block(const block_type& block) noexcept -> Key;
};
```

A bitmask type keyed on its own one-bit values.

- `Key` is an enumeration, an integer type other than `bool` and the character types, or a `std::bitset<M>` with
  `M > 0`. `block_type` is the unsigned word a mask is read and written as: an enumeration's or an integer's unsigned
  counterpart, or for a bitset the least block holding `M`, or past the widest block an array of them.
- `N` defaults to every bit of that word but a sign bit, and for a bitset to `M`. An enumeration with no fixed
  underlying type has no default, its values stopping at the bit-field its enumerators need. *Mandates:* `N` is no
  more than that default width.
- `is_key(k)` holds where `k` has exactly one bit set, below `N`; zero and a value of several bits are no key.
- `to_index(k)` is the position of `k`'s one bit. *Preconditions:* `k` has exactly one bit set.
- `from_index(i)` is the value with bit `i` alone. *Preconditions:* `i < N`.
- `to_block` and `from_block` read any value of `Key`, one-bit or not, as its block and back, which makes the
  mapping model `bit_mask_mapping<bit_flag_mapping<Key, N>, Key>`.

### `bit_enum_set`

- `bit_enum_set<Enum>` is the fixed set of an enumeration's keys, one bit each in the smallest block that holds them:
  `basic_bit_fixed_set<Enum, least_block_t<S>, S, bit_key_mapping<Enum>>` for `S = bit_key_mapping<Enum>::size`.
- *Constraints:* `Enum` is an enumeration whose default mapping names a size: its values listed in `enum_traits`,
  ended by a `count` sentinel, or a `bit_key_mapping` specialized directly.
- `basic_bit_fixed_set{E::a, E::b}` deduces the same type. Another block is
  `basic_bit_fixed_set<Enum, Block, S>`'s to spell.
- An enumerator is the one-element set holding it, on either side of `|`, `&`, `^` and `-`, as for `basic_bit_set`.

### `bit_flag_set`

- `bit_flag_set<Mask, N>` is the set of a bitmask type's one-bit values below `N`:
  `basic_bit_fixed_set<Mask, least_block_t<N>, N, bit_flag_mapping<Mask, N>, greater<Mask>>`.
- *Constraints:* `Mask` models `xstd::bit_mask`, is a signed integer, or is an enumeration whose `|` compiles,
  whatever it returns; and it is an enumeration, an integer or a `std::bitset`, as `bit_flag_mapping` takes.
  `N` defaults as `bit_flag_mapping`'s does: a signed type keeps its sign bit out, and an enumeration with no fixed
  underlying type spells its width.
- The mapping reads every value of `Mask` as a mask, so the set converts implicitly from and to it, and takes it in
  every mixed operator, as `basic_bit_fixed_set`'s mask members specify. Code written against
  `std::filesystem::perms` keeps its constants and changes only the variable's type.
- A braced list is the union of its values, and `contains(k)` is membership of one flag, false for a value of
  several bits.
- Iteration yields the one-bit values from the highest down, and `<=>` is the masks' numeric order.
- `xstd::ranges::bit_includes(p, m)`, `bit_intersects(p, m)` and `bit_disjoint(p, m)` take a mask as their second
  argument, and answer all-of, any-of and none-of. `~p` stays within `N`.

### `erase_if`

- As [[set.erasure]](https://eel.is/c++draft/set.erasure), over every set of this header.
- *Returns:* the number of keys removed.
- It is declared in a namespace of the library's adaptor and found by argument-dependent lookup, not as
  `xstd::erase_if`.

### `hash`, `is_range`, `is_tuple_like`

- `hash<T>` for each owner and for `bit_set_view`: `operator()` is `noexcept` and returns
  `xstd::bit_hash<T, boost::hash2::xxhash_64>()(x)`. Equal sets hash equal, whatever their widths.
- `is_range` and `is_tuple_like` are `false_type` for each owner, and `is_range` for the view, so Boost.ContainerHash
  and Hash2 take the hook of `basic_bit_set`.

## `<xstd/bits/bit_valarray.hpp>`

```cpp
namespace xstd {
  // [template.valarray], class template valarray, beside it the fixed and inplace columns
  template<unsigned_integer Block, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_valarray;
  using bit_valarray = basic_bit_valarray<size_t>;
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_fixed_valarray;
  template<size_t N>
    using bit_fixed_valarray = basic_bit_fixed_valarray<size_t, N>;
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_inplace_valarray;
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

### `basic_bit_valarray`, `bit_valarray`

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

### `basic_bit_fixed_valarray`, `bit_fixed_valarray`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_fixed_valarray {
public:
  // as basic_bit_valarray, with
  using block_container_type = array<Block, see below>;

  template<size_t M>
      requires (M == N)
    constexpr explicit basic_bit_fixed_valarray(const bool (&a)[M]) noexcept;
  template<class Bits>
      requires see below
    constexpr basic_bit_fixed_valarray(from_blocks_t, const Bits& b) noexcept;
};
```

As `basic_bit_valarray`, at a size fixed by its type, with:

- `bit_fixed_valarray<N>` holds `N` bools in `size_t` blocks: `std::bitset<N>` read elementwise. The requires-clause
  admits `Block` as `basic_bit_array`'s does.
- There is no constructor from a count, no `resize`, no allocator and no adopting `from_blocks` constructor. The
  `initializer_list` constructor sets the leading positions and leaves the rest `false`; the built-in array and
  `from_blocks` constructors are `basic_bit_array`'s. `size()` is a function.
- *Preconditions:* The pointer constructor's `n`, and the size a proxy selects for its constructor, is `N`.
- A `const` subscript gathers into `basic_bit_inplace_valarray<Block, N>`, where `Block` admits one, and throws
  `bad_alloc` where a gather selects more than `N` positions.
- Its `constexpr` members are usable in a constant expression, the subscripts by a `bit_slice` and by a mask
  included, and it is a structural type exactly where `N` fills its blocks.

### `basic_bit_inplace_valarray`, `bit_inplace_valarray`

As `basic_bit_valarray`, at a size within a capacity of `N`, with:

- `bit_inplace_valarray<N>` holds up to `N` bools in `size_t` blocks, inline. The requires-clause admits `Block` as
  `basic_bit_inplace_vector`'s does, and `block_container_type` is that class's.
- A size past `N` throws `bad_alloc`, and a refused `resize` leaves the array as it was.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined.

### `bit_slice`

```cpp
class bit_slice {
public:
  bit_slice() = default;
  constexpr bit_slice(size_t start, size_t size, size_t stride) noexcept;
  explicit bit_slice(const slice& s) noexcept;

  constexpr auto start() const noexcept -> size_t;
  constexpr auto size() const noexcept -> size_t;
  constexpr auto stride() const noexcept -> size_t;

  friend auto operator==(const bit_slice&, const bit_slice&) -> bool = default;
};
```

- As [[class.slice]](https://eel.is/c++draft/class.slice), usable in a constant expression, where `slice`'s
  constructors are not, so `v[bit_slice(start, size, stride)]` runs in one.
- It selects what the `slice` of the same three numbers selects.

### `bit_gslice`

```cpp
class bit_gslice {
public:
  bit_gslice();
  bit_gslice(size_t start, const valarray<size_t>& lengths, const valarray<size_t>& strides);
  explicit bit_gslice(const gslice& g);

  auto start() const noexcept -> size_t;
  auto size() const -> valarray<size_t>;
  auto stride() const -> valarray<size_t>;
};
```

- As [[class.gslice]](https://eel.is/c++draft/class.gslice), with its positions worked out once, in its constructor,
  and kept for every subscript that reuses it: as one bit per position where they ascend, and as the positions
  themselves where they repeat or turn back.
- It selects what the `gslice` of the same numbers selects, over every owner and block.
- Copies share the positions, and a proxy holds a copy, so a `bit_gslice_array` outlives the `bit_gslice` it was
  made from.

### `bit_slice_array`, `bit_gslice_array`, `bit_mask_array`, `bit_indirect_array`

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

## `<xstd/bits/bit_span.hpp>`

```cpp
namespace xstd {
  // [views.span], class template span
  template<bit_blocks Blocks, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_span;
  // beside it: the window its first, last and subspan return
  template<bit_blocks Blocks, size_t Extent = dynamic_extent, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_subspan;
}

namespace std::ranges {
  template<class Blocks, size_t N>
    constexpr bool enable_view<xstd::bit_span<Blocks, N>> = true;
  template<class Blocks, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_span<Blocks, N>> = true;
  template<class Blocks, size_t Extent, size_t N>
    constexpr bool enable_view<xstd::bit_subspan<Blocks, Extent, N>> = true;
  template<class Blocks, size_t Extent, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_subspan<Blocks, Extent, N>> = true;
}

namespace boost::container_hash {
  template<class Blocks, size_t N>
    struct is_range<xstd::bit_span<Blocks, N>> : false_type {};
  template<class Blocks, size_t N>
    struct is_tuple_like<xstd::bit_span<Blocks, N>> : false_type {};
  template<class Blocks, size_t Extent, size_t N>
    struct is_range<xstd::bit_subspan<Blocks, Extent, N>> : false_type {};
  template<class Blocks, size_t Extent, size_t N>
    struct is_tuple_like<xstd::bit_subspan<Blocks, Extent, N>> : false_type {};
}
```

### `bit_span`

```cpp
template<bit_blocks Blocks, size_t N>
class bit_span {
public:
  // types as basic_bit_array's, with
  using element_type = see below;
  static constexpr size_t extent = N;
  using subspan_type = bit_subspan<Blocks, dynamic_extent, N>;

  template<class W>
      requires see below
    constexpr explicit bit_span(W&& blocks) noexcept;
  template<class Owner>
      requires see below
    constexpr bit_span(Owner& c) noexcept;

  // subviews
  constexpr auto first(size_type count) const noexcept -> subspan_type;
  constexpr auto last(size_type count) const noexcept -> subspan_type;
  constexpr auto subspan(size_type offset, size_type count = dynamic_extent) const noexcept -> subspan_type;
  template<size_t Count>
      requires see below
    constexpr auto first() const noexcept -> bit_subspan<Blocks, Count, N>;
  template<size_t Count>
      requires see below
    constexpr auto last() const noexcept -> bit_subspan<Blocks, Count, N>;
  template<size_t Offset, size_t Count = dynamic_extent>
      requires see below
    constexpr auto subspan() const noexcept -> bit_subspan<Blocks, see below, N>;

  // observers
  constexpr auto size() const noexcept -> size_type;
  constexpr auto empty() const noexcept -> bool;
  constexpr auto max_size() const noexcept -> size_type;
};

template<class Owner>
    requires see below
  bit_span(Owner&) -> bit_span<see below>;
template<class W>
    requires see below
  bit_span(W&&) -> bit_span<see below>;
```

The sequence reading of bits another object owns. As [[views.span]](https://eel.is/c++draft/views.span) and
[[span.overview]](https://eel.is/c++draft/span.overview), with:

- `Blocks` names the blocks the view refers to, as for `bit_set_view`, and `N` their width in bits, or
  `dynamic_extent`. Over an owner they are the owner's block container and width: a `bit_span` over a
  `basic_bit_array<Block, N>` is a `bit_span<array<Block, K>, N>`.
- `element_type` is `bool`, or `const bool` where the bits are `const`; `value_type` is `bool`; `extent` is `N`.
- The reference and the iterator are `basic_bit_array`'s. So `pointer` names `void`, and what `span` builds on an
  address is absent: `data()`, `size_bytes()`, `as_bytes` and `as_writable_bytes`, and the constructors from a
  contiguous iterator, an array or a contiguous range of `bool`, for which a view over blocks or over an owner stands
  in. There is no default constructor.
- The constructor from blocks is `explicit`, and takes what `bit_set_view`'s does, its guide deducing as that one's.
- The constructor from an owner is implicit, and takes an lvalue owner of the sequence or the valarray reading; a
  `const` owner gives a view of `const` bits. A view of mutable bits converts implicitly to a view of the same bits
  as `const`.
- `size()`, `empty()` and `max_size()` are functions, whatever the width; `max_size()` is `size()`. `at(n)` throws
  `out_of_range` where `n >= size()`.
- *Subviews:* As [[span.sub]](https://eel.is/c++draft/span.sub), returning `bit_subspan`; `dynamic_extent` as a
  count reaches the end. *Preconditions:* as [[span.sub]](https://eel.is/c++draft/span.sub)'s, checked by an
  `assert`. A compile-time count the static width says cannot fit fails the constraints.
- Like `span`, it does not compare, order or hash: `hash<bit_span<Blocks, N>>` is disabled and there is no Hash2
  hook. It has no bitwise operators and no shifts.
- It is trivially copyable, a `ranges::view` and a borrowed range. A write through a `const` view writes the bits,
  as through a `const span`. Its iterators rebuild the storage from the blocks they hold, so they outlive the view.
- It cannot grow what it views: it has no `resize`, `push_back`, `insert` or `erase`.

### `bit_subspan`

```cpp
template<bit_blocks Blocks, size_t Extent, size_t N>
class bit_subspan {
public:
  // as bit_span, with
  static constexpr size_t extent = Extent;

  template<class OtherBlocks, size_t OtherExtent>
      requires see below
    constexpr explicit(Extent != dynamic_extent) bit_subspan(const bit_subspan<OtherBlocks, OtherExtent, N>& other)
      noexcept;
};
```

A window: the part of a `bit_span` or of another window that `first`, `last` and `subspan` return. As `bit_span`,
with:

- It stores what `span` stores, the start split over a pointer and a position because bits are not addressable,
  and the count where `Extent` is `dynamic_extent`: three words, or two at a static extent.
- It is made by `first`, `last` and `subspan`, which on a window return a window over the same `Blocks` and `N`.
  Between extents the rule is `span`'s: a static window converts to a dynamic one implicitly, and a dynamic one to a
  static one explicitly. *Preconditions:* the sizes agree. A window of mutable bits converts to one of `const` bits.
- At a static extent, `size`, `empty` and `max_size` are constants of the type, as for `basic_bit_array`; at the
  dynamic extent they are functions, and `max_size()` is `size()`.
- A write to a window masks its end blocks, so the bits around it, which share those blocks, are untouched.

## `<xstd/bits/ext/boost.hpp>`

```cpp
namespace xstd {
  // the sequence reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_vector;
  template<size_t N>
    using bit_small_vector = basic_bit_small_vector<size_t, N>;

  // the valarray reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_valarray;
  template<size_t N>
    using bit_small_valarray = basic_bit_small_valarray<size_t, N>;

  // the set reading
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_set;
  template<size_t N>
    using bit_small_set = basic_bit_small_set<size_t, size_t, N>;
}

namespace std {
  template<class Block, size_t N, class Alloc>
    struct hash<xstd::basic_bit_small_vector<Block, N, Alloc>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct hash<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>>;
}

namespace boost::container_hash {
  template<class Block, size_t N, class Alloc>
    struct is_range<xstd::basic_bit_small_vector<Block, N, Alloc>> : false_type {};
  template<class Block, size_t N, class Alloc>
    struct is_tuple_like<xstd::basic_bit_small_vector<Block, N, Alloc>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct is_range<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct is_tuple_like<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>> : false_type {};
}
```

The three readings over the small storage, `boost::container::small_vector<Block, K, Alloc>` for `K = ceil(N / D)`:
inline up to `N` bits, and on the heap past them. `<xstd/bits.hpp>` does not include this header, so the dependency
on Boost.Container's `small_vector` is opted into by name. It also exports the sequence reading's `erase` and
`erase_if` and the set reading's `erase_if`. `<xstd/bits/ext/boost/dynamic_bitset.hpp>`, which converts to and from
`boost::dynamic_bitset`, is outside this part.

### `basic_bit_small_vector`, `bit_small_vector`

```cpp
template<unsigned_integer Block, size_t N, class Alloc>
  requires see below
class basic_bit_small_vector {
public:
  // as basic_bit_vector, with
  using allocator_type       = see below;
  using block_container_type = boost::container::small_vector<Block, see below, Alloc>;

  constexpr basic_bit_small_vector() noexcept(noexcept(allocator_type()));
  constexpr explicit basic_bit_small_vector(const allocator_type& a) noexcept;
  constexpr basic_bit_small_vector(from_blocks_t, block_container_type blocks)
    noexcept(is_nothrow_move_constructible_v<block_container_type>);
  constexpr basic_bit_small_vector(from_blocks_t, block_container_type blocks, const allocator_type& a);
};
```

As `basic_bit_vector`, with:

- `bit_small_vector<N>` holds its bools in `size_t` blocks, inline up to `N` of them. The requires-clause admits
  `Block` as `basic_bit_vector`'s does, and `Alloc` defaults to Boost's own, as `small_vector`'s does.
- `allocator_type` is the small vector's, which wraps `Alloc`, and every allocator argument is of that type.
  Constructing from an allocator does not throw, inline blocks allocating nothing.
- `capacity()`, `reserve` and `shrink_to_fit` are the small vector's. There are no deduction guides.
- The Hash2 hook's model is `vector<bool>`.

### `basic_bit_small_set`, `bit_small_set`

As `basic_bit_set`, over the small storage, with:

- `bit_small_set<N>` holds its keys in `size_t` blocks, inline below `N`. *Mandates:* as `basic_bit_fixed_set`'s.
- `allocator_type` and the allocator arguments are as for `basic_bit_small_vector`; the constructor from an
  allocator is `noexcept`, and the `from_blocks` constructor `noexcept` where the block container's move is.
- A key past the inline capacity moves the blocks to the heap, keeping the keys and their count. There are no
  deduction guides.

### `basic_bit_small_valarray`, `bit_small_valarray`

As `basic_bit_valarray`, over the small storage, with:

- `bit_small_valarray<N>` holds its bools in `size_t` blocks, inline up to `N` of them, and `Alloc` defaults to
  Boost's own. Past the inline capacity the blocks move to the heap, and the elements stay what they were.
- A `const` subscript gathers into a `basic_bit_small_valarray` of the same parameters.

## `<xstd/bits/bit_algorithm.hpp>`

```cpp
namespace xstd::ranges {
  // [alg.nonmodifying], non-modifying sequence operations
  // [alg.all.of], all of
  template<see below R>
    constexpr auto bit_all_of(const R& r) noexcept -> bool;

  // [alg.any.of], any of
  template<see below R>
    constexpr auto bit_any_of(const R& r) noexcept -> bool;

  // [alg.none.of], none of
  template<see below R>
    constexpr auto bit_none_of(const R& r) noexcept -> bool;

  // [alg.foreach], for each
  template<class R, class F>
      requires see below
    constexpr auto bit_for_each(R&& r, F f) -> ranges::for_each_result<ranges::borrowed_iterator_t<R>, F>;
  template<class R, class F>
      requires see below
    constexpr auto bit_for_each(R&& r, F f) -> ranges::for_each_result<ranges::borrowed_iterator_t<R>, F>;

  // [alg.find], find
  template<see below R>
    constexpr auto bit_find(R&& r, bool value) noexcept -> ranges::borrowed_iterator_t<R>;

  // [alg.find.last], find last
  template<see below R>
    constexpr auto bit_find_last(R&& r, bool value) noexcept -> ranges::borrowed_subrange_t<R>;

  // [alg.count], count
  template<see below R>
    constexpr auto bit_count(const R& r, bool value) noexcept -> ranges::range_difference_t<const R>;

  // [mismatch], mismatch
  template<class R1, class R2>
      requires see below
    constexpr auto bit_mismatch(R1&& r1, R2&& r2) noexcept
      -> ranges::mismatch_result<ranges::borrowed_iterator_t<R1>, ranges::borrowed_iterator_t<R2>>;

  // [alg.equal], equal
  template<class R1, class R2>
      requires see below
    constexpr auto bit_equal(const R1& r1, const R2& r2) noexcept -> bool;

  // [alg.modifying.operations], mutating sequence operations
  // [alg.copy], copy
  template<see below R, see below O>
    constexpr auto bit_copy(R&& r, O out) noexcept -> ranges::copy_result<ranges::borrowed_iterator_t<R>, O>;
  template<see below R, see below O>
    constexpr auto bit_copy_backward(R&& r, O out_last) noexcept
      -> ranges::copy_backward_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.move], move
  template<see below R, see below O>
    constexpr auto bit_move(R&& r, O out) noexcept -> ranges::move_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.swap], swap
  template<see below R1, see below R2>
    constexpr auto bit_swap_ranges(R1&& r1, R2&& r2) noexcept
      -> ranges::swap_ranges_result<ranges::borrowed_iterator_t<R1>, ranges::borrowed_iterator_t<R2>>;

  // [alg.transform], transform
  template<see below R, see below O, see below F>
    constexpr auto bit_transform(R&& r, O out, F f)
      -> ranges::unary_transform_result<ranges::borrowed_iterator_t<R>, O>;
  template<see below R1, see below R2, see below O, see below F>
    constexpr auto bit_transform(R1&& r1, R2&& r2, O out, F f)
      -> ranges::binary_transform_result<ranges::borrowed_iterator_t<R1>, ranges::borrowed_iterator_t<R2>, O>;

  // [alg.fill], fill
  template<see below R>
    constexpr auto bit_fill(R&& r, bool value) noexcept -> ranges::borrowed_iterator_t<R>;

  // [alg.reverse], reverse
  template<see below R>
      requires see below
    constexpr auto bit_reverse(R&& r) noexcept -> ranges::borrowed_iterator_t<R>;
  template<see below R, see below O>
    constexpr auto bit_reverse_copy(R&& r, O out) noexcept
      -> ranges::reverse_copy_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.rotate], rotate
  template<see below R>
      requires see below
    constexpr auto bit_rotate(R&& r, ranges::iterator_t<R> middle) noexcept -> ranges::borrowed_subrange_t<R>;
  template<see below R, see below O>
    constexpr auto bit_rotate_copy(R&& r, ranges::iterator_t<R> middle, O out) noexcept
      -> ranges::rotate_copy_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.shift], shift
  template<see below R>
      requires see below
    constexpr auto bit_shift_left(R&& r, ranges::range_difference_t<R> n) noexcept
      -> ranges::borrowed_subrange_t<R>;
  template<see below R>
      requires see below
    constexpr auto bit_shift_right(R&& r, ranges::range_difference_t<R> n) noexcept
      -> ranges::borrowed_subrange_t<R>;

  // [alg.sorting], sorting and related operations
  template<see below R>
    constexpr auto bit_sort(R&& r) noexcept -> ranges::borrowed_iterator_t<R>;
  template<see below R>
    constexpr auto bit_is_sorted(const R& r) noexcept -> bool;

  // [alg.partitions], partitions
  template<see below R>
    constexpr auto bit_is_partitioned(const R& r) noexcept -> bool;
  template<see below R>
    constexpr auto bit_partition(R&& r) noexcept -> ranges::borrowed_subrange_t<R>;
  template<see below R>
    constexpr auto bit_partition_point(R&& r) noexcept -> ranges::borrowed_iterator_t<R>;

  // [includes], includes, with intersects and disjoint beside it
  template<see below S>
    constexpr auto bit_includes(const S& s1, const type_identity_t<S>& s2) noexcept -> bool;
  template<see below S>
    constexpr auto bit_intersects(const S& s1, const type_identity_t<S>& s2) noexcept -> bool;
  template<see below S>
    constexpr auto bit_disjoint(const S& s1, const type_identity_t<S>& s2) noexcept -> bool;

  // [alg.set.operations], set operations, each returning the set
  template<see below S>
      requires requires (const S& s) { { s | s } -> same_as<S>; }
    constexpr auto bit_set_union(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;
  template<see below S>
      requires requires (const S& s) { { s & s } -> same_as<S>; }
    constexpr auto bit_set_intersection(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;
  template<see below S>
      requires requires (const S& s) { { s - s } -> same_as<S>; }
    constexpr auto bit_set_difference(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;
  template<see below S>
      requires requires (const S& s) { { s ^ s } -> same_as<S>; }
    constexpr auto bit_set_symmetric_difference(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;

  // [alg.three.way], three-way comparison algorithms
  template<class R1, class R2>
      requires see below
    constexpr auto bit_lexicographical_compare_three_way(const R1& r1, const R2& r2) noexcept -> strong_ordering;
}
```

Each algorithm has a header of its own under `<xstd/bits/bit_algorithm/>`, named after it, and the umbrella exports
them in the order `<algorithm>` gives their counterparts, `bit_intersects` and `bit_disjoint` beside
`bit_includes`. Each is specified as the `std::ranges` algorithm it is named after, *with* what the packing changes.
What every one of them changes is stated here once:

- **No function object but the caller's.** None takes a projection. Where the counterpart takes a predicate it is
  `identity()`, so `bit_all_of(r)` is `ranges::all_of(r, identity())`; where it takes a comparison it is
  `ranges::less`, so `bit_sort(r)` is `ranges::sort(r)`; a value searched, counted or filled is a `bool`. Only
  `bit_for_each` and `bit_transform` take a function, and the set operations return a set where
  [[alg.set.operations]](https://eel.is/c++draft/alg.set.operations) writes through an output iterator.
- **Ranges of ours alone.** The constraints admit the library's own owners, views and iterators, whose blocks the
  algorithm reads, and nothing else: `vector<bool>`, `set<size_t>` and their iterators are turned away, as is a
  sequence where a set is asked for and a set where a sequence is. The `std::ranges` algorithms run over those and
  over ours alike.
- **A block at a time.** Complexity is counted in block operations, each a read, write or popcount of one block. `N`
  is the size of the range, `D` the digits of its block, and O(`N`/`D`) means at most a constant times
  ⌈`N`/`D`⌉ + 1.
- **The counterpart's results.** Each returns the type its counterpart returns, so the iterator into an rvalue owner
  is `ranges::dangling`. Every algorithm that calls no function of the caller's is `noexcept`; `bit_for_each` and
  `bit_transform` throw what their function throws, and a set operation is `noexcept` where its operator is.

The constraints are exposition-only concepts in the code, written below as these phrases:

- A *sequence of ours* is an owner or a view of the sequence reading, the valarray reading included:
  `basic_bit_array`, `basic_bit_inplace_vector`, `basic_bit_vector`, `basic_bit_small_vector`, the valarrays,
  `bit_span` and `bit_subspan`, `const` or not. A *whole sequence* is one that is not a window.
- A sequence is *writable* where its storage is reached as writable: an owner that is not `const`, or a view of bits
  that are not `const`, whatever the view's own `const`.
- A *set of ours* is an owner or a view of the set reading: `basic_bit_set`, `basic_bit_fixed_set`,
  `basic_bit_inplace_set`, `basic_bit_small_set`, `bit_set_view` and the aliases over them.
- An *output iterator of ours* is an iterator of a sequence of ours that is `indirectly_writable<O, bool>`, a
  view's or a window's included: `iterator`, never `const_iterator`.

### `bit_all_of`, `bit_any_of`, `bit_none_of`

As `ranges::all_of`, `ranges::any_of` and `ranges::none_of` in
[[alg.all.of]](https://eel.is/c++draft/alg.all.of), [[alg.any.of]](https://eel.is/c++draft/alg.any.of) and
[[alg.none.of]](https://eel.is/c++draft/alg.none.of), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* Whether every, some or no position of `r` is `true`: `true`, `false` and `true` on an empty sequence.
- *Complexity:* O(`N`/`D`). A whole sequence asks its storage; a window asks the blocks it covers, its end blocks
  masked to its own positions.

### `bit_for_each`

As `ranges::for_each` in [[alg.foreach]](https://eel.is/c++draft/alg.foreach), with:

- *Constraints:* `invocable<F&, ranges::range_value_t<R>>` is modeled, and:
  - for the first form, `R` is a sequence or a set of ours, a window included;
  - for the second, `remove_cvref_t<R>` is `ranges::reverse_view<V>`, where `V` is a view of ours or a
    `ranges::ref_view` of an owner of ours: what `views::reverse` makes of a view or of an lvalue owner.
- *Effects:* Calls `f` with each element by value, a `bool` or a key, in the order `r` iterates. The second form
  walks the range under the `reverse_view` against its order, so `bit_for_each(views::reverse(r), f)` is
  `ranges::for_each(views::reverse(r), f)`.
- *Returns:* `{ranges::end(r), std::move(f)}`.
- *Complexity:* Exactly `ranges::distance(r)` applications of `f`, and one load per block. A set's keys come from
  each block lowest first or highest first, as the set and the direction order them.
- A functor that takes the element as a non-`const` lvalue reference is turned away, and of a functor overloaded on
  value category the overload taking an rvalue is the one called.
- A functor returning `bool` is called on every element, its answer ignored, as `ranges::for_each` calls it.

### `bit_find`, `bit_find_last`

As `ranges::find` and `ranges::find_last` in [[alg.find]](https://eel.is/c++draft/alg.find) and
[[alg.find.last]](https://eel.is/c++draft/alg.find.last), with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* For `bit_find`, `ranges::begin(r) + i`, where `i` is the first position holding `value`, or
  `ranges::size(r)` where none does. For `bit_find_last`, `{ranges::begin(r) + i, ranges::end(r)}`, where `i` is
  the last such position, or `{ranges::end(r), ranges::end(r)}` where none is.
- *Complexity:* O(`N`/`D`). `bit_find` reads up from the first block and `bit_find_last` down from the last, each
  stopping at the block that holds the answer. A search for `false` reads the blocks complemented, at the same cost.

### `bit_count`

As `ranges::count` in [[alg.count]](https://eel.is/c++draft/alg.count), with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* The number of positions of `r` holding `value`, as `ranges::range_difference_t<const R>`, signed as
  `ranges::count`'s result is: the ones popcounted, and the zeros as `ranges::size(r)` less the ones.
- *Complexity:* O(`N`/`D`) popcounts. The padding of the last block is never counted.

### `bit_mismatch`

As `ranges::mismatch` in [[mismatch]](https://eel.is/c++draft/mismatch), with:

- *Constraints:* `R1` is a sequence of ours, and `remove_cvref_t<R1>` and `remove_cvref_t<R2>` are the same type:
  `bit_array<8>` and `bit_array<9>` are turned away.
- *Effects:* Two whole sequences whose storage compares its blocks are compared a block at a time, at two run-time
  sizes as well, stopping at the shorter's end, so set bits in the longer where the shorter has padding do not
  count. Two windows are compared by `ranges::mismatch(r1, r2)` itself.
- *Complexity:* O(min(`N1`, `N2`)/`D`) for two whole sequences; for two windows, `ranges::mismatch`'s.

### `bit_equal`

As `ranges::equal` in [[alg.equal]](https://eel.is/c++draft/alg.equal), with:

- *Constraints:* `R1` and `R2` are of one reading and one width kind:
  - both sequences of ours, or both sets of ours with the same `key_type`, `key_mapping_type` and `key_compare`;
  - the width their types fix, a window's extent where it has one and otherwise its storage's, is the same, or both
    widths are run-time values.

  So `bit_array<8>` compares with `basic_bit_array<uint8_t, 8>` and `bit_vector` with `basic_bit_vector<uint8_t>`,
  and `bit_array<8>` does not compare with `bit_vector`, with `bit_array<9>` or with `bit_fixed_set<8>`, nor
  `bit_fixed_set<8>` with `bit_set` or with a set of the same keys under `greater<>`.
- *Returns:* `ranges::equal(r1, r2)` over the elements in iteration order: for two sequences, whether the sizes and
  every position agree; for two sets, whether they hold the same keys, at any two widths.
- *Complexity:* Two whole operands over one storage type compare their own blocks, in O(`N`/`D`). Any other pair
  compares 64-bit words assembled from each side's blocks at each side's offset, in O(`N1`/`D1` + `N2`/`D2`).
- *Remarks:* The constraints admit exactly the pairs `bit_hash` hashes alike where their values are equal, so what
  compares equal hashes equal. `bit_equal_to` is the transparent comparison over it.
- A capacity of zero against a run-time width that holds bools compares their sizes.

### `bit_copy`, `bit_copy_backward`, `bit_move`

As `ranges::copy` and `ranges::copy_backward` in [[alg.copy]](https://eel.is/c++draft/alg.copy), and `ranges::move`
in [[alg.move]](https://eel.is/c++draft/alg.move), with:

- *Constraints:* `R` is a sequence of ours, a window included, and `O` is an output iterator of ours.
- *Preconditions:* Where the destination lies in the blocks `r` reads, `out`, or for `bit_copy_backward` `out_last`,
  is not in `(ranges::begin(r), ranges::end(r))`. These are [[alg.copy]](https://eel.is/c++draft/alg.copy)'s,
  except that a destination exactly on its source is admitted.
- *Effects:* The `N` positions of `r` are written a block of the destination at a time: lowest first for `bit_copy`
  and `bit_move`, highest first for `bit_copy_backward`. Source and destination may sit at any offsets in storages
  of different block types; the source's bits are assembled from its own blocks where the two differ, and each
  destination block is written under the mask of the positions that remain, so no position outside the destination
  range changes. Moving a `bool` is copying it.
- *Complexity:* O(`N`/`D`), where `D` is the digits of the narrower of the two blocks.

### `bit_swap_ranges`

As `ranges::swap_ranges` in [[alg.swap]](https://eel.is/c++draft/alg.swap), with:

- *Constraints:* `R1` and `R2` are writable sequences of ours, windows included.
- *Effects:* Exchanges the first `M` = min(`ranges::size(r1)`, `ranges::size(r2)`) positions of the two, a block of
  the narrower block type at a time, at any two offsets.
- *Complexity:* O(`M`/`D`), where `D` is the digits of the narrower block.

### `bit_transform`

As `ranges::transform` in [[alg.transform]](https://eel.is/c++draft/alg.transform), with:

- *Constraints:* `R`, or `R1` and `R2`, are sequences of ours, windows included; `O` is an output iterator of ours;
  `invocable<F&, bool>`, or `invocable<F&, bool, bool>`, is modeled and its result converts to `bool`. A function
  taking `bool&`, or answering a `string`, is turned away.
- *Effects:* The unary form calls `f(false)` and `f(true)` once each; the binary form calls `f` once at each of the
  four pairs. The answers make a truth table that is applied as one block expression, a block of the destination at
  a time as `bit_copy` writes it: for the unary form `(x & T) | (~x & U)`, where `T` and `U` are all ones or all
  zeros as `f(true)` and `f(false)` answer. The binary form goes as far as the shorter input. `out` may be
  `ranges::begin(r1)`, as [[alg.transform]](https://eel.is/c++draft/alg.transform) allows.
- *Complexity:* Exactly two applications of `f` for the unary form and four for the binary, whatever `N`, an empty
  input included; O(`N`/`D`) block operations.
- *Remarks:* The positions written are those `ranges::transform` writes; a function with side effects sees two or
  four calls where `ranges::transform` makes one per element.

### `bit_fill`

As `ranges::fill` in [[alg.fill]](https://eel.is/c++draft/alg.fill), with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* A whole sequence fills every block of its storage; a window sets its own positions, its end blocks
  masked, and leaves every other position as it was.
- *Complexity:* O(`N`/`D`).

### `bit_reverse`

As `ranges::reverse` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* `R` is a whole sequence of ours whose storage is writable: an owner that is not `const`, or a whole
  view of bits that are not `const`. A window, which shares its end blocks with positions outside it, is turned
  away.
- *Effects:* Position `i` takes position `N - 1 - i`: the blocks in reverse order, each block's bits reversed, and
  the padding, which then lies at the bottom, shifted out. The padding stays clear.
- *Returns:* `ranges::end(r)`.
- *Complexity:* O(`N`/`D`), without allocating.

### `bit_reverse_copy`

As `ranges::reverse_copy` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* As for `bit_copy`.
- *Effects:* Each block of the destination is read from the top of `r`, its bits reversed, and written as `bit_copy`
  writes it.
- *Complexity:* As for `bit_copy`.

### `bit_rotate`

As `ranges::rotate` in [[alg.rotate]](https://eel.is/c++draft/alg.rotate), with:

- *Constraints:* As for `bit_reverse`.
- *Preconditions:* `middle` is in `[ranges::begin(r), ranges::end(r)]`, checked by an `assert`.
- *Effects:* Position `i` takes position (`i` + `m`) mod `N`, for `m = middle - ranges::begin(r)`, so a whole turn
  is no turn. The blocks rotate by whole blocks, then a funnel shift moves the remaining bits across each boundary.
- *Returns:* `{ranges::begin(r) + (ranges::end(r) - middle), ranges::end(r)}`.
- *Complexity:* O(`N`/`D`), without allocating.
- *Remarks:* It moves no proxy through the standard library, so it is exact over proxy references where libstdc++'s
  `ranges::rotate` before version 16 is not ([GCC PR 121913](https://gcc.gnu.org/PR121913)). The `rotr(n)` of
  [P3103R2](https://wg21.link/P3103R2) is `bit_rotate(v, v.begin() + n)`, and its `rotl(n)` is
  `bit_rotate(v, v.end() - n)`.

### `bit_rotate_copy`

As `ranges::rotate_copy` in [[alg.rotate]](https://eel.is/c++draft/alg.rotate), with:

- *Constraints:* As for `bit_copy`.
- *Preconditions:* `middle` is in `[ranges::begin(r), ranges::end(r)]`, checked by an `assert`.
- *Effects:* Copies `[middle, ranges::end(r))` and then `[ranges::begin(r), middle)`, each as `bit_copy` does.
- *Complexity:* As for `bit_copy`.

### `bit_shift_left`, `bit_shift_right`

As `ranges::shift_left` and `ranges::shift_right` in [[alg.shift]](https://eel.is/c++draft/alg.shift), with:

- *Constraints:* As for `bit_reverse`.
- *Preconditions:* `0 <= n`, checked by an `assert`.
- *Effects:* None where `n == 0` or `n >= N`. Otherwise `bit_shift_left` moves position `i + n` to `i`, and
  `bit_shift_right` position `i` to `i + n`, for every `i < N - n`. The `n` positions vacated are `false`, where
  [[alg.shift]](https://eel.is/c++draft/alg.shift) leaves them valid but unspecified.
- *Returns:* As [[alg.shift]](https://eel.is/c++draft/alg.shift): for `bit_shift_left`,
  `{ranges::begin(r), ranges::begin(r) + (N - n)}` where `n < N` and `{ranges::begin(r), ranges::begin(r)}`
  otherwise; for `bit_shift_right`, `{ranges::begin(r) + n, ranges::end(r)}` where `n < N` and
  `{ranges::end(r), ranges::end(r)}` otherwise.
- *Complexity:* O(`N`/`D`).
- *Remarks:* A sequence's front is a bit string's low end, so `bit_shift_left` is the storage's `>>=` and
  `bit_shift_right` its `<<=`.

### `bit_sort`

As `ranges::sort` in [[alg.sorting]](https://eel.is/c++draft/alg.sorting), `false` ordered before `true`, with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* Counts the `k` positions that are `true`, then sets the first `N - k` positions `false` and the last `k`
  `true`. Every `false` is like every other, so the result is `ranges::stable_sort`'s too.
- *Complexity:* O(`N`/`D`).

### `bit_is_sorted`, `bit_is_partitioned`

As `ranges::is_sorted` in [[alg.sorting]](https://eel.is/c++draft/alg.sorting) and `ranges::is_partitioned` in
[[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* For `bit_is_sorted`, whether no `false` follows the first `true`; for `bit_is_partitioned`, whether no
  `true` follows the first `false`.
- *Complexity:* O(`N`/`D`): two searches, the second from where the first stopped.

### `bit_partition`

As `ranges::partition` in [[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* Counts the `k` positions that are `true`, then sets the first `k` positions `true` and the rest
  `false`.
- *Returns:* `{ranges::begin(r) + k, ranges::end(r)}`.
- *Complexity:* O(`N`/`D`).

### `bit_partition_point`

As `ranges::partition_point` in [[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Preconditions:* `r` is partitioned, as [[alg.partitions]](https://eel.is/c++draft/alg.partitions) requires.
- *Returns:* `ranges::begin(r) + i`, where `i` is the first position that is `false`, or `ranges::size(r)`.
- *Complexity:* O(`i`/`D`): a search up to the block holding the first `false`, where
  [[alg.partitions]](https://eel.is/c++draft/alg.partitions) bisects.

### `bit_includes`, `bit_intersects`, `bit_disjoint`

As `ranges::includes` in [[includes]](https://eel.is/c++draft/includes), and as whether `ranges::set_intersection`
in [[set.intersection]](https://eel.is/c++draft/set.intersection) would write a key, or none, with:

- *Constraints:* `S` is a set of ours, a view included. The second parameter is `const type_identity_t<S>&`, so `S`
  deduces from `s1` alone and `s2` converts to it: a flag set takes a flag second,
  `bit_includes(perms, fs::perms::owner_read)`, and `bit_fixed_set<8>` with `bit_fixed_set<9>` is turned away.
- *Returns:*
  - `bit_includes`: whether every key of `s2` is a key of `s1`, as `ranges::includes(s1, s2, s1.key_comp())`;
  - `bit_intersects`: whether some key is a key of both;
  - `bit_disjoint`: `not bit_intersects(s1, s2)`.
- *Complexity:* O(`K`), where `K` is the number of blocks of the wider. The blocks both storages hold are compared
  pairwise, and `bit_includes` also asks whether `s2` holds a key in the blocks above `s1`'s. A width of zero
  fixed in the type is answered without a block: `true` for `bit_includes`, `false` for `bit_intersects`.
- They answer at any two run-time widths.
- *Remarks:* `x` is a subset of `y` where `bit_includes(y, x)`, and a proper one where `not bit_includes(x, y)` as
  well; every other relation between two sets is spelled from these three.

### `bit_set_union`, `bit_set_intersection`, `bit_set_difference`, `bit_set_symmetric_difference`

As `ranges::set_union`, `ranges::set_intersection`, `ranges::set_difference` and
`ranges::set_symmetric_difference` in [[set.union]](https://eel.is/c++draft/set.union),
[[set.intersection]](https://eel.is/c++draft/set.intersection),
[[set.difference]](https://eel.is/c++draft/set.difference) and
[[set.symmetric.difference]](https://eel.is/c++draft/set.symmetric.difference), returning the set rather than
writing through an output iterator, with:

- *Constraints:* `S` is a set of ours whose operator `|`, `&`, `-` or `^` respectively answers an `S`: an owner. A
  view, which has no result to hold, is turned away. `s2` converts to `S`, as for `bit_includes`.
- *Returns:* `s1 | s2`, `s1 & s2`, `s1 - s2` and `s1 ^ s2`: the set that
  [range-v3](https://github.com/ericniebler/range-v3)'s `views::set_union(s1, s2, s1.key_comp())` and its siblings
  would hold, of the first argument's type, whatever widths held the keys.
- *Complexity:* That of the operator.
- *Remarks:* The exception specification is `noexcept(declval<const S&>() | declval<const S&>())`, with the
  operator each one calls.

### `bit_lexicographical_compare_three_way`

As `lexicographical_compare_three_way` in [[alg.three.way]](https://eel.is/c++draft/alg.three.way), over two ranges
rather than two iterator pairs, the shape [P2022R3](https://wg21.link/P2022R3) proposes for
`ranges::lexicographical_compare_three_way`, with:

- *Constraints:* As for `bit_equal`.
- *Returns:* `lexicographical_compare_three_way(ranges::begin(r1), ranges::end(r1), ranges::begin(r2),
  ranges::end(r2))`, a `strong_ordering`. Over two sequences, `false` precedes `true` and a prefix precedes the
  longer sequence. Over two sets it is `set`'s order, the keys compared lexicographically in iteration order:
  ascending keys position by position, and descending ones as their masks compared as numbers.
- *Complexity:* As for `bit_equal`.
- *Remarks:* `lexicographical_compare(r1, r2)` is `bit_lexicographical_compare_three_way(r1, r2) < 0`.

## `<xstd/bits/bit_iterator.hpp>`

```cpp
namespace xstd::ranges {
  // [range.iter.ops], range iterator operations
  template<see below I>
    constexpr auto bit_advance(I& it, iter_difference_t<I> n) noexcept -> void;
  template<see below I>
    constexpr auto bit_advance(I& it, iter_difference_t<I> n, I bound) noexcept -> iter_difference_t<I>;

  template<see below I>
    constexpr auto bit_distance(I first, I last) noexcept -> iter_difference_t<I>;
  template<ranges::common_range R>
      requires see below
    constexpr auto bit_distance(R&& r) noexcept -> ranges::range_difference_t<R>;

  template<see below I>
    constexpr auto bit_next(I it, iter_difference_t<I> n) noexcept -> I;
  template<see below I>
    constexpr auto bit_next(I it, iter_difference_t<I> n, I bound) noexcept -> I;

  template<see below I>
      requires bidirectional_iterator<I>
    constexpr auto bit_prev(I it, iter_difference_t<I> n) noexcept -> I;
  template<see below I>
      requires bidirectional_iterator<I>
    constexpr auto bit_prev(I it, iter_difference_t<I> n, I bound) noexcept -> I;
}
```

The iterator operations over any iterator of ours, each in a header of its own under `<xstd/bits/bit_iterator/>`. A
sequence's iterator is random access and steps by an offset, so over it each is its `std::ranges` counterpart. A
set's iterator is bidirectional and steps a key at a time, so over it each skips whole blocks by their popcount and
selects within the block that holds its target: select is `*bit_next(s.begin(), n)`, and the rank of `k` is
`bit_distance(s.begin(), s.lower_bound(k))`.

An *iterator of ours* is an iterator of a sequence or a set of ours, `const` or not, a view's or a window's
included; `vector<bool>::iterator` and `set<size_t>::iterator` are turned away. Only the forms that take a count are
provided: there is no `bit_advance(it, bound)`, `bit_next(it)`, `bit_next(it, bound)` or `bit_prev(it)`.

### `bit_advance`

As `ranges::advance` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* `I` is an iterator of ours.
- *Preconditions:* As [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops)'s. For the first form over a set,
  the `n` keys are there to step over, checked by an `assert`.
- *Effects:* Over a sequence, `ranges::advance(it, n)` and `ranges::advance(it, n, bound)`. Over a set, moves `it`
  `n` keys on in the set's order, or `-n` back, skipping whole blocks by their popcount and selecting the key within
  the block that holds it, from the block's top where the walk goes down; the second form stops at `bound`.
- *Returns:* For the second form, the steps not taken, with the sign of `n`, as `ranges::advance(it, n, bound)`
  answers them.
- *Complexity:* Constant over a sequence. Over a set, linear in the number of blocks between `it` and where it
  stops, rather than one step per key.
- A view's iterators step over the keys it views, its owner's or lent blocks, and an empty set's `begin()` is its
  `end()`.

### `bit_distance`

As `ranges::distance` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* For the first form, `I` is an iterator of ours. For the second, `R` models `ranges::common_range`
  and its iterator is an iterator of ours.
- *Preconditions:* `[first, last)` denotes a range.
- *Returns:* For the first form, `ranges::distance(first, last)` over a sequence, and over a set the number of keys
  in `[first, last)`. For the second, `ranges::distance(r)` where `R` models `ranges::sized_range`, as a set does,
  and otherwise `bit_distance(ranges::begin(r), ranges::end(r))`: so
  `bit_distance(subrange(s.lower_bound(a), s.lower_bound(b)))` counts the keys in `[a, b)`.
- *Complexity:* Constant over a sequence and over a sized range. Over a set's iterators, linear in the number of
  blocks between them: a popcount of the whole blocks and of the masked ends.
- *Remarks:* Counting a set's keys is a distance and not a count: `ranges::count(s, k)` asks how often `k` occurs,
  which `s.contains(k)` answers.

### `bit_next`, `bit_prev`

As `ranges::next` and `ranges::prev` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* `I` is an iterator of ours, and for `bit_prev` it models `bidirectional_iterator`, as
  `ranges::prev` asks.
- *Effects:* `bit_next` calls `bit_advance(it, n)`, or `bit_advance(it, n, bound)`, on its copy `it`; `bit_prev`
  calls them with `-n`.
- *Returns:* `it`.
- *Complexity:* As for `bit_advance`.
