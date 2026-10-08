# CLAUDE.md

[CONTRIBUTING.md](CONTRIBUTING.md) is the authority on what a PR must pass, and
[`.clang-format`](.clang-format) on where the code sits on the page. This file is the authority on
how the code reads, which no gate checks. Code satisfies all three; nothing here licenses a
clang-format diff.

## Comments

What follows is about explanatory prose in hand-written C++. A license header, an `#endif` naming
its guard, the note saying what an `#include` was needed for, and a tool directive — `NOLINT`,
`IWYU pragma`, `GCOVR_EXCL_LINE`, `clang-format off` — are none of them prose, and none of them
bound by these rules. Other file types use their own comment syntax and are not covered at all.

- **One line each.** A comment is a single `//` line. Never continue it onto a second. A note that
  will not fit is carrying something that belongs in a name, a type, or a document; put it there
  rather than wrapping it across lines.
- **120 columns, indentation included.** This is a limit on a line that *is* a comment.
  `.clang-format` sets `ColumnLimit: 0` and will not wrap for you, so this is yours to hold. Code
  lines have no such limit and routinely run past it, and a comment trailing one inherits the room
  the code took: what is measured is the comment's own line, never the statement it annotates.
- **Self-contained.** Name no file in this repository, no issue or pull request, and no other
  comment — not design.md, not "as above", not "which X already records". A reader who has only the
  lines in front of them must be able to act on it. A standard clause, a paper number or another
  fixed outside citation is not a reference of that kind: it names the authority for a constraint
  the code cannot otherwise justify, and it stays valid wherever the line ends up.
- **Prefer a name.** Rename the variable, extract the function, or tighten the type before reaching
  for prose. The best comment is the one a good name made unnecessary.
- **Declarative, and about the code as it stands.** Say what the code does, not what it used to do
  or why it changed: no "used to", "no longer", "once answered", "now that X is gone". A reader
  cannot see the version you are contrasting with. History is in the commit logs.

Shorten to the claim the code cannot make for itself. A measurement, a standard citation or a
rejected alternative earns its line when it says why this code is the way it is; the reasoning that
led there belongs in a document or in the commit that made the change.

## Include order

Three groups, in this order, with no blank line between them: this project's own `<xstd/...>`
headers, then third-party ones such as `<boost/...>` and `<absl/...>`, then the standard library.
Alphabetical by path within each group, so `<cstddef>` precedes `<functional>`, and
`<xstd/bits/detail/contiguous_bit_array.hpp>` precedes `<xstd/bits/detail/ownership.hpp>`.

`.clang-format` sets `SortIncludes: Never`, so nothing enforces this and nothing will reorder for
you. A file that already deviates is a file to fix, not the convention speaking.

**An umbrella header is the exception.** Where every include carries `IWYU pragma: export`, the
file publishes a surface rather than naming what it needs, and its order is the one a reader
should meet it in: a macro before what expands it, a type before the functions returning it, and
whatever grouping the comments mark. Alphabetizing that throws away the argument it was making.

A trailing `//` names what each include is for, and that comment, an `IWYU pragma` and a `NOLINT`
belong to the line rather than to the position: move the whole line or none of it. Reordering
changes which header is found first, so build and test afterwards rather than trusting that only
whitespace moved.

## Trailing return types

Every function starts with `auto`. No leading return types, `main` included, which is
`auto main() -> int`. Constructors, destructors, conversion operators and deduction guides have no
return type of their own to write and are exempt.

Where the type is written it goes after the parameter list, never before it. Leaving it to
deduction is allowed, and is the better choice where the return statement states the type more
exactly than a written one would.

Where the arrow goes depends on what follows it.

**Same line**, for a declaration with no compound-statement body — `= default` and `= delete`
included — and for every lambda:

```cpp
[[nodiscard]] friend auto operator==(T const&, T const&) -> bool = default;
auto operator=(T const&) -> T& = delete;
for_each_block(n, len, [&](std::size_t pos, block_type mask) -> void { block_at(pos, ones, mask); });
```

**Indented new line**, in a function definition, where a body follows:

```cpp
[[nodiscard]] constexpr auto offset() const noexcept
        -> std::size_t
{
        return m_offset;
}
```

## Template declarations

A template declaration is followed by a blank line before whatever comes next, as a definition is. The
`template<...>` head and the declaration it introduces read as one unit, and the next declaration starts another:

```cpp
template<bit_block_container_type OtherBits, storage OtherStore, class OtherDerived, class OtherKey, class OtherKeyTraits, class OtherCompare>
friend class set_adaptor;

friend class basic_reference<Value>;
```

`.clang-format` cannot insert this line, so it is yours to keep.

## Size parameters

A template's size parameter is one letter, and the letter is its unit: `N` counts bits, `K` counts blocks, and `E` is
an extent, which may be `std::dynamic_extent`. So `basic_bit_array<Block, N>` is `N` bits wide, and its guide from
`std::array<Block, K>` deduces `K` blocks of them. A second count of the same unit takes the next letter, as a
constructor's `M` beside `N`. A type parameter names what it holds, `Block` for one block, `Blocks` for a range of
them and `Bits` for either, and takes `Other…` where an enclosing template already has the name. A type
transformation's parameter names what it takes, `Owner` for the owning container whose block or width it changes.

## What the language already says

Do not write out what a declaration already has.

`constexpr` is not written on a **defaulted or deleted** member. The standard admits it there only
where the function would have been implicitly `constexpr` anyway, so it never carries information.

```cpp
[[nodiscard]] friend auto operator==(T const&, T const&) -> bool = default;
auto operator=(T const&) -> T& = delete;
```

