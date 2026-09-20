# CLAUDE.md

[CONTRIBUTING.md](CONTRIBUTING.md) is the authority on what a PR must pass. This file is the
authority on how the code reads, which no gate checks.

## Comments

- **One line each.** A comment is a single `//` line. Never continue it onto a second. A note that
  will not fit is carrying something that belongs in a name, a type, or a document; put it there
  rather than wrapping it across lines.
- **120 columns, indentation included.** `.clang-format` sets `ColumnLimit: 0` and will not wrap
  for you, so this is yours to hold.
- **Self-contained.** Name no other file, issue or pull request, and no other comment — not
  design.md, not "as above", not "which X already records". A reader who has only the lines in
  front of them must be able to act on it.
- **Prefer a name.** Rename the variable, extract the function, or tighten the type before reaching
  for prose. The best comment is the one a good name made unnecessary.

Shorten to the claim the code cannot make for itself. A measurement, a standard citation or a
rejected alternative earns its line when it says why this code is the way it is; the reasoning that
led there belongs in a document or in the commit that made the change.

## Trailing return types

Every function starts with `auto` and states its return type after the parameter list, `-> void`
included. No leading return types, `main` included.

Where the arrow goes depends on what follows it.

**Same line**, when the declaration is the whole thing — a defaulted or deleted member, or a lambda:

```cpp
[[nodiscard]] friend constexpr auto operator==(T const&, T const&) noexcept -> bool = default;
auto operator=(T const&) -> T& = delete;
```

**Indented new line**, when a body follows:

```cpp
[[nodiscard]] constexpr auto offset() const noexcept
        -> std::size_t
{
        return m_offset;
}
```
