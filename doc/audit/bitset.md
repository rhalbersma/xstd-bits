# `[bitset]` audit

Every numbered paragraph of every leaf clause of `[bitset]`, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/utilities.tex), `source/utilities.tex`, `\rSec1[bitset]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/bitset), generated on 2026-08-23 from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same paragraph numbers in all six clauses.
- **Subjects:** `test::spec::bitset::all`, the standard library's `std::bitset`, `boost::dynamic_bitset`, and every xstd bitset.

A row's `outcome` is one of:

| outcome | meaning |
| :--- | :--- |
| `answered` | a test cites the paragraph |
| `declined` | deliberately not answered; the note says why |
| `forced` | a recorded departure the packing forces; the note says what is done instead |
| `no-requirement` | introductory or descriptive text with nothing to test; the note says why |
| `gap` | testable, and not yet cited |

| clause | ¶ | declaration | element | outcome | note |
| :--- | ---: | :--- | :--- | :--- | :--- |
| `[bitset.syn]` | 1 | `<bitset>` | — | no-requirement | Says what the header declares; each declaration is audited in the clause that specifies it. |
| `[template.bitset.general]` | 1 | `class bitset` | — | answered | Checked as a fixed number of bits: a static width has none of the members `boost::dynamic_bitset` grows and shrinks by, and every run-time width has all of them. The width itself is `size()`'s Returns in `[bitset.members]`. |
| `[template.bitset.general]` | 2 | `class bitset` | — | no-requirement | Defines set, reset, toggle and bit value, the terms `[bitset.cons]/2` and `[bitset.members]/37-40` are stated in and checked by. |
| `[template.bitset.general]` | 3 | `class reference` | — | no-requirement | Introduces the proxy; what it does is ¶4-12. |
| `[template.bitset.general]` | 4 | `reference(const reference& x)` | Effects | answered | |
| `[template.bitset.general]` | 5 | `~reference()` | Effects | answered | "None" is checked as the bitset being unchanged once a proxy is destroyed. |
| `[template.bitset.general]` | 6 | `reference::operator=`, three overloads | Effects | answered | The `const`-qualified overload is checked where the proxy has it: ours and libc++'s. |
| `[template.bitset.general]` | 7 | `reference::operator=`, three overloads | Returns | answered | |
| `[template.bitset.general]` | 8 | `reference::operator bool()` | Returns | answered | |
| `[template.bitset.general]` | 9 | `reference::operator~()` | Returns | answered | |
| `[template.bitset.general]` | 10 | `swap`, three hidden friends | Effects | answered | Checked where the proxy has them: ours and libc++'s. libstdc++'s and boost's pass vacuously. |
| `[template.bitset.general]` | 11 | `reference::flip()` | Effects | answered | |
| `[template.bitset.general]` | 12 | `reference::flip()` | Returns | answered | libc++ gives `std::bitset` `vector<bool>`'s proxy, whose `flip` returns `void`; checked on the others. |
| `[template.bitset.general]` | 13 | — | — | no-requirement | Names the three error kinds and their exception types; each is checked by the Throws paragraph that raises it. |
| `[bitset.cons]` | 1 | `bitset()` | Effects | answered | |
| `[bitset.cons]` | 2 | `bitset(unsigned long long val)` | Effects | answered | |
| `[bitset.cons]` | 3 | `bitset(const basic_string&, ...)`, `bitset(basic_string_view, ...)` | Effects | answered | |
| `[bitset.cons]` | 4 | `bitset(const basic_string&, ...)`, `bitset(basic_string_view, ...)` | Effects | answered | |
| `[bitset.cons]` | 5 | `bitset(const basic_string&, ...)`, `bitset(basic_string_view, ...)` | Effects | answered | At a static width; a run-time width takes `rlen` as its width, so `M == N` there. |
| `[bitset.cons]` | 6 | `bitset(const basic_string&, ...)`, `bitset(basic_string_view, ...)` | Effects | answered | Through traits that compare letters regardless of case. Not asked of `boost::dynamic_bitset`, which takes no `zero` and `one`. |
| `[bitset.cons]` | 7 | `bitset(const basic_string&, ...)`, `bitset(basic_string_view, ...)` | Throws | answered | `boost::dynamic_bitset` asserts where the standard throws. libstdc++ before 16 checks only the `N` characters it stores. |
| `[bitset.cons]` | 8 | `bitset(const charT* str, ...)` | Constraints | answered | LWG 4294. The refusal is asked of `std::bitset` only from libstdc++ 16 and libc++ 23, where it is stated. |
| `[bitset.cons]` | 9 | `bitset(const charT* str, ...)` | Effects | answered | |
| `[bitset.members]` | 1 | `operator&=(const bitset& rhs)` | Effects | answered | |
| `[bitset.members]` | 2 | `operator&=(const bitset& rhs)` | Returns | answered | |
| `[bitset.members]` | 3 | `operator\|=(const bitset& rhs)` | Effects | answered | |
| `[bitset.members]` | 4 | `operator\|=(const bitset& rhs)` | Returns | answered | |
| `[bitset.members]` | 5 | `operator^=(const bitset& rhs)` | Effects | answered | |
| `[bitset.members]` | 6 | `operator^=(const bitset& rhs)` | Returns | answered | |
| `[bitset.members]` | 7 | `operator<<=(size_t pos)` | Effects | answered | |
| `[bitset.members]` | 8 | `operator<<=(size_t pos)` | Returns | answered | |
| `[bitset.members]` | 9 | `operator>>=(size_t pos)` | Effects | answered | |
| `[bitset.members]` | 10 | `operator>>=(size_t pos)` | Returns | answered | |
| `[bitset.members]` | 11 | `operator<<(size_t pos) const` | Returns | answered | |
| `[bitset.members]` | 12 | `operator>>(size_t pos) const` | Returns | answered | |
| `[bitset.members]` | 13 | `set()` | Effects | answered | |
| `[bitset.members]` | 14 | `set()` | Returns | answered | |
| `[bitset.members]` | 15 | `set(size_t pos, bool val = true)` | Effects | answered | |
| `[bitset.members]` | 16 | `set(size_t pos, bool val = true)` | Returns | answered | |
| `[bitset.members]` | 17 | `set(size_t pos, bool val = true)` | Throws | answered | At a static width; a run-time width asserts, as `boost::dynamic_bitset`'s contract has it. |
| `[bitset.members]` | 18 | `reset()` | Effects | answered | |
| `[bitset.members]` | 19 | `reset()` | Returns | answered | |
| `[bitset.members]` | 20 | `reset(size_t pos)` | Effects | answered | |
| `[bitset.members]` | 21 | `reset(size_t pos)` | Returns | answered | |
| `[bitset.members]` | 22 | `reset(size_t pos)` | Throws | answered | At a static width, as ¶17. |
| `[bitset.members]` | 23 | `operator~() const` | Effects | answered | |
| `[bitset.members]` | 24 | `operator~() const` | Returns | answered | |
| `[bitset.members]` | 25 | `flip()` | Effects | answered | |
| `[bitset.members]` | 26 | `flip()` | Returns | answered | |
| `[bitset.members]` | 27 | `flip(size_t pos)` | Effects | answered | |
| `[bitset.members]` | 28 | `flip(size_t pos)` | Returns | answered | |
| `[bitset.members]` | 29 | `flip(size_t pos)` | Throws | answered | At a static width, as ¶17. |
| `[bitset.members]` | 30 | `operator[](size_t pos) const` | Hardened preconditions | declined | A violation ends the process, under an `assert` in ours and a hardened standard library, so no in-process check can observe it. The cases call only with `pos < size()`. |
| `[bitset.members]` | 31 | `operator[](size_t pos) const` | Returns | answered | |
| `[bitset.members]` | 32 | `operator[](size_t pos) const` | Throws | answered | |
| `[bitset.members]` | 33 | `operator[](size_t pos)` | Hardened preconditions | declined | As ¶30. |
| `[bitset.members]` | 34 | `operator[](size_t pos)` | Returns | answered | |
| `[bitset.members]` | 35 | `operator[](size_t pos)` | Throws | answered | |
| `[bitset.members]` | 36 | `operator[](size_t pos)` | Remarks | no-requirement | Lets an access through the proxy touch the whole bitset for data-race purposes: a license to the implementation that a single thread cannot observe. |
| `[bitset.members]` | 37 | `to_ulong() const` | Returns | answered | |
| `[bitset.members]` | 38 | `to_ulong() const` | Throws | answered | |
| `[bitset.members]` | 39 | `to_ullong() const` | Returns | answered | Not asked of `boost::dynamic_bitset`, which has no `to_ullong`. |
| `[bitset.members]` | 40 | `to_ullong() const` | Throws | answered | As ¶39. |
| `[bitset.members]` | 41 | `to_string(charT zero, charT one) const` | Effects | answered | `boost::dynamic_bitset` is asked through its free `to_string`, in `char` only. |
| `[bitset.members]` | 42 | `to_string(charT zero, charT one) const` | Returns | answered | |
| `[bitset.members]` | 43 | `count() const` | Returns | answered | |
| `[bitset.members]` | 44 | `size() const` | Returns | answered | At a static width; a run-time width is the one the bitset was given. |
| `[bitset.members]` | 45 | `operator==(const bitset& rhs) const` | Returns | answered | `noexcept` is asserted of every implementation but `boost::dynamic_bitset`, whose `==` declares none. |
| `[bitset.members]` | 46 | `test(size_t pos) const` | Returns | answered | |
| `[bitset.members]` | 47 | `test(size_t pos) const` | Throws | answered | At a static width, as ¶17. |
| `[bitset.members]` | 48 | `all() const` | Returns | answered | |
| `[bitset.members]` | 49 | `any() const` | Returns | answered | |
| `[bitset.members]` | 50 | `none() const` | Returns | answered | |
| `[bitset.hash]` | 1 | `hash<bitset<N>>` | — | answered | Enabled at a static width; `boost::dynamic_bitset`'s own contract says nothing of `std::hash`. |
| `[bitset.operators]` | 1 | `operator&(const bitset<N>&, const bitset<N>&)` | Returns | answered | |
| `[bitset.operators]` | 2 | `operator\|(const bitset<N>&, const bitset<N>&)` | Returns | answered | |
| `[bitset.operators]` | 3 | `operator^(const bitset<N>&, const bitset<N>&)` | Returns | answered | |
| `[bitset.operators]` | 4 | `operator>>(basic_istream&, bitset<N>&)` | — | answered | A formatted input function, checked as leading whitespace skipped. |
| `[bitset.operators]` | 5 | `operator>>(basic_istream&, bitset<N>&)` | Effects | answered | |
| `[bitset.operators]` | 6 | `operator>>(basic_istream&, bitset<N>&)` | — | answered | |
| `[bitset.operators]` | 7 | `operator>>(basic_istream&, bitset<N>&)` | Returns | answered | |
| `[bitset.operators]` | 8 | `operator<<(basic_ostream&, const bitset<N>&)` | Returns | answered | |