`noexcept` is not written on a **defaulted** member either. Its exception specification is deduced from
what the definition calls, and a guarantee worth having is asserted in a test, which proves the deduced
answer rather than imposing one that a throwing member would turn into `std::terminate`.

```cpp
[[nodiscard]] T() = default;
static_assert(std::is_nothrow_default_constructible_v<T>);
```

The exception is a member over a dependency that throws nothing without declaring so, such as
`std::vector`'s `==` or `boost::container::static_vector`'s `swap`. There the deduced answer is wrong,
so the right one is written, and a comment names the dependency.

A **lambda** is implicitly `constexpr` when it is eligible, so that is not written either. It is also
not implicitly `noexcept`: `static_assert(!noexcept(plain(1)))` holds for a lambda with nothing
written on it. Write `noexcept` on a lambda where it is wanted, and keep it where it is already
there.

## Loops

A counted loop tests with `<`, never `!=`: an index that overshoots its bound stops rather than runs on. Where
it costs nothing, it is not written as a counter at all.

**Ascending by one** is a range over `iota`, and a closed bound is written as the half-open one it is:

```cpp
for (auto const i : std::views::iota(0UZ, n)) { /* ... */ }
for (auto const i : std::views::iota(lo, hi + 1UZ)) { /* ... */ }
```

A loop that only repeats marks its unread counter `[[maybe_unused]]`, as `-Wunused-variable` asks.

**Strided** stays a raw loop. `views::stride` must never step past the end, so each increment computes
`min(stride, end - current)`, which GCC keeps in the loop and Clang keeps as two `cmov`s that stop it unrolling:

```cpp
for (auto k = 0UZ; k < n; k += digits) { /* ... */ }
```

**Descending** runs from `hi - 1` down to `lo` over `N = hi - lo` indices, and lets unsigned wraparound end it:
one step below `lo`, `i - lo` wraps to the maximum and the `<` test fails. It needs `lo <= hi`, and with `lo == 0`
it is the plain countdown. It runs zero times when `N == 0`.

```cpp
for (auto i = N - 1UZ; i < N; --i) { /* ... */ }
for (auto i = hi - 1UZ, N = hi - lo; i - lo < N; --i) { /* ... */ }
```

`views::reverse` over `iota` compiles to the same loop at `-O2`, but to 2.5–3.7× the code at `-O0` and in constant
evaluation, so it is not used. The wraparound is defined behaviour; only Clang's opt-in `-fsanitize=integer`
reports it. MSVC's `/analyze` does report it as C6293 in code outside a template, and CI fails on that: there,
walk backward with an algorithm such as `std::ranges::copy_backward` rather than suppress the warning.

Iterator loops (`first != last`) and searches that stop at a sentinel such as `npos` are not counted loops, and
compare as their types allow. A loop whose test carries a second condition stays raw.

## `detail` is exposition-only

What sits in a `detail` namespace plays the part the standard gives an exposition-only name: it is specified by
the code that uses it, and no user spells it. Follow the standard's practice with it, and go no further.

- **No public header opens a `detail` namespace.** Machinery goes in a header under a `detail/` directory, and the
  public header includes it. `test/CMakeLists.txt` fails configuration on a public header that does otherwise.
- **A `detail` name may appear wherever the standard puts an exposition-only one:** as a base class, in a member's
  body, on the right-hand side of a concept, in a deduction guide, and in a constraint. `subrange` is constrained on
  *`convertible-to-non-slicing`*, `vector`'s `from_range` constructor on *`container-compatible-range`*, and
  `span`'s deduction guide returns through *`maybe-static-ext`*.
- **A default template argument is public where a user may set a later parameter,** since that spells every one
  before it: `less<Key>`, `char_traits<charT>`, `allocator<T>`, `dynamic_extent`. The standard's one exception,
  `basic_simd`'s *`native-abi<T>`*, sits on the last parameter, which users reach through the `simd<T, N>` alias.
- **A constraint becomes a public concept when users have a reason to name it**, as `bit_convertible` names what
  `bit_convert` takes. A public concept carries semantics: do not promote one to keep a `detail` name out of a
  signature, and do not merge two to save a name.

## Checking your work

Compile with the **stable rung** of [README.md](README.md)'s matrix, never with whatever `g++` or `clang++`
happen to resolve to. Ubuntu 24.04 ships GCC 13, which rejects `-std=c++2c` outright, so the default compiler
cannot build this library at all, and a check that quietly falls back to a cut-down reproduction proves less than
it appears to. [`tools/setup-toolchain.sh`](tools/setup-toolchain.sh) installs the rung;
`XSTD_TOOLCHAIN_FULL=1` adds the qualification rung, which a leg carrying a newer standard library needs.

A comment or formatting change is not exempt, because two gates read lines rather than syntax. `NOLINT` and
`GCOVR_EXCL_LINE` suppress the line they sit on, so a formatter that splits that line leaves the marker behind on
the wrong half, where it goes silent with nothing to say it has. Check that the code is byte-identical when a
change is meant to be comment-only, and that every marker still sits on the line it names.

## `[[nodiscard]]`

Every function that returns a value carries it. Two kinds do not.

**A reference handed back for chaining.** `operator@=` and anything else returning `*this` is meant to
be used or dropped as the caller pleases.

**A by-product the caller may reasonably ignore.** These return something worth having, and calling
them for their effect alone is ordinary use:

- `operator++(int)` and `operator--(int)`, whose old value `it++;` discards by design
- `insert`, `insert_range`, `emplace`, `emplace_hint` and `erase`, whose iterator or
  `pair<iterator, bool>` is a by-product of the modification
- `emplace_back` and the `unchecked_` and `try_` doors, whose reference or `optional<reference>` most
  callers never look at
- `test_set`, whose previous bit is useful and routinely dropped

A function returning `void` needs nothing: there is no result to discard.
