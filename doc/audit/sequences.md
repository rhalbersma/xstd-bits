# Sequence containers audit

Every numbered paragraph of `[sequence.reqmts]`, of the leaf clauses of `[array]`, `[vector]`, `[vector.bool]` and `[inplace.vector]`, and of `[depr.vector.bool.swap]`, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/containers.tex), `source/containers.tex`, `\rSec2[sequence.reqmts]`, `\rSec2[array]`, `\rSec2[vector]`, `\rSec2[vector.bool]` and `\rSec2[inplace.vector]`; and [`source/future.tex`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/future.tex), `\rSec2[depr.vector.bool.swap]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/sequence.reqmts), generated from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same paragraph numbers in all twenty-three clauses.
- **Subjects:** `test::spec::sequence::all`, each column behind its model: `std::array<bool, N>` and the fixed sequences for `[array]`, `std::vector<bool>` and the dynamic and small sequences for `[vector]` and `[vector.bool]`, and `std::inplace_vector<bool, N>`, where the standard library has it, and the bounded sequences for `[inplace.vector]`.
- **Not audited:** `[array.syn]`, `[vector.syn]` and `[inplace.vector.syn]` say what the headers declare, each declaration audited where it is specified; `[deque]`, `[forward.list]`, `[hive]` and `[list]` have no column.

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
| `[sequence.reqmts]` | 1 | — | — | no-requirement | Names the sequence containers; array's limited sequence operations are `[array.overview]/3`'s. |
| `[sequence.reqmts]` | 2 | — | — | no-requirement | Names the variables the expressions below use. |
| `[sequence.reqmts]` | 3 | — | — | no-requirement | Says the complexities are each sequence's own; each is audited where stated. |
| `[sequence.reqmts]` | 4 | — | — | no-requirement | Introduces the requirements; each expression is audited in its own paragraphs. |
| `[sequence.reqmts]` | 5 | `X u(n, t);` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 6 | `X u(n, t);` | Effects | answered |  |
| `[sequence.reqmts]` | 7 | `X u(n, t);` | Postconditions | answered |  |
| `[sequence.reqmts]` | 8 | `X u(i, j);` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 9 | `X u(i, j);` | Effects | answered | Checked for the value and, through a view that counts its reads, for one read of each element. |
| `[sequence.reqmts]` | 10 | `X u(i, j);` | Postconditions | answered |  |
| `[sequence.reqmts]` | 11 | `X(from_range, rg)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 12 | `X(from_range, rg)` | Effects | answered | Checked for the value and, through a view that counts its reads, for one read of each element. |
| `[sequence.reqmts]` | 13 | `X(from_range, rg)` | Recommended practice | no-requirement | Recommended practice, which a conforming implementation may leave aside. |
| `[sequence.reqmts]` | 14 | `X(from_range, rg)` | Postconditions | answered |  |
| `[sequence.reqmts]` | 15 | `X(il)` | Effects | answered |  |
| `[sequence.reqmts]` | 16 | `a = il` | Result | answered |  |
| `[sequence.reqmts]` | 17 | `a = il` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 18 | `a = il` | Effects | answered |  |
| `[sequence.reqmts]` | 19 | `a = il` | Returns | answered |  |
| `[sequence.reqmts]` | 20 | `a.emplace(p, args)` | Result | answered |  |
| `[sequence.reqmts]` | 21 | `a.emplace(p, args)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 22 | `a.emplace(p, args)` | Effects | answered |  |
| `[sequence.reqmts]` | 23 | `a.emplace(p, args)` | Returns | answered |  |
| `[sequence.reqmts]` | 24 | `a.insert(p, t)` | Result | answered |  |
| `[sequence.reqmts]` | 25 | `a.insert(p, t)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 26 | `a.insert(p, t)` | Effects | answered |  |
| `[sequence.reqmts]` | 27 | `a.insert(p, t)` | Returns | answered |  |
| `[sequence.reqmts]` | 28 | `a.insert(p, rv)` | Result | answered |  |
| `[sequence.reqmts]` | 29 | `a.insert(p, rv)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 30 | `a.insert(p, rv)` | Effects | answered |  |
| `[sequence.reqmts]` | 31 | `a.insert(p, rv)` | Returns | answered |  |
| `[sequence.reqmts]` | 32 | `a.insert(p, n, t)` | Result | answered |  |
| `[sequence.reqmts]` | 33 | `a.insert(p, n, t)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 34 | `a.insert(p, n, t)` | Effects | answered |  |
| `[sequence.reqmts]` | 35 | `a.insert(p, n, t)` | Returns | answered |  |
| `[sequence.reqmts]` | 36 | `a.insert(p, i, j)` | Result | answered |  |
| `[sequence.reqmts]` | 37 | `a.insert(p, i, j)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 38 | `a.insert(p, i, j)` | Effects | answered | Checked through a view that counts its reads. |
| `[sequence.reqmts]` | 39 | `a.insert(p, i, j)` | Returns | answered |  |
| `[sequence.reqmts]` | 40 | `a.insert_range(p, rg)` | Result | answered |  |
| `[sequence.reqmts]` | 41 | `a.insert_range(p, rg)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 42 | `a.insert_range(p, rg)` | Effects | answered | Checked through a view that counts its reads. |
| `[sequence.reqmts]` | 43 | `a.insert_range(p, rg)` | Returns | answered |  |
| `[sequence.reqmts]` | 44 | `a.insert(p, il)` | Effects | answered |  |
| `[sequence.reqmts]` | 45 | `a.erase(q)` | Result | answered |  |
| `[sequence.reqmts]` | 46 | `a.erase(q)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 47 | `a.erase(q)` | Effects | answered |  |
| `[sequence.reqmts]` | 48 | `a.erase(q)` | Returns | answered |  |
| `[sequence.reqmts]` | 49 | `a.erase(q1, q2)` | Result | answered |  |
| `[sequence.reqmts]` | 50 | `a.erase(q1, q2)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 51 | `a.erase(q1, q2)` | Effects | answered |  |
| `[sequence.reqmts]` | 52 | `a.erase(q1, q2)` | Returns | answered |  |
| `[sequence.reqmts]` | 53 | `a.clear()` | Result | answered |  |
| `[sequence.reqmts]` | 54 | `a.clear()` | Effects | answered | Checked as no element left; what it invalidates a valid program cannot observe. |
| `[sequence.reqmts]` | 55 | `a.clear()` | Postconditions | answered |  |
| `[sequence.reqmts]` | 56 | `a.clear()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[sequence.reqmts]` | 57 | `a.assign(i, j)` | Result | answered |  |
| `[sequence.reqmts]` | 58 | `a.assign(i, j)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 59 | `a.assign(i, j)` | Effects | answered |  |
| `[sequence.reqmts]` | 60 | `a.assign_range(rg)` | Result | answered |  |
| `[sequence.reqmts]` | 61 | `a.assign_range(rg)` | Mandates | declined | A Mandates, whose violation makes the program ill-formed rather than observable; the cases stay within it. |
| `[sequence.reqmts]` | 62 | `a.assign_range(rg)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 63 | `a.assign_range(rg)` | Effects | answered |  |
| `[sequence.reqmts]` | 64 | `a.assign_range(rg)` | Recommended practice | no-requirement | Recommended practice, which a conforming implementation may leave aside. |
| `[sequence.reqmts]` | 65 | `a.assign(il)` | Effects | answered |  |
| `[sequence.reqmts]` | 66 | `a.assign(n, t)` | Result | answered |  |
| `[sequence.reqmts]` | 67 | `a.assign(n, t)` | Preconditions | no-requirement | A precondition on the program: `bool` is Cpp17CopyInsertable and Cpp17CopyAssignable, and the value passed is never a reference into `a`. |
| `[sequence.reqmts]` | 68 | `a.assign(n, t)` | Effects | answered |  |
| `[sequence.reqmts]` | 69 | `a.assign(n, t)` | — | answered | Checked as two integers taken as a count and a value by the constructor, `assign` and `insert`. The deduction-guide half is not asked. |
| `[sequence.reqmts]` | 70 | `a.assign(n, t)` | — | declined | Introduces the optional operations and bounds them to amortized constant time, which no functional check can observe. |
| `[sequence.reqmts]` | 71 | `a.front()` | Result | answered |  |
| `[sequence.reqmts]` | 72 | `a.front()` | Hardened preconditions | declined | A violation ends the process under a hardened library and an `assert` in ours, so no in-process check can observe it. The cases call only within it. |
| `[sequence.reqmts]` | 73 | `a.front()` | Returns | answered |  |
| `[sequence.reqmts]` | 74 | `a.front()` | Remarks | answered | Checked as the member existing on every column. |
| `[sequence.reqmts]` | 75 | `a.back()` | Result | answered |  |
| `[sequence.reqmts]` | 76 | `a.back()` | Hardened preconditions | declined | A violation ends the process under a hardened library and an `assert` in ours, so no in-process check can observe it. The cases call only within it. |
| `[sequence.reqmts]` | 77 | `a.back()` | Effects | answered |  |
| `[sequence.reqmts]` | 78 | `a.back()` | Remarks | answered | Checked as the member existing on every column. |
| `[sequence.reqmts]` | 79 | `a.emplace_front(args)` | Result | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 80 | `a.emplace_front(args)` | Preconditions | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 81 | `a.emplace_front(args)` | Effects | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 82 | `a.emplace_front(args)` | Returns | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 83 | `a.emplace_front(args)` | Remarks | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 84 | `a.emplace_back(args)` | Result | answered |  |
| `[sequence.reqmts]` | 85 | `a.emplace_back(args)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 86 | `a.emplace_back(args)` | Effects | answered |  |
| `[sequence.reqmts]` | 87 | `a.emplace_back(args)` | Returns | answered |  |
| `[sequence.reqmts]` | 88 | `a.emplace_back(args)` | Remarks | answered | Checked as the member existing on the vector and bounded columns. |
| `[sequence.reqmts]` | 89 | `a.push_front(t)` | Result | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 90 | `a.push_front(t)` | Preconditions | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 91 | `a.push_front(t)` | Effects | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 92 | `a.push_front(t)` | Remarks | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 93 | `a.push_front(rv)` | Result | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 94 | `a.push_front(rv)` | Preconditions | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 95 | `a.push_front(rv)` | Effects | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 96 | `a.push_front(rv)` | Remarks | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 97 | `a.prepend_range(rg)` | Result | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 98 | `a.prepend_range(rg)` | Preconditions | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 99 | `a.prepend_range(rg)` | Effects | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 100 | `a.prepend_range(rg)` | Remarks | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 101 | `a.push_back(t)` | Result | answered | On the vector column; `[inplace.vector.modifiers]/4` gives the bounded column a `reference` instead. |
| `[sequence.reqmts]` | 102 | `a.push_back(t)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 103 | `a.push_back(t)` | Effects | answered |  |
| `[sequence.reqmts]` | 104 | `a.push_back(t)` | Remarks | answered | Checked as the member existing on the vector and bounded columns. |
| `[sequence.reqmts]` | 105 | `a.push_back(rv)` | Result | answered | On the vector column; `[inplace.vector.modifiers]/4` gives the bounded column a `reference` instead. |
| `[sequence.reqmts]` | 106 | `a.push_back(rv)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 107 | `a.push_back(rv)` | Effects | answered |  |
| `[sequence.reqmts]` | 108 | `a.push_back(rv)` | Remarks | answered | Checked as the member existing on the vector and bounded columns. |
| `[sequence.reqmts]` | 109 | `a.append_range(rg)` | Result | answered |  |
| `[sequence.reqmts]` | 110 | `a.append_range(rg)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[sequence.reqmts]` | 111 | `a.append_range(rg)` | Effects | answered | Checked through a view that counts its reads. |
| `[sequence.reqmts]` | 112 | `a.append_range(rg)` | Remarks | answered | Checked as the member existing on the vector and bounded columns. |
| `[sequence.reqmts]` | 113 | `a.pop_front()` | Result | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 114 | `a.pop_front()` | Hardened preconditions | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 115 | `a.pop_front()` | Effects | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 116 | `a.pop_front()` | Remarks | no-requirement | Required only of `deque`, `forward_list` and `list`; no subject provides it. |
| `[sequence.reqmts]` | 117 | `a.pop_back()` | Result | answered |  |
| `[sequence.reqmts]` | 118 | `a.pop_back()` | Hardened preconditions | declined | A violation ends the process under a hardened library and an `assert` in ours, so no in-process check can observe it. The cases call only within it. |
| `[sequence.reqmts]` | 119 | `a.pop_back()` | Effects | answered |  |
| `[sequence.reqmts]` | 120 | `a.pop_back()` | Remarks | answered | Checked as the member existing on the vector and bounded columns. |
| `[sequence.reqmts]` | 121 | `a[n]` | Result | answered |  |
| `[sequence.reqmts]` | 122 | `a[n]` | Hardened preconditions | declined | A violation ends the process under a hardened library and an `assert` in ours, so no in-process check can observe it. The cases call only within it. |
| `[sequence.reqmts]` | 123 | `a[n]` | Effects | answered |  |
| `[sequence.reqmts]` | 124 | `a[n]` | Remarks | answered | Checked as the member existing on every column. |
| `[sequence.reqmts]` | 125 | `a.at(n)` | Result | answered |  |
| `[sequence.reqmts]` | 126 | `a.at(n)` | Returns | answered |  |
| `[sequence.reqmts]` | 127 | `a.at(n)` | Throws | answered |  |
| `[sequence.reqmts]` | 128 | `a.at(n)` | Remarks | answered | Checked as the member existing on every column. |
| `[array.overview]` | 1 | `struct array` | — | answered | A packed column is random access rather than contiguous: a bit has no address, the recorded forced departure. |
| `[array.overview]` | 2 | — | — | answered | Checked as list-initialization. `std::array` is an aggregate; `xstd::bit_array` is not, and takes the list through a constructor. |
| `[array.overview]` | 3 | — | — | answered |  |
| `[array.overview]` | 4 | — | — | gap | `std::array<bool, N>` is a structural type; `xstd::bit_array` is not, its members being private. Whether it should be needs the repo owner. |
| `[array.overview]` | 5 | — | — | answered |  |
| `[array.cons]` | 1 | `array`'s special member functions | — | answered | Checked as the special members being trivial, which only implicit ones over `bool` are. |
| `[array.cons]` | 2 | `array(T, U...) -> array<T, 1 + sizeof...(U)>` | Mandates | declined | A Mandates, whose violation makes the program ill-formed rather than observable; the cases stay within it. `xstd::bit_array` has no such guide: a list of bools cannot deduce a block type. |
| `[array.members]` | 1 | `constexpr size_type size() const noexcept;` | Returns | answered |  |
| `[array.members]` | 2 | `constexpr T* data() noexcept; and 1 more` | Returns | forced | A packed column has no `data()`: a bit has no address, the recorded forced departure. `std::array<bool, N>` and `std::inplace_vector<bool, N>` have one and are asserted to, uncited. |
| `[array.members]` | 3 | `constexpr void fill(const T& u);` | Effects | answered |  |
| `[array.members]` | 4 | `swap(array& y)` | Effects | answered |  |
| `[array.members]` | 5 | `swap(array& y)` | — | no-requirement | A note, which is not normative. |
| `[array.special]` | 1 | `swap(array<T, N>& x, array<T, N>& y)` | Constraints | answered |  |
| `[array.special]` | 2 | `swap(array<T, N>& x, array<T, N>& y)` | Effects | answered |  |
| `[array.special]` | 3 | `swap(array<T, N>& x, array<T, N>& y)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[array.zero]` | 1 | `array<T, 0>` | — | answered |  |
| `[array.zero]` | 2 | — | — | answered |  |
| `[array.zero]` | 3 | — | — | answered |  |
| `[array.creation]` | 1 | `to_array(T (&a)[N])` | Mandates | declined | A Mandates, whose violation makes the program ill-formed rather than observable; the cases stay within it. |
| `[array.creation]` | 2 | `to_array(T (&a)[N])` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[array.creation]` | 3 | `to_array(T (&a)[N])` | Returns | answered | `xstd::to_bit_array` makes an `xstd::bit_array<N>` from a built-in array of `bool`, `const` or not; a built-in array names no block type, so the default one is taken. |
| `[array.creation]` | 4 | `to_array(T (&&a)[N])` | Mandates | declined | A Mandates, whose violation makes the program ill-formed rather than observable; the cases stay within it. |
| `[array.creation]` | 5 | `to_array(T (&&a)[N])` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[array.creation]` | 6 | `to_array(T (&&a)[N])` | Returns | answered | `xstd::to_bit_array`'s rvalue overload, which for `bool` moves exactly what the lvalue one copies. |
| `[array.tuple]` | 1 | `tuple_element<I, array<T, N>>` | Mandates | declined | A Mandates `I < N`, whose violation makes the program ill-formed rather than observable. The cases ask only `I < N`, so a width of nought is asked nothing. |
| `[array.tuple]` | 2 | `get(array<T, N>& a)`, and three more | Mandates | declined | A Mandates `I < N`, whose violation makes the program ill-formed rather than observable. The cases ask only `I < N`, so a width of nought is asked nothing. |
| `[array.tuple]` | 3 | `get(array<T, N>& a)`, and three more | Returns | answered |  |
| `[vector.overview]` | 1 | `class vector` | — | no-requirement | Describes the vector and its costs in prose; each operation is audited where it is specified. |
| `[vector.overview]` | 2 | — | — | answered | Checked as a random access range with an allocator: `vector<bool>` is exempt from contiguity. |
| `[vector.overview]` | 3 | — | — | answered | Asked of the columns whose storage is constant-evaluable. Boost's `small_vector`, under the small columns, is not. |
| `[vector.overview]` | 4 | — | — | no-requirement | Admits an incomplete element type; `bool` is complete. |
| `[vector.cons]` | 1 | `vector(const Allocator&)` | Effects | answered |  |
| `[vector.cons]` | 2 | `vector(const Allocator&)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.cons]` | 3 | `vector(size_type n, const Allocator& = Allocator())` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[vector.cons]` | 4 | `vector(size_type n, const Allocator& = Allocator())` | Effects | answered |  |
| `[vector.cons]` | 5 | `vector(size_type n, const Allocator& = Allocator())` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.cons]` | 6 | `vector(size_type n, const T& value, const Allocator& = Allocator())` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[vector.cons]` | 7 | `vector(size_type n, const T& value, const Allocator& = Allocator())` | Effects | answered |  |
| `[vector.cons]` | 8 | `vector(size_type n, const T& value, const Allocator& = Allocator())` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.cons]` | 9 | `vector(InputIterator first, InputIterator last, const Allocator& = Allocator())` | Effects | answered |  |
| `[vector.cons]` | 10 | `vector(InputIterator first, InputIterator last, const Allocator& = Allocator())` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.cons]` | 11 | `vector(from_range_t, R&& rg, const Allocator& = Allocator())` | Effects | answered |  |
| `[vector.cons]` | 12 | `vector(from_range_t, R&& rg, const Allocator& = Allocator())` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.cons]` | 13 | `vector(from_range_t, R&& rg, const Allocator& = Allocator())` | — | declined | A bound on reallocations, which no functional check can observe. |
| `[vector.capacity]` | 1 | `capacity() const` | Returns | answered |  |
| `[vector.capacity]` | 2 | `capacity() const` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.capacity]` | 3 | `reserve(size_type n)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[vector.capacity]` | 4 | `reserve(size_type n)` | Effects | answered |  |
| `[vector.capacity]` | 5 | `reserve(size_type n)` | Throws | answered | MSVC's `vector<bool>` allocates rather than throwing `length_error`, and is asked for `bad_alloc` there. |
| `[vector.capacity]` | 6 | `reserve(size_type n)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.capacity]` | 7 | `reserve(size_type n)` | Remarks | answered | Checked as the value unchanged; what a reallocation invalidates a valid program cannot observe. |
| `[vector.capacity]` | 8 | `shrink_to_fit()` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[vector.capacity]` | 9 | `shrink_to_fit()` | Effects | answered |  |
| `[vector.capacity]` | 10 | `shrink_to_fit()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.capacity]` | 11 | `shrink_to_fit()` | Remarks | answered | Checked for the half a valid program can see: without a reallocation an iterator taken before still stands at `begin()`. |
| `[vector.capacity]` | 12 | `swap(vector& x)` | Effects | answered | The capacities are asked to change places where the blocks are on the heap. The small columns keep their inline blocks, so there only the contents do: a departure the repo owner should confirm. |
| `[vector.capacity]` | 13 | `swap(vector& x)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.capacity]` | 14 | `resize(size_type sz)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[vector.capacity]` | 15 | `resize(size_type sz)` | Effects | answered |  |
| `[vector.capacity]` | 16 | `resize(size_type sz)` | Remarks | answered | Checked as `resize` past `max_size()` throwing `length_error` with no effect. |
| `[vector.capacity]` | 17 | `resize(size_type sz, const T& c)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[vector.capacity]` | 18 | `resize(size_type sz, const T& c)` | Effects | answered |  |
| `[vector.capacity]` | 19 | `resize(size_type sz, const T& c)` | Remarks | answered |  |
| `[vector.data]` | 1 | `data()`, two overloads | Returns | forced | A packed column has no `data()`: a bit has no address, the recorded forced departure. `std::vector<bool>` has none either. |
| `[vector.data]` | 2 | `data()`, two overloads | Complexity | forced | A packed column has no `data()`: a bit has no address, the recorded forced departure. `std::vector<bool>` has none either. |
| `[vector.modifiers]` | 1 | `insert`, `insert_range`, `emplace`, `emplace_back`, `push_back`, `append_range` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.modifiers]` | 2 | `insert`, `insert_range`, `emplace`, `emplace_back`, `push_back`, `append_range` | Remarks | answered | Checked for what a valid program can see: without a reallocation the iterators before the insertion point stand and read as before. The no-effects clause is asked through `[container.reqmts]/66`'s refused allocation, a `bool` raising nothing of its own. |
| `[vector.modifiers]` | 3 | `insert`, `insert_range`, `emplace`, `emplace_back`, `push_back`, `append_range` | — | declined | A bound on reallocations, which no functional check can observe. |
| `[vector.modifiers]` | 4 | `erase`, two overloads, and `pop_back()` | Effects | answered | Checked as the iterators before the erasure standing and reading as before. |
| `[vector.modifiers]` | 5 | `erase`, two overloads, and `pop_back()` | Throws | answered |  |
| `[vector.modifiers]` | 6 | `erase`, two overloads, and `pop_back()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[vector.erasure]` | 1 | `erase(vector<T, Allocator>& c, const U& value)` | Effects | answered |  |
| `[vector.erasure]` | 2 | `erase_if(vector<T, Allocator>& c, Predicate pred)` | Effects | answered |  |
| `[vector.bool.pspc]` | 1 | `class vector<bool, Allocator>` | — | no-requirement | The synopsis; each member with semantics of its own is audited in its paragraph, and the rest are `[vector]`'s. |
| `[vector.bool.pspc]` | 2 | — | — | no-requirement | Maps the primary template's semantics onto bits; every `[vector]` and `[sequence.reqmts]` check runs on this column. |
| `[vector.bool.pspc]` | 3 | — | — | no-requirement | Permits and recommends a packed representation: a license, not a requirement. |
| `[vector.bool.pspc]` | 4 | — | — | no-requirement | Introduces the proxy; what it does is ¶5-11. |
| `[vector.bool.pspc]` | 5 | `reference::reference(const reference& x)` | Effects | answered |  |
| `[vector.bool.pspc]` | 6 | `reference::~reference()` | Effects | answered |  |
| `[vector.bool.pspc]` | 7 | `reference::operator=`, three overloads | Effects | answered | P2321R2's `const`-qualified overload is checked where the library has it. |
| `[vector.bool.pspc]` | 8 | `reference::operator=`, three overloads | Returns | answered |  |
| `[vector.bool.pspc]` | 9 | `reference::operator bool() const` | Returns | answered |  |
| `[vector.bool.pspc]` | 10 | `reference::flip()` | Effects | answered |  |
| `[vector.bool.pspc]` | 11 | `swap`, three hidden friends | Effects | answered | P3612R1's hidden friends, reached by argument-dependent lookup. |
| `[vector.bool.pspc]` | 12 | `flip()` | Effects | answered |  |
| `[vector.bool.pspc]` | 13 | `hash<vector<bool, Allocator>>` | — | answered |  |
| `[vector.bool.pspc]` | 14 | `is-vector-bool-reference<T>` | — | no-requirement | An exposition-only variable, observed only through `[vector.bool.fmt]`. |
| `[vector.bool.fmt]` | 1 | `formatter::parse(ParseContext& ctx)` | — | answered |  |
| `[vector.bool.fmt]` | 2 | `formatter::format(const T& ref, FormatContext& ctx) const` | — | answered |  |
| `[depr.vector.bool.swap]` | 1 | `static swap(reference x, reference y)` | — | answered | Checked as the static member existing on every column. |
| `[depr.vector.bool.swap]` | 2 | `static swap(reference x, reference y)` | Effects | answered |  |
| `[inplace.vector.overview]` | 1 | `class inplace_vector` | — | answered | A packed column is random access rather than contiguous: a bit has no address, the recorded forced departure. |
| `[inplace.vector.overview]` | 2 | — | — | answered |  |
| `[inplace.vector.overview]` | 3 | — | — | answered | Asked where the bounded columns hold their blocks in `std::inplace_vector`. Before C++26 they use Boost's `static_vector`, which is not constant-evaluable. |
| `[inplace.vector.overview]` | 4 | — | — | answered |  |
| `[inplace.vector.overview]` | 5 | — | — | gap | Holds for `std::inplace_vector<bool, N>`. `xstd::bit_bounded_vector` of capacity nought is neither empty nor trivially copyable, and the trivial copy and move it asks of other capacities hold only over `std::inplace_vector`. Needs the repo owner. |
| `[inplace.vector.cons]` | 1 | `inplace_vector(size_type n)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[inplace.vector.cons]` | 2 | `inplace_vector(size_type n)` | Effects | answered |  |
| `[inplace.vector.cons]` | 3 | `inplace_vector(size_type n)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.cons]` | 4 | `inplace_vector(size_type n, const T& value)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[inplace.vector.cons]` | 5 | `inplace_vector(size_type n, const T& value)` | Effects | answered |  |
| `[inplace.vector.cons]` | 6 | `inplace_vector(size_type n, const T& value)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.cons]` | 7 | `inplace_vector(InputIterator first, InputIterator last)` | Effects | answered |  |
| `[inplace.vector.cons]` | 8 | `inplace_vector(InputIterator first, InputIterator last)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.cons]` | 9 | `inplace_vector(from_range_t, R&& rg)` | Mandates | declined | A Mandates, whose violation makes the program ill-formed rather than observable; the cases stay within it. |
| `[inplace.vector.cons]` | 10 | `inplace_vector(from_range_t, R&& rg)` | Effects | answered |  |
| `[inplace.vector.cons]` | 11 | `inplace_vector(from_range_t, R&& rg)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.capacity]` | 1 | `static capacity()`, `static max_size()` | Returns | answered |  |
| `[inplace.vector.capacity]` | 2 | `resize(size_type sz)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[inplace.vector.capacity]` | 3 | `resize(size_type sz)` | Effects | answered |  |
| `[inplace.vector.capacity]` | 4 | `resize(size_type sz)` | Remarks | answered |  |
| `[inplace.vector.capacity]` | 5 | `resize(size_type sz, const T& c)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[inplace.vector.capacity]` | 6 | `resize(size_type sz, const T& c)` | Effects | answered |  |
| `[inplace.vector.capacity]` | 7 | `resize(size_type sz, const T& c)` | Remarks | answered |  |
| `[inplace.vector.capacity]` | 8 | `static reserve(size_type n)` | Effects | answered |  |
| `[inplace.vector.capacity]` | 9 | `static reserve(size_type n)` | Throws | answered |  |
| `[inplace.vector.capacity]` | 10 | `static shrink_to_fit()` | Effects | answered |  |
| `[inplace.vector.data]` | 1 | `data()`, two overloads | Returns | forced | A packed column has no `data()`: a bit has no address, the recorded forced departure. `std::inplace_vector<bool, N>` has one and is asserted to, uncited. |
| `[inplace.vector.data]` | 2 | `data()`, two overloads | Complexity | forced | A packed column has no `data()`: a bit has no address, the recorded forced departure. `std::inplace_vector<bool, N>` has one and is asserted to, uncited. |
| `[inplace.vector.modifiers]` | 1 | `insert`, `insert_range`, `emplace`, `append_range` | — | no-requirement | Defines the n that ¶3 is stated in. |
| `[inplace.vector.modifiers]` | 2 | `insert`, `insert_range`, `emplace`, `append_range` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.modifiers]` | 3 | `insert`, `insert_range`, `emplace`, `append_range` | Remarks | answered | libstdc++ fills a single pass up to the capacity before it throws, and is asked the weaker remainder of ¶3 there. |
| `[inplace.vector.modifiers]` | 4 | `push_back`, two overloads, and `emplace_back` | Returns | answered |  |
| `[inplace.vector.modifiers]` | 5 | `push_back`, two overloads, and `emplace_back` | Throws | answered |  |
| `[inplace.vector.modifiers]` | 6 | `push_back`, two overloads, and `emplace_back` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.modifiers]` | 7 | `push_back`, two overloads, and `emplace_back` | Remarks | answered |  |
| `[inplace.vector.modifiers]` | 8 | `try_emplace_back`, and `try_push_back`, two overloads | — | no-requirement | Defines the pack that ¶9-10 are stated in. |
| `[inplace.vector.modifiers]` | 9 | `try_emplace_back`, and `try_push_back`, two overloads | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[inplace.vector.modifiers]` | 10 | `try_emplace_back`, and `try_push_back`, two overloads | Effects | answered |  |
| `[inplace.vector.modifiers]` | 11 | `try_emplace_back`, and `try_push_back`, two overloads | Returns | answered | P3981R0's `optional<reference>` is asserted on ours alone: libstdc++ 16 still returns P0843R14's pointer. |
| `[inplace.vector.modifiers]` | 12 | `try_emplace_back`, and `try_push_back`, two overloads | Throws | answered |  |
| `[inplace.vector.modifiers]` | 13 | `try_emplace_back`, and `try_push_back`, two overloads | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.modifiers]` | 14 | `try_emplace_back`, and `try_push_back`, two overloads | Remarks | no-requirement | Initializing a `bool` cannot throw, and ¶12 says nothing else does, so no exception is there to leave no effects. |
| `[inplace.vector.modifiers]` | 15 | `unchecked_emplace_back(Args&&... args)` | Preconditions | no-requirement | A precondition on the caller, `size() < capacity()`, which is not a hardened one; the cases call only with room. |
| `[inplace.vector.modifiers]` | 16 | `unchecked_emplace_back(Args&&... args)` | Effects | answered |  |
| `[inplace.vector.modifiers]` | 17 | `unchecked_push_back`, two overloads | Preconditions | no-requirement | A precondition on the caller, `size() < capacity()`, which is not a hardened one; the cases call only with room. |
| `[inplace.vector.modifiers]` | 18 | `unchecked_push_back`, two overloads | Effects | answered |  |
| `[inplace.vector.modifiers]` | 19 | `erase`, two overloads, and `pop_back()` | Effects | answered | Checked as the iterators before the erasure standing and reading as before. |
| `[inplace.vector.modifiers]` | 20 | `erase`, two overloads, and `pop_back()` | Throws | answered |  |
| `[inplace.vector.modifiers]` | 21 | `erase`, two overloads, and `pop_back()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[inplace.vector.modifiers]` | 22 | `swap(inplace_vector& x)` | Preconditions | no-requirement | A precondition on the element type, which `bool` meets; it asks nothing of the container. |
| `[inplace.vector.modifiers]` | 23 | `swap(inplace_vector& x)` | Effects | answered |  |
| `[inplace.vector.erasure]` | 1 | `erase(inplace_vector<T, N>& c, const U& value)` | Effects | answered |  |
| `[inplace.vector.erasure]` | 2 | `erase_if(inplace_vector<T, N>& c, Predicate pred)` | Effects | answered |  |
