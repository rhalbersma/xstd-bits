# `[set]` audit

Every numbered paragraph of `[associative.reqmts]` and of every leaf clause of `[set]`, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/containers.tex), `source/containers.tex`, `\rSec3[associative.reqmts]` and `\rSec3[set]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/set), generated from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same paragraph numbers in all six clauses.
- **Subjects:** `test::spec::set::all`, the standard library's `std::set<std::size_t>` and, where it has one, `std::flat_set<std::size_t>`, and every xstd set.

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
| `[associative.reqmts.general]` | 1 | — | — | no-requirement | Introduces the four associative containers and the flat adaptors. |
| `[associative.reqmts.general]` | 2 | — | — | no-requirement | Defines `Compare` and the comparison object; the ordering it induces is ¶177-178's. |
| `[associative.reqmts.general]` | 3 | — | — | no-requirement | Defines equivalence of keys, and asks a stable result of the program's comparator, which `std::less` gives. |
| `[associative.reqmts.general]` | 4 | — | — | answered | Checked as one element holding the key after each `insert`. |
| `[associative.reqmts.general]` | 5 | — | — | answered | |
| `[associative.reqmts.general]` | 6 | `iterator`, `const_iterator` | — | answered | Constant: nothing is writable through either, a proxy that converts to the key and a `const Key&` alike. |
| `[associative.reqmts.general]` | 7 | — | — | no-requirement | Notation. |
| `[associative.reqmts.general]` | 8 | — | — | no-requirement | Says a type meets the requirements when each row below does; the allocator-aware part is `[container.alloc.reqmts]`'s own audit. |
| `[associative.reqmts.general]` | 9 | `typename X::key_type` | Result | answered | |
| `[associative.reqmts.general]` | 10 | `typename X::mapped_type` | Result | no-requirement | For `map` and `multimap` only, as ¶11 says. |
| `[associative.reqmts.general]` | 11 | `typename X::mapped_type` | Remarks | no-requirement | Restricts ¶10 to `map` and `multimap`. |
| `[associative.reqmts.general]` | 12 | `typename X::value_type` | Result | answered | |
| `[associative.reqmts.general]` | 13 | `typename X::value_type` | Preconditions | answered | Checked as the key being destructible, which is what `Cpp17Erasable` asks under `std::allocator`. |
| `[associative.reqmts.general]` | 14 | `typename X::key_compare` | Result | answered | |
| `[associative.reqmts.general]` | 15 | `typename X::key_compare` | Preconditions | answered | |
| `[associative.reqmts.general]` | 16 | `typename X::value_compare` | Result | answered | |
| `[associative.reqmts.general]` | 17 | `typename X::node_type` | Result | forced | No nodes: a key is a bit in a block, so there is nothing to unlink and hand over. `erase(k)` removes a key where `extract` would, and `a \|= a2` inserts every key of `a2` where `merge` would, leaving `a2` unchanged. |
| `[associative.reqmts.general]` | 18 | `X(c)` | Effects | answered | |
| `[associative.reqmts.general]` | 19 | `X(c)` | Complexity | declined | Not checked: the tests observe results rather than count steps, and every subject's `key_compare` is `std::less<key_type>`, which gives nothing to count comparisons through. |
| `[associative.reqmts.general]` | 20 | `X u = X(); X u;` | Preconditions | answered | |
| `[associative.reqmts.general]` | 21 | `X u = X(); X u;` | Effects | answered | |
| `[associative.reqmts.general]` | 22 | `X u = X(); X u;` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 23 | `X(i, j, c)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 24 | `X(i, j, c)` | Effects | answered | |
| `[associative.reqmts.general]` | 25 | `X(i, j, c)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 26 | `X(i, j)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 27 | `X(i, j)` | Effects | answered | |
| `[associative.reqmts.general]` | 28 | `X(i, j)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 29 | `X(from_range, rg, c)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 30 | `X(from_range, rg, c)` | Effects | answered | |
| `[associative.reqmts.general]` | 31 | `X(from_range, rg, c)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 32 | `X(from_range, rg)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 33 | `X(from_range, rg)` | Effects | answered | |
| `[associative.reqmts.general]` | 34 | `X(from_range, rg)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 35 | `X(il, c)` | Effects | answered | |
| `[associative.reqmts.general]` | 36 | `X(il)` | Effects | answered | |
| `[associative.reqmts.general]` | 37 | `a = il` | Result | answered | |
| `[associative.reqmts.general]` | 38 | `a = il` | Preconditions | answered | |
| `[associative.reqmts.general]` | 39 | `a = il` | Effects | answered | |
| `[associative.reqmts.general]` | 40 | `a = il` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 41 | `b.key_comp()` | Result | answered | |
| `[associative.reqmts.general]` | 42 | `b.key_comp()` | Returns | answered | `std::less` has no state, so the object is checked by what it answers. |
| `[associative.reqmts.general]` | 43 | `b.key_comp()` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 44 | `b.value_comp()` | Result | answered | |
| `[associative.reqmts.general]` | 45 | `b.value_comp()` | Returns | answered | As ¶42. |
| `[associative.reqmts.general]` | 46 | `b.value_comp()` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 47 | `a_uniq.emplace(args)` | Result | answered | |
| `[associative.reqmts.general]` | 48 | `a_uniq.emplace(args)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 49 | `a_uniq.emplace(args)` | Effects | answered | |
| `[associative.reqmts.general]` | 50 | `a_uniq.emplace(args)` | Returns | answered | |
| `[associative.reqmts.general]` | 51 | `a_uniq.emplace(args)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 52 | `a_eq.emplace(args)` | Result | no-requirement | States `a_eq`, a container with equivalent keys; a set has unique keys, so none of its values is one. |
| `[associative.reqmts.general]` | 53 | `a_eq.emplace(args)` | Preconditions | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 54 | `a_eq.emplace(args)` | Effects | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 55 | `a_eq.emplace(args)` | Returns | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 56 | `a_eq.emplace(args)` | Complexity | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 57 | `a.emplace_hint(p, args)` | Result | answered | |
| `[associative.reqmts.general]` | 58 | `a.emplace_hint(p, args)` | Effects | answered | |
| `[associative.reqmts.general]` | 59 | `a.emplace_hint(p, args)` | Returns | answered | |
| `[associative.reqmts.general]` | 60 | `a.emplace_hint(p, args)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 61 | `a_uniq.insert(t)` | Result | answered | |
| `[associative.reqmts.general]` | 62 | `a_uniq.insert(t)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 63 | `a_uniq.insert(t)` | Effects | answered | |
| `[associative.reqmts.general]` | 64 | `a_uniq.insert(t)` | Returns | answered | |
| `[associative.reqmts.general]` | 65 | `a_uniq.insert(t)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 66 | `a_eq.insert(t)` | Result | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 67 | `a_eq.insert(t)` | Preconditions | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 68 | `a_eq.insert(t)` | Effects | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 69 | `a_eq.insert(t)` | Complexity | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 70 | `a.insert(p, t)` | Result | answered | |
| `[associative.reqmts.general]` | 71 | `a.insert(p, t)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 72 | `a.insert(p, t)` | Effects | answered | |
| `[associative.reqmts.general]` | 73 | `a.insert(p, t)` | Returns | answered | |
| `[associative.reqmts.general]` | 74 | `a.insert(p, t)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 75 | `a.insert(i, j)` | Result | answered | |
| `[associative.reqmts.general]` | 76 | `a.insert(i, j)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 77 | `a.insert(i, j)` | Effects | answered | |
| `[associative.reqmts.general]` | 78 | `a.insert(i, j)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 79 | `a.insert_range(rg)` | Result | answered | Asked of the standard library's sets where `__cpp_lib_containers_ranges` says they have it. |
| `[associative.reqmts.general]` | 80 | `a.insert_range(rg)` | Preconditions | answered | |
| `[associative.reqmts.general]` | 81 | `a.insert_range(rg)` | Effects | answered | |
| `[associative.reqmts.general]` | 82 | `a.insert_range(rg)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 83 | `a.insert(il)` | Effects | answered | |
| `[associative.reqmts.general]` | 84 | `a_uniq.insert(nh)` | Result | forced | As ¶17. |
| `[associative.reqmts.general]` | 85 | `a_uniq.insert(nh)` | Preconditions | forced | As ¶17. |
| `[associative.reqmts.general]` | 86 | `a_uniq.insert(nh)` | Effects | forced | As ¶17. |
| `[associative.reqmts.general]` | 87 | `a_uniq.insert(nh)` | Returns | forced | As ¶17. |
| `[associative.reqmts.general]` | 88 | `a_uniq.insert(nh)` | Complexity | forced | As ¶17. |
| `[associative.reqmts.general]` | 89 | `a_eq.insert(nh)` | Result | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 90 | `a_eq.insert(nh)` | Preconditions | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 91 | `a_eq.insert(nh)` | Effects | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 92 | `a_eq.insert(nh)` | Postconditions | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 93 | `a_eq.insert(nh)` | Complexity | no-requirement | As ¶52. |
| `[associative.reqmts.general]` | 94 | `a.insert(p, nh)` | Result | forced | As ¶17. |
| `[associative.reqmts.general]` | 95 | `a.insert(p, nh)` | Preconditions | forced | As ¶17. |
| `[associative.reqmts.general]` | 96 | `a.insert(p, nh)` | Effects | forced | As ¶17. |
| `[associative.reqmts.general]` | 97 | `a.insert(p, nh)` | Postconditions | forced | As ¶17. |
| `[associative.reqmts.general]` | 98 | `a.insert(p, nh)` | Returns | forced | As ¶17. |
| `[associative.reqmts.general]` | 99 | `a.insert(p, nh)` | Complexity | forced | As ¶17. |
| `[associative.reqmts.general]` | 100 | `a.extract(k)` | Result | forced | As ¶17. |
| `[associative.reqmts.general]` | 101 | `a.extract(k)` | Effects | forced | As ¶17. |
| `[associative.reqmts.general]` | 102 | `a.extract(k)` | Returns | forced | As ¶17. |
| `[associative.reqmts.general]` | 103 | `a.extract(k)` | Complexity | forced | As ¶17. |
| `[associative.reqmts.general]` | 104 | `a_tran.extract(kx)` | Result | forced | As ¶17; nor is any value an `a_tran`, as ¶144 says. |
| `[associative.reqmts.general]` | 105 | `a_tran.extract(kx)` | Effects | forced | As ¶17; nor is any value an `a_tran`, as ¶144 says. |
| `[associative.reqmts.general]` | 106 | `a_tran.extract(kx)` | Returns | forced | As ¶17; nor is any value an `a_tran`, as ¶144 says. |
| `[associative.reqmts.general]` | 107 | `a_tran.extract(kx)` | Complexity | forced | As ¶17; nor is any value an `a_tran`, as ¶144 says. |
| `[associative.reqmts.general]` | 108 | `a.extract(q)` | Result | forced | As ¶17. |
| `[associative.reqmts.general]` | 109 | `a.extract(q)` | Effects | forced | As ¶17. |
| `[associative.reqmts.general]` | 110 | `a.extract(q)` | Returns | forced | As ¶17. |
| `[associative.reqmts.general]` | 111 | `a.extract(q)` | Complexity | forced | As ¶17. |
| `[associative.reqmts.general]` | 112 | `a.merge(a2)` | Result | forced | As ¶17. |
| `[associative.reqmts.general]` | 113 | `a.merge(a2)` | Preconditions | forced | As ¶17. |
| `[associative.reqmts.general]` | 114 | `a.merge(a2)` | Effects | forced | As ¶17. |
| `[associative.reqmts.general]` | 115 | `a.merge(a2)` | Postconditions | forced | As ¶17. |
| `[associative.reqmts.general]` | 116 | `a.merge(a2)` | Throws | forced | As ¶17. |
| `[associative.reqmts.general]` | 117 | `a.merge(a2)` | Complexity | forced | As ¶17. |
| `[associative.reqmts.general]` | 118 | `a.erase(k)` | Result | answered | |
| `[associative.reqmts.general]` | 119 | `a.erase(k)` | Effects | answered | |
| `[associative.reqmts.general]` | 120 | `a.erase(k)` | Returns | answered | |
| `[associative.reqmts.general]` | 121 | `a.erase(k)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 122 | `a_tran.erase(kx)` | Result | declined | As ¶144. |
| `[associative.reqmts.general]` | 123 | `a_tran.erase(kx)` | Effects | declined | As ¶144. |
| `[associative.reqmts.general]` | 124 | `a_tran.erase(kx)` | Returns | declined | As ¶144. |
| `[associative.reqmts.general]` | 125 | `a_tran.erase(kx)` | Complexity | declined | As ¶144. |
| `[associative.reqmts.general]` | 126 | `a.erase(q)` | Result | answered | |
| `[associative.reqmts.general]` | 127 | `a.erase(q)` | Effects | answered | |
| `[associative.reqmts.general]` | 128 | `a.erase(q)` | Returns | answered | |
| `[associative.reqmts.general]` | 129 | `a.erase(q)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 130 | `a.erase(r)` | Result | answered | |
| `[associative.reqmts.general]` | 131 | `a.erase(r)` | Effects | answered | |
| `[associative.reqmts.general]` | 132 | `a.erase(r)` | Returns | answered | |
| `[associative.reqmts.general]` | 133 | `a.erase(r)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 134 | `a.erase(q1, q2)` | Result | answered | |
| `[associative.reqmts.general]` | 135 | `a.erase(q1, q2)` | Effects | answered | |
| `[associative.reqmts.general]` | 136 | `a.erase(q1, q2)` | Returns | answered | |
| `[associative.reqmts.general]` | 137 | `a.erase(q1, q2)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 138 | `a.clear()` | Effects | answered | |
| `[associative.reqmts.general]` | 139 | `a.clear()` | Postconditions | answered | |
| `[associative.reqmts.general]` | 140 | `a.clear()` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 141 | `b.find(k)` | Result | answered | |
| `[associative.reqmts.general]` | 142 | `b.find(k)` | Returns | answered | |
| `[associative.reqmts.general]` | 143 | `b.find(k)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 144 | `a_tran.find(ke)` | Result | declined | `key_compare` is `std::less<key_type>` on every subject and has no `is_transparent`, so no value is an `a_tran`; ¶180 checks that the member templates are absent. |
| `[associative.reqmts.general]` | 145 | `a_tran.find(ke)` | Returns | declined | As ¶144. |
| `[associative.reqmts.general]` | 146 | `a_tran.find(ke)` | Complexity | declined | As ¶144. |
| `[associative.reqmts.general]` | 147 | `b.count(k)` | Result | answered | |
| `[associative.reqmts.general]` | 148 | `b.count(k)` | Returns | answered | |
| `[associative.reqmts.general]` | 149 | `b.count(k)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 150 | `a_tran.count(ke)` | Result | declined | As ¶144. |
| `[associative.reqmts.general]` | 151 | `a_tran.count(ke)` | Returns | declined | As ¶144. |
| `[associative.reqmts.general]` | 152 | `a_tran.count(ke)` | Complexity | declined | As ¶144. |
| `[associative.reqmts.general]` | 153 | `b.contains(k)` | Result | answered | |
| `[associative.reqmts.general]` | 154 | `b.contains(k)` | Effects | answered | |
| `[associative.reqmts.general]` | 155 | `a_tran.contains(ke)` | Result | declined | As ¶144. |
| `[associative.reqmts.general]` | 156 | `a_tran.contains(ke)` | Effects | declined | As ¶144. |
| `[associative.reqmts.general]` | 157 | `b.lower_bound(k)` | Result | answered | |
| `[associative.reqmts.general]` | 158 | `b.lower_bound(k)` | Returns | answered | |
| `[associative.reqmts.general]` | 159 | `b.lower_bound(k)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 160 | `a_tran.lower_bound(kl)` | Result | declined | As ¶144. |
| `[associative.reqmts.general]` | 161 | `a_tran.lower_bound(kl)` | Returns | declined | As ¶144. |
| `[associative.reqmts.general]` | 162 | `a_tran.lower_bound(kl)` | Complexity | declined | As ¶144. |
| `[associative.reqmts.general]` | 163 | `b.upper_bound(k)` | Result | answered | |
| `[associative.reqmts.general]` | 164 | `b.upper_bound(k)` | Returns | answered | |
| `[associative.reqmts.general]` | 165 | `b.upper_bound(k)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 166 | `a_tran.upper_bound(ku)` | Result | declined | As ¶144. |
| `[associative.reqmts.general]` | 167 | `a_tran.upper_bound(ku)` | Returns | declined | As ¶144. |
| `[associative.reqmts.general]` | 168 | `a_tran.upper_bound(ku)` | Complexity | declined | As ¶144. |
| `[associative.reqmts.general]` | 169 | `b.equal_range(k)` | Result | answered | |
| `[associative.reqmts.general]` | 170 | `b.equal_range(k)` | Effects | answered | |
| `[associative.reqmts.general]` | 171 | `b.equal_range(k)` | Complexity | declined | As ¶19. |
| `[associative.reqmts.general]` | 172 | `a_tran.equal_range(ke)` | Result | declined | As ¶144. |
| `[associative.reqmts.general]` | 173 | `a_tran.equal_range(ke)` | Effects | declined | As ¶144. |
| `[associative.reqmts.general]` | 174 | `a_tran.equal_range(ke)` | Complexity | declined | As ¶144. |
| `[associative.reqmts.general]` | 175 | — | — | forced | Different invalidation: an iterator is the set and a position, so no insert or erase moves one to another key, but a run-time width's `end()` is its width, and an insert that grows the width leaves an earlier `end()` on a position. |
| `[associative.reqmts.general]` | 176 | — | — | forced | As ¶17. |
| `[associative.reqmts.general]` | 177 | — | — | answered | |
| `[associative.reqmts.general]` | 178 | — | — | answered | |
| `[associative.reqmts.general]` | 179 | — | — | declined | `std::less<key_type>` has no state, so neither a stored reference to it nor which object a copy compares with can be observed. |
| `[associative.reqmts.general]` | 180 | — | — | answered | Checked with a key that orders against `std::size_t` without converting to it. |
| `[associative.reqmts.general]` | 181 | — | — | answered | Asked of the two class templates with guides: `std::set` and `xstd::basic_bit_set`. The others fix a width or a capacity in the type, which no argument deduces. |
| `[associative.reqmts.except]` | 1 | `clear()`, `erase(k)` | — | answered | |
| `[associative.reqmts.except]` | 2 | `insert`, `emplace` | — | answered | Checked on a key past what the set can hold, which a static width refuses with `out_of_range` and a run-time one with `length_error`; the standard library's sets hold it. |
| `[associative.reqmts.except]` | 3 | `swap` | — | answered | Checked as no swap throwing, and as `noexcept` on every column for the member, the free function and `std::ranges::swap`. |
| `[set.overview]` | 1 | `class set` | — | answered | |
| `[set.overview]` | 2 | `class set` | — | answered | Each requirements clause it names is audited on its own; checked here are `key_type` and `value_type` both being the key, and `insert` answering as `a_uniq` does. |
| `[set.overview]` | 3 | `iterator`, `const_iterator` | — | answered | Walked in a constant expression where one can hold the set: at a static width, at a static capacity where `std::inplace_vector` holds the blocks, at a run-time width, and `std::set` from `__cpp_lib_constexpr_set`. Elsewhere walked at run time. |
| `[set.cons]` | 1 | `explicit set(const Compare& comp, const Allocator& = Allocator())` | Effects | answered | |
| `[set.cons]` | 2 | `explicit set(const Compare& comp, const Allocator& = Allocator())` | Complexity | declined | As `[associative.reqmts.general]/19`. |
| `[set.cons]` | 3 | `set(InputIterator first, InputIterator last, const Compare& comp = Compare(), ...)` | Effects | answered | |
| `[set.cons]` | 4 | `set(InputIterator first, InputIterator last, const Compare& comp = Compare(), ...)` | Complexity | declined | As ¶2. |
| `[set.cons]` | 5 | `set(from_range_t, R&& rg, const Compare& comp = Compare(), ...)` | Effects | answered | |
| `[set.cons]` | 6 | `set(from_range_t, R&& rg, const Compare& comp = Compare(), ...)` | Complexity | declined | As ¶2. |
| `[set.modifiers]` | 1 | `template<class K> pair<iterator, bool> insert(K&& x)`, `template<class K> iterator insert(const_iterator hint, K&& x)` | Constraints | answered | |
| `[set.modifiers]` | 2 | as ¶1 | Preconditions | declined | P2363R5's heterogeneous `insert` is constrained on `Compare::is_transparent`, which `std::less<key_type>` does not have; ¶1 checks both overloads are absent, so there is no call to observe. |
| `[set.modifiers]` | 3 | as ¶1 | Effects | declined | As ¶2. |
| `[set.modifiers]` | 4 | as ¶1 | Returns | declined | As ¶2. |
| `[set.modifiers]` | 5 | as ¶1 | Complexity | declined | As ¶2. |
| `[set.erasure]` | 1 | `erase_if(set& c, Predicate pred)` | Effects | answered | Checked against the standard library's `erase_if` over a `std::set` of the same keys. |
