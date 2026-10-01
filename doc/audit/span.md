# `[views.span]` audit

Every numbered paragraph of the leaf clauses of `[views.span]`, and of `[span.objectrep]` beside it, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/containers.tex), `source/containers.tex`, `\rSec3[views.span]` and `\rSec3[span.objectrep]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/views.span), generated from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same paragraph numbers in all eight clauses.
- **Subjects:** `test::spec::span::all`, `std::span<bool>` and `std::span<bool const>` at run-time and static extents first, then `xstd::bit_span` over the whole of an owner's bits and `xstd::bit_subspan` over a window of them, at run-time and static extents, over fixed and growing storage, mutable and const. Each is handed out over an owner that an input keeps alive beside it.
- **Not audited:** `[span.syn]` says what the header declares, each declaration audited where it is specified; `[views.multidim]` has no candidate.

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
| `[span.overview]` | 1 | `span` | — | answered | Checked as a borrowed `std::ranges::view` through whose copy every write is seen. A packed view is random access rather than contiguous: a bit has no address. |
| `[span.overview]` | 2 | `span` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[span.overview]` | 3 | `span` | — | answered |  |
| `[span.overview]` | 4 | `span` | — | no-requirement | A requirement on the element type, which `bool` and `bool const` meet; it asks nothing of the view. |
| `[span.overview]` | 5 | `span` | — | no-requirement | An invalidation rule, which says when a view dangles; no check can observe a dangling view. |
| `[span.cons]` | 1 | `span()` | Constraints | declined | No default constructor: a packed view is always taken over an owner or its blocks, and holds no null storage to stand for none. |
| `[span.cons]` | 2 | `span()` | Postconditions | declined | As ¶1. |
| `[span.cons]` | 3 | `span(It first, size_type count)` | Constraints | forced | No iterator reaches a bit as a `contiguous_iterator`, a bit having no address. A view is taken over an owner, `bit_span(owner)`, or over its blocks, `bit_span(blocks)`. |
| `[span.cons]` | 4 | `span(It first, size_type count)` | Preconditions | forced | As ¶3. |
| `[span.cons]` | 5 | `span(It first, size_type count)` | Hardened preconditions | forced | As ¶3. |
| `[span.cons]` | 6 | `span(It first, size_type count)` | Effects | forced | As ¶3. |
| `[span.cons]` | 7 | `span(It first, size_type count)` | Throws | forced | As ¶3. |
| `[span.cons]` | 8 | `span(It first, End last)` | Constraints | forced | As ¶3. |
| `[span.cons]` | 9 | `span(It first, End last)` | Preconditions | forced | As ¶3. |
| `[span.cons]` | 10 | `span(It first, End last)` | Hardened preconditions | forced | As ¶3. |
| `[span.cons]` | 11 | `span(It first, End last)` | Effects | forced | As ¶3. |
| `[span.cons]` | 12 | `span(It first, End last)` | Throws | forced | As ¶3. |
| `[span.cons]` | 13 | `span(type_identity_t<element_type> (&arr)[N])`, and two more | Constraints | forced | An array of `bool` holds its elements unpacked, one to a byte, and no packed view reads it. The packed counterpart is a view over an array of blocks, `bit_span(blocks)`. |
| `[span.cons]` | 14 | `span(type_identity_t<element_type> (&arr)[N])`, and two more | Effects | forced | As ¶13. |
| `[span.cons]` | 15 | `span(type_identity_t<element_type> (&arr)[N])`, and two more | Postconditions | forced | As ¶13. |
| `[span.cons]` | 16 | `span(R&& r)` | Constraints | forced | A contiguous range of `bool` holds its elements unpacked, and no packed view reads it. The packed counterpart is a view over a contiguous range of blocks, `bit_span(blocks)`. |
| `[span.cons]` | 17 | `span(R&& r)` | Preconditions | forced | As ¶16. |
| `[span.cons]` | 18 | `span(R&& r)` | Hardened preconditions | forced | As ¶16. |
| `[span.cons]` | 19 | `span(R&& r)` | Effects | forced | As ¶16. |
| `[span.cons]` | 20 | `span(R&& r)` | Throws | forced | As ¶16. |
| `[span.cons]` | 21 | `span(const span& other)` | Postconditions | answered | `data() == other.data()` checked as equal iterators and as positions that a write through one shows through the other. |
| `[span.cons]` | 22 | `span(const span<OtherElementType, OtherExtent>& s)` | Constraints | answered | Checked between the extents of `std::span` and of a window. A whole view has one extent, its owner's width, and neither packed view converts from mutable to const bits: a const view is taken over the const owner. |
| `[span.cons]` | 23 | `span(const span<OtherElementType, OtherExtent>& s)` | Hardened preconditions | declined | A violation ends the process under a hardened library and an `assert` in ours, so no in-process check can observe it. The cases convert only at the right size. |
| `[span.cons]` | 24 | `span(const span<OtherElementType, OtherExtent>& s)` | Effects | answered |  |
| `[span.cons]` | 25 | `span(const span<OtherElementType, OtherExtent>& s)` | Postconditions | answered | As ¶21. |
| `[span.cons]` | 26 | `span(const span<OtherElementType, OtherExtent>& s)` | Remarks | answered |  |
| `[span.cons]` | 27 | `operator=(const span& other)` | Postconditions | answered | As ¶21. |
| `[span.deduct]` | 1 | `span(It, EndOrSize)` | Constraints | forced | As `[span.cons]/3`: there is no iterator constructor to deduce from. A packed view deduces from its owner or its blocks instead. |
| `[span.deduct]` | 2 | `span(R&&)` | Constraints | forced | As `[span.cons]/16`: a packed view deduces from its owner or from a contiguous range of blocks. |
| `[span.sub]` | 1 | `first<Count>()` | Mandates | declined | A Mandates, whose violation makes the program ill-formed rather than observable; the cases stay within it. |
| `[span.sub]` | 2 | `first<Count>()` | Hardened preconditions | declined | As `[span.cons]/23`. The cases take only what the view holds. |
| `[span.sub]` | 3 | `first<Count>()` | Effects | answered | Checked as the same positions, read alike and written through, and for the extent of the view handed back. |
| `[span.sub]` | 4 | `last<Count>()` | Mandates | declined | As ¶1. |
| `[span.sub]` | 5 | `last<Count>()` | Hardened preconditions | declined | As ¶2. |
| `[span.sub]` | 6 | `last<Count>()` | Effects | answered | As ¶3. |
| `[span.sub]` | 7 | `subspan<Offset, Count>()` | Mandates | declined | As ¶1. |
| `[span.sub]` | 8 | `subspan<Offset, Count>()` | Hardened preconditions | declined | As ¶2. |
| `[span.sub]` | 9 | `subspan<Offset, Count>()` | Effects | answered | As ¶3. |
| `[span.sub]` | 10 | `subspan<Offset, Count>()` | Remarks | answered |  |
| `[span.sub]` | 11 | `first(size_type count)` | Hardened preconditions | declined | As ¶2. |
| `[span.sub]` | 12 | `first(size_type count)` | Effects | answered | As ¶3. |
| `[span.sub]` | 13 | `last(size_type count)` | Hardened preconditions | declined | As ¶2. |
| `[span.sub]` | 14 | `last(size_type count)` | Effects | answered | As ¶3. |
| `[span.sub]` | 15 | `subspan(size_type offset, size_type count)` | Hardened preconditions | declined | As ¶2. |
| `[span.sub]` | 16 | `subspan(size_type offset, size_type count)` | Effects | answered | As ¶3, with the count given, left to its default and given as `dynamic_extent`. |
| `[span.obs]` | 1 | `size()` | Effects | answered |  |
| `[span.obs]` | 2 | `size_bytes()` | Effects | forced | Not declared, by decision: a packed view has no bytes of its own, its bits sharing blocks with bits it does not view, so `size() * sizeof(element_type)` would count bytes it does not hold. The bytes are those of the blocks the view was taken over, which the caller holds and can measure. |
| `[span.obs]` | 3 | `empty()` | Effects | answered |  |
| `[span.elem]` | 1 | `operator[](size_type idx)` | Hardened preconditions | declined | As `[span.cons]/23`. The cases index only within the view. |
| `[span.elem]` | 2 | `operator[](size_type idx)` | Returns | answered | `*(data() + idx)` checked as the element the iterator reaches `idx` steps from `begin()`, read alike and written through. |
| `[span.elem]` | 3 | `operator[](size_type idx)` | Throws | answered |  |
| `[span.elem]` | 4 | `at(size_type idx)` | Returns | answered | As ¶2, against `operator[]`. |
| `[span.elem]` | 5 | `at(size_type idx)` | Throws | answered |  |
| `[span.elem]` | 6 | `front()` | Hardened preconditions | declined | As ¶1. |
| `[span.elem]` | 7 | `front()` | Returns | answered | As ¶2, against `*begin()`. |
| `[span.elem]` | 8 | `front()` | Throws | answered |  |
| `[span.elem]` | 9 | `back()` | Hardened preconditions | declined | As ¶1. |
| `[span.elem]` | 10 | `back()` | Returns | answered | As ¶2, against `*(end() - 1)`. |
| `[span.elem]` | 11 | `back()` | Throws | answered |  |
| `[span.elem]` | 12 | `data()` | Returns | forced | A bit has no address, so there is no pointer to its first element; `pointer` is `void`, as for `std::vector<bool>`'s proxies. |
| `[span.iterators]` | 1 | `iterator` | — | answered | A packed view's iterator is random access rather than contiguous: a bit has no address. Checked as a constexpr iterator by walking and writing a view in a constant expression. |
| `[span.iterators]` | 2 | `iterator` | — | answered | Checked for what a container's iterator adds: default construction, equality, the view's `difference_type`, and conversion to `const_iterator`. |
| `[span.iterators]` | 3 | `begin()` | Returns | answered |  |
| `[span.iterators]` | 4 | `end()` | Returns | answered |  |
| `[span.iterators]` | 5 | `rbegin()` | Effects | answered |  |
| `[span.iterators]` | 6 | `rend()` | Effects | answered |  |
| `[span.objectrep]` | 1 | `as_bytes(span<ElementType, Extent> s)` | Constraints | forced | Not declared, by decision, as `[span.obs]/2`: a window's bits need not start or end on a byte, so there is no span of bytes that holds them and nothing else, and over a whole view it would be the caller's own blocks as bytes. |
| `[span.objectrep]` | 2 | `as_bytes(span<ElementType, Extent> s)` | Effects | forced | As ¶1. |
| `[span.objectrep]` | 3 | `as_writable_bytes(span<ElementType, Extent> s)` | Constraints | forced | As ¶1. |
| `[span.objectrep]` | 4 | `as_writable_bytes(span<ElementType, Extent> s)` | Effects | forced | As ¶1. |
