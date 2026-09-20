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
- **120 columns, indentation included.** This is a limit on comment lines. `.clang-format` sets
  `ColumnLimit: 0` and will not wrap for you, so this is yours to hold; code lines have no such
  limit and routinely run past it.
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
[[nodiscard]] friend auto operator==(T const&, T const&) noexcept -> bool = default;
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

## What the language already says

Do not write out what a declaration already has.

`constexpr` is not written on a **defaulted or deleted** member. The standard admits it there only
where the function would have been implicitly `constexpr` anyway, so it never carries information.

```cpp
[[nodiscard]] friend auto operator==(T const&, T const&) noexcept -> bool = default;
auto operator=(T const&) -> T& = delete;
```

`noexcept` on a defaulted member is a different matter, and **is** written: since P1286R2 the explicit
specification is honoured rather than making the function deleted, so it can differ from the one the
members imply — which means removing it can change the answer.

```cpp
struct Throwy { Throwy() {} };                     // not noexcept
struct T { Throwy t; T() noexcept = default; };    // noexcept anyway, and not deleted
static_assert(std::is_nothrow_default_constructible_v<T>);
```

A **lambda** is implicitly `constexpr` when it is eligible, so that is not written either. It is also
not implicitly `noexcept`: `static_assert(!noexcept(plain(1)))` holds for a lambda with nothing
written on it. Write `noexcept` on a lambda where it is wanted, and keep it where it is already
there.

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
