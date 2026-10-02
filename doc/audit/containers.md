# `[container.requirements]` audit

Every numbered paragraph of the general container requirements, and of `[iterator.range]` that the containers are handed to, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/containers.tex), `source/containers.tex`, `\rSec3[container.reqmts]`, `[container.rev.reqmts]`, `[container.opt.reqmts]` and `[container.alloc.reqmts]`; and [`source/iterators.tex`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/iterators.tex), `\rSec2[iterator.range]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/container.reqmts), generated from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same paragraph numbers in all five clauses.
- **Subjects:** `test::spec::container::all`, every owner of the set reading, `test::spec::set::owners`, and every candidate of the sequence reading; the allocator-aware ones, `std::set`, `std::vector<bool>` and the dynamic and small bit containers, under three kinds of allocator.

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
| `[container.reqmts]` | 1 | — | — | no-requirement | Introduces the requirements; each expression is audited in its own paragraphs. |
| `[container.reqmts]` | 2 | `typename X::value_type` | Result | answered |  |
| `[container.reqmts]` | 3 | `typename X::value_type` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.reqmts]` | 4 | `typename X::reference` | Result | answered | Asked as written where an element has an address; a packed column hands out a proxy instead, the recorded forced departure. |
| `[container.reqmts]` | 5 | `typename X::const_reference` | Result | answered | Asked as written where an element has an address; a packed column hands out a proxy instead, the recorded forced departure. |
| `[container.reqmts]` | 6 | `typename X::iterator` | Result | answered |  |
| `[container.reqmts]` | 7 | `typename X::const_iterator` | Result | answered | Checked as a forward iterator that cannot be written through. |
| `[container.reqmts]` | 8 | `typename X::difference_type` | Result | answered |  |
| `[container.reqmts]` | 9 | `typename X::size_type` | Result | answered |  |
| `[container.reqmts]` | 10 | `X u; and 1 more` | Postconditions | answered | `[array.overview]/3` exempts an array of non-zero width, which is checked for its width instead. |
| `[container.reqmts]` | 11 | `X u; and 1 more` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 12 | `X u(v); and 1 more` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.reqmts]` | 13 | `X u(v); and 1 more` | Postconditions | answered |  |
| `[container.reqmts]` | 14 | `X u(v); and 1 more` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 15 | `X u(rv); and 1 more` | Postconditions | answered |  |
| `[container.reqmts]` | 16 | `X u(rv); and 1 more` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 17 | `t = v` | Result | answered |  |
| `[container.reqmts]` | 18 | `t = v` | Postconditions | answered |  |
| `[container.reqmts]` | 19 | `t = v` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 20 | `t = rv` | Result | answered |  |
| `[container.reqmts]` | 21 | `t = rv` | Effects | answered | Checked as the size the target ends with: none of its former elements survive the assignment. |
| `[container.reqmts]` | 22 | `t = rv` | Postconditions | answered |  |
| `[container.reqmts]` | 23 | `t = rv` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 24 | `a.~X()` | Result | answered | MSVC mis-types an explicit destructor call through a template parameter, so there only destructibility is asked. |
| `[container.reqmts]` | 25 | `a.~X()` | Effects | answered | Checked through an allocator that counts its outstanding allocations, on the allocator-aware columns; the others allocate nothing. The same allocator refuses each allocation in turn of a set's range insertion, an assignment and the allocator-extended copy, and asks that the object left behind is valid and gives back every allocation, which is the basic guarantee. |
| `[container.reqmts]` | 26 | `a.~X()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 27 | `b.begin()` | Result | answered |  |
| `[container.reqmts]` | 28 | `b.begin()` | Returns | answered |  |
| `[container.reqmts]` | 29 | `b.begin()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 30 | `b.end()` | Result | answered |  |
| `[container.reqmts]` | 31 | `b.end()` | Returns | answered |  |
| `[container.reqmts]` | 32 | `b.end()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 33 | `b.cbegin()` | Result | answered |  |
| `[container.reqmts]` | 34 | `b.cbegin()` | Returns | answered |  |
| `[container.reqmts]` | 35 | `b.cbegin()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 36 | `b.cend()` | Result | answered |  |
| `[container.reqmts]` | 37 | `b.cend()` | Returns | answered |  |
| `[container.reqmts]` | 38 | `b.cend()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 39 | `i <=> j` | Result | answered |  |
| `[container.reqmts]` | 40 | `i <=> j` | Constraints | answered | Checked both ways: the bidirectional iterators of a set have no `<=>`. |
| `[container.reqmts]` | 41 | `i <=> j` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 42 | `c == b` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.reqmts]` | 43 | `c == b` | Result | answered |  |
| `[container.reqmts]` | 44 | `c == b` | Returns | answered |  |
| `[container.reqmts]` | 45 | `c == b` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 46 | `c == b` | Remarks | answered | Checked as reflexive, symmetric and transitive over the pairs. |
| `[container.reqmts]` | 47 | `c != b` | Effects | answered |  |
| `[container.reqmts]` | 48 | `t.swap(s)` | Result | answered |  |
| `[container.reqmts]` | 49 | `t.swap(s)` | Effects | answered |  |
| `[container.reqmts]` | 50 | `t.swap(s)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 51 | `swap(t, s)` | Effects | answered |  |
| `[container.reqmts]` | 52 | `c.size()` | Result | answered |  |
| `[container.reqmts]` | 53 | `c.size()` | Returns | answered |  |
| `[container.reqmts]` | 54 | `c.size()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 55 | `c.size()` | Remarks | no-requirement | Says that constructors, insertions and erasures define the count; each checks the size it leaves. |
| `[container.reqmts]` | 56 | `c.max_size()` | Result | answered |  |
| `[container.reqmts]` | 57 | `c.max_size()` | Returns | answered |  |
| `[container.reqmts]` | 58 | `c.max_size()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 59 | `c.empty()` | Result | answered |  |
| `[container.reqmts]` | 60 | `c.empty()` | Returns | answered |  |
| `[container.reqmts]` | 61 | `c.empty()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.reqmts]` | 62 | `c.empty()` | Remarks | answered |  |
| `[container.reqmts]` | 63 | `c.empty()` | — | answered |  |
| `[container.reqmts]` | 64 | `c.empty()` | — | answered | Checked on the allocator-aware columns: the copy takes `select_on_container_copy_construction`, assignment replaces the allocator only as the propagation traits say, and every other constructor takes an allocator argument. |
| `[container.reqmts]` | 65 | `c.empty()` | — | answered | Checked as the allocators exchanged where `propagate_on_container_swap` says so. That an iterator follows its element is not asked: the small columns swap inline storage, the recorded invalidation departure. |
| `[container.reqmts]` | 66 | `c.empty()` | — | answered | Checked as `swap`, `clear`, `erase` and `pop_back` not throwing, and a single-element insertion that cannot allocate having no effects. `swap` is asserted `noexcept` of every column. |
| `[container.reqmts]` | 67 | `c.empty()` | — | answered | Checked for the observers: `size`, `max_size`, `empty` and `==` move no iterator and change no value. |
| `[container.reqmts]` | 68 | `c.empty()` | — | no-requirement | Defines a contiguous container; `[array.overview]/1` and `[inplace.vector.overview]/1` are where it is asked. |
| `[container.reqmts]` | 69 | `c.empty()` | — | answered | Checked for the iterator half: two integers construct by count. Which types qualify as allocators is unspecified beyond a minimum no constructor call can tell apart. |
| `[container.rev.reqmts]` | 1 | — | — | answered | Checked as bidirectional iterators; the rest of the paragraph introduces ¶2-15. |
| `[container.rev.reqmts]` | 2 | `typename X::reverse_iterator` | Result | answered |  |
| `[container.rev.reqmts]` | 3 | `typename X::const_reverse_iterator` | Result | answered |  |
| `[container.rev.reqmts]` | 4 | `a.rbegin()` | Result | answered |  |
| `[container.rev.reqmts]` | 5 | `a.rbegin()` | Returns | answered |  |
| `[container.rev.reqmts]` | 6 | `a.rbegin()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.rev.reqmts]` | 7 | `a.rend()` | Result | answered |  |
| `[container.rev.reqmts]` | 8 | `a.rend()` | Returns | answered |  |
| `[container.rev.reqmts]` | 9 | `a.rend()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.rev.reqmts]` | 10 | `a.crbegin()` | Result | answered |  |
| `[container.rev.reqmts]` | 11 | `a.crbegin()` | Returns | answered |  |
| `[container.rev.reqmts]` | 12 | `a.crbegin()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.rev.reqmts]` | 13 | `a.crend()` | Result | answered |  |
| `[container.rev.reqmts]` | 14 | `a.crend()` | Returns | answered |  |
| `[container.rev.reqmts]` | 15 | `a.crend()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.opt.reqmts]` | 1 | — | — | answered | The constant evaluation is asked where the storage is constant-evaluable: not `std::set`, `std::flat_set`, the small columns, the user storage, or the bounded columns before C++26. |
| `[container.opt.reqmts]` | 2 | `a <=> b` | Result | answered |  |
| `[container.opt.reqmts]` | 3 | `a <=> b` | Preconditions | no-requirement | A precondition on the element type: `bool` and `std::size_t` are totally ordered. |
| `[container.opt.reqmts]` | 4 | `a <=> b` | Returns | answered |  |
| `[container.opt.reqmts]` | 5 | `a <=> b` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 1 | — | — | answered | Checked on the allocator-aware columns: `std::vector<bool>`, `std::set` and the dynamic and small bit containers. The array and bounded columns have no allocator, as this paragraph exempts them. |
| `[container.alloc.reqmts]` | 2 | — | — | no-requirement | Defines Cpp17DefaultInsertable and its kin in terms of the allocator; each precondition stated with them is audited where it is stated. |
| `[container.alloc.reqmts]` | 3 | — | — | no-requirement | Names the variables of the expressions below and introduces them; each is audited in its own paragraphs. |
| `[container.alloc.reqmts]` | 4 | `typename X::allocator_type` | Result | answered | The small columns hand out Boost's wrapper around the allocator they were declared with. |
| `[container.alloc.reqmts]` | 5 | `typename X::allocator_type` | Mandates | declined | A packed container allocates blocks, so its `allocator_type::value_type` is the block type rather than the `bool` or key it holds. The standard library's models meet it. |
| `[container.alloc.reqmts]` | 6 | `c.get_allocator()` | Result | answered |  |
| `[container.alloc.reqmts]` | 7 | `c.get_allocator()` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 8 | `X u; and 1 more` | Preconditions | no-requirement | A precondition on the allocator type, which every allocator the cases pass meets. |
| `[container.alloc.reqmts]` | 9 | `X u; and 1 more` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 10 | `X u; and 1 more` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 11 | `X u(m);` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 12 | `X u(m);` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 13 | `X u(t, m);` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.alloc.reqmts]` | 14 | `X u(t, m);` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 15 | `X u(t, m);` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 16 | `X u(rv);` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 17 | `X u(rv);` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 18 | `X u(rv, m);` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.alloc.reqmts]` | 19 | `X u(rv, m);` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 20 | `X u(rv, m);` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 21 | `a = t` | Result | answered |  |
| `[container.alloc.reqmts]` | 22 | `a = t` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.alloc.reqmts]` | 23 | `a = t` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 24 | `a = t` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 25 | `a = rv` | Result | answered |  |
| `[container.alloc.reqmts]` | 26 | `a = rv` | Preconditions | no-requirement | A precondition on the element type, which `bool` and `std::size_t` meet; it asks nothing of the container. |
| `[container.alloc.reqmts]` | 27 | `a = rv` | Effects | answered | Checked as the size the target ends with, as `[container.reqmts]/21`. |
| `[container.alloc.reqmts]` | 28 | `a = rv` | Postconditions | answered |  |
| `[container.alloc.reqmts]` | 29 | `a = rv` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[container.alloc.reqmts]` | 30 | `a.swap(b)` | Result | answered |  |
| `[container.alloc.reqmts]` | 31 | `a.swap(b)` | Effects | answered |  |
| `[container.alloc.reqmts]` | 32 | `a.swap(b)` | Complexity | declined | A complexity bound, which no functional check can observe. |
| `[iterator.range]` | 1 | `<iterator>` | — | no-requirement | Names the standard headers that also declare these templates: where a name is available, not what any subject does. |
| `[iterator.range]` | 2 | `begin(C& c)`, `begin(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 3 | `end(C& c)`, `end(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 4 | `begin(T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 5 | `end(T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 6 | `cbegin(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 7 | `cend(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 8 | `rbegin(C& c)`, `rbegin(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 9 | `rend(C& c)`, `rend(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 10 | `rbegin(T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 11 | `rend(T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 12 | `rbegin(initializer_list<E> il)` | Returns | no-requirement | The overload for an `initializer_list`, which no subject of this reading is. |
| `[iterator.range]` | 13 | `rend(initializer_list<E> il)` | Returns | no-requirement | The overload for an `initializer_list`, which no subject of this reading is. |
| `[iterator.range]` | 14 | `crbegin(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 15 | `crend(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 16 | `size(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 17 | `size(const T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 18 | `ssize(const C& c)` | Effects | answered |  |
| `[iterator.range]` | 19 | `ssize(const T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 20 | `empty(const C& c)` | Returns | answered |  |
| `[iterator.range]` | 21 | `empty(const T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
| `[iterator.range]` | 22 | `data(C& c)`, `data(const C& c)` | Returns | forced | A packed column has no `data()`: a bit has no address, the recorded forced departure. `std::array<bool, N>` and `std::inplace_vector<bool, N>` have one and are asserted to, uncited. |
| `[iterator.range]` | 23 | `data(T (&array)[N])` | Returns | no-requirement | The overload for a built-in array, which no subject of this reading is. |
