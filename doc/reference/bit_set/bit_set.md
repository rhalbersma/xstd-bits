# `<xstd/bits/bit_set/bit_set.hpp>`

## `basic_bit_set`, `bit_set`

```cpp
template<class Key, bit_storage Blocks = vector<size_t>, size_t N = dynamic_extent,
         bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, see below Compare = less<Key>>
  requires see below
class basic_bit_set {
public:
  // types
  using key_type               = Key;
  using key_compare            = Compare;
  using value_type             = Key;
  using value_compare          = Compare;
  using allocator_type         = typename Blocks::allocator_type;  // where Blocks has one
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
  using block_container_type   = Blocks;

  // construct: [set.cons]'s, each allocator form where Blocks has an allocator, with
  constexpr basic_bit_set(from_blocks_t, block_container_type blocks)
    noexcept(is_nothrow_move_constructible_v<block_container_type>);
  constexpr basic_bit_set(from_blocks_t, block_container_type blocks, const allocator_type& a);

  // element access
  constexpr auto front() const noexcept -> const_reference;
  constexpr auto back() const noexcept -> const_reference;

  // modifiers: [set.modifiers]'s, with
  constexpr auto insert(value_type x) -> pair<iterator, bool>;
  constexpr auto insert(const_iterator position, value_type x) -> iterator;
  constexpr auto pop_front() noexcept -> void;
  constexpr auto pop_back() noexcept -> void;
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

As [[set]](https://eel.is/c++draft/set) for `set<Key, Compare>`, over the blocks of `Blocks` as
[[flat.set]](https://eel.is/c++draft/flat.set) is over its `KeyContainer`, with:

- `bit_set` is a set of `size_t` in a `vector<size_t>`, and `basic_bit_set<Key, Blocks, N, KeyMapping, Compare>` a set
  of `Key` whose positions `KeyMapping` names, in the blocks of `Blocks`. `Blocks` and the cap `N` are as
  [`basic_bit_vector`](../bit_vector/bit_vector.md)'s; `bit_fixed_set` and `bit_inplace_set` name the standard ones.
  Where `Blocks` has an allocator, each constructor of [[set.cons]](https://eel.is/c++draft/set.cons) has its
  allocator-extended form; where it has none, there is no allocator argument.
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
- An iterator pair is taken where [[set.cons]](https://eel.is/c++draft/set.cons) and `insert` take one: an
  `InputIterator` qualifies by its `iterator_category`, as
  [[container.requirements.general]](https://eel.is/c++draft/container.requirements.general) has it, so a C++17 input
  iterator with no default constructor or no `const` `operator*` is taken, and so is a C++20 iterator with its sentinel.
- `size()` counts the keys, and runs in constant time, the count being kept beside the blocks. A key past the width
  grows the width; the width is capacity and never value.
- `max_size()` answers the keys the storage could hold, and no more than `KeyMapping::size` where the mapping names
  one.
- `front()` and `back()` return the first and the last key in the set's order, and `pop_front()` and `pop_back()`
  remove them; *Preconditions:* `not empty()`, as for [[deque]](https://eel.is/c++draft/deque)'s. Each pop clears one
  bit in the first block that holds a key, as a walk over the set steps, so `front()` then `pop_front()` compiles to an
  integer's `countr_zero` and `w & (w - 1)` over one block. On a view they write through, where the storage lets the
  view write.
- **Out of range.** `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range` and `erase(const
  key_type&)` are total, as on `set`: a key outside the universe, or a value that is no key at all, is absent, and
  where `Key` is totally ordered the bounds place it among the keys by that order. `insert`, `emplace` and the
  ranged forms throw `out_of_range` for a value the mapping ranks at or past `KeyMapping::size`; a key of an open
  universe throws `length_error` past what the storage can count.
- **Unchecked.** The operators are the unchecked doors, as `bitset`'s `operator[]` is beside `set`, and the named
  members and `xstd::shl` and `xstd::shr` the checked or total ones. For the operators with an enumerator, `|=`,
  `^=`, `-=`, `&=` and the non-members over them, and for `operator[](const key_type&) const`, which returns `bool`,
  a position below `max_size()` is the precondition; for `<<= n` and `>>= n`, `n < max_size()` is. At a run-time
  width a position past the current width reads as absent, and `|=` and `^=` grow to it, the width being capacity
  and never value. Each writes as the storage's `set`, `flip`, `reset`, one-position `&=` or shift does, and a
  hardened build checks the precondition.
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
  and under an unequal one it copies them into `a`'s. The guides deduce the storage, capped at its constant capacity
  where it has one.
- *`replace`:* *Effects:* Moves `blocks` in, the keys becoming those whose positions are set. *`extract`:*
  *Returns:* the blocks, moved out. *Postconditions:* `empty()`. A set loses nothing in the round trip, its width
  being capacity.
- The guides of [[set.overview]](https://eel.is/c++draft/set.overview) deduce `Key` as `size_t` and a `vector` over
  the block of the allocator, `vector<size_t>` where none is given.
- **Enumerator operands.** Where `Key` is an enumeration and `KeyMapping` does not read it as a mask, an enumerator
  is the one-element set holding it: `&=`, `|=`, `^=` and `-=` take one on the right, and on an owner `&`, `|`, `^`
  and `-` take one on either side. The enumeration itself gains no operator. They are unchecked, as above.
- The Boost.Hash2 hook appends the keys in iteration order, then their count, exactly as Hash2 writes the
  `set<Key, Compare>` of the same keys.
