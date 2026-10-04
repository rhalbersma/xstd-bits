# `[alg.set.operations]` audit

Every numbered paragraph of every leaf clause of `[alg.set.operations]`, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea`](https://github.com/cplusplus/draft/blob/f2aa898ea51c1cdc42ad8096bbfebdc2ab54baea/source/algorithms.tex), `source/algorithms.tex`, `\rSec3[alg.set.operations]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/alg.set.operations), generated from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same paragraph numbers in all six clauses.
- **Subjects:** the algorithms, run over the iterators of every set in `test::spec::set::all`: the standard library's `std::set<std::size_t>` and, where it has one, `std::flat_set<std::size_t>`, every xstd set, and `xstd::bit_set_view`.

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
| `[alg.set.operations.general]` | 1 | — | — | no-requirement | Introduces the set operations on sorted ranges and how they generalize to multisets; each operation's paragraphs are audited in its own clause. |
| `[includes]` | 1 | `includes(first1, last1, first2, last2)`, and the `comp`, execution-policy and `ranges` overloads | — | answered | Checked as the overloads without `comp` answering as the ones given `less{}`. |
| `[includes]` | 2 | as ¶1 | Preconditions | answered | Checked as every subject's range being sorted, which is what makes it an input at all. |
| `[includes]` | 3 | as ¶1 | Returns | answered | |
| `[includes]` | 4 | as ¶1 | Complexity | answered | Comparisons, and in the `ranges` overload applications of each projection, counted through arguments that count. Two empty ranges are held to none, the formula's -1 not being a count. |
| `[set.union]` | 1 | `set_union(first1, last1, first2, last2, result)`, and the `comp`, execution-policy and `ranges` overloads | — | answered | Checked as the overloads without `comp` answering as the ones given `less{}`. |
| `[set.union]` | 2 | as ¶1 | Preconditions | answered | As `[includes]/2`; the result is written to a buffer of its own. |
| `[set.union]` | 3 | as ¶1 | Effects | answered | Over unique keys, so the multiset counts of the second sentence reduce to the set operation. |
| `[set.union]` | 4 | as ¶1 | Returns | answered | The iterator-returning overloads; the output-range overloads of the parallel `ranges` algorithms are not in any standard library the suite builds with. |
| `[set.union]` | 5 | as ¶1 | Complexity | answered | As `[includes]/4`. |
| `[set.union]` | 6 | as ¶1 | Remarks | declined | Not checked: stability orders equivalent elements, and under a set's own `std::less<std::size_t>` or `std::greater<std::size_t>` equivalent keys are equal values, so which range an element was copied from cannot be observed. |
| `[set.intersection]` | 1 | `set_intersection(first1, last1, first2, last2, result)`, and the `comp`, execution-policy and `ranges` overloads | — | answered | Checked as the overloads without `comp` answering as the ones given `less{}`. |
| `[set.intersection]` | 2 | as ¶1 | Preconditions | answered | As `[includes]/2`; the result is written to a buffer of its own. |
| `[set.intersection]` | 3 | as ¶1 | Effects | answered | Over unique keys, so the multiset counts of the second sentence reduce to the set operation. |
| `[set.intersection]` | 4 | as ¶1 | Returns | answered | The iterator-returning overloads; the output-range overloads of the parallel `ranges` algorithms are not in any standard library the suite builds with. |
| `[set.intersection]` | 5 | as ¶1 | Complexity | answered | As `[includes]/4`. |
| `[set.intersection]` | 6 | as ¶1 | Remarks | declined | As `[set.union]/6`. |
| `[set.difference]` | 1 | `set_difference(first1, last1, first2, last2, result)`, and the `comp`, execution-policy and `ranges` overloads | — | answered | Checked as the overloads without `comp` answering as the ones given `less{}`. |
| `[set.difference]` | 2 | as ¶1 | Preconditions | answered | As `[includes]/2`; the result is written to a buffer of its own. |
| `[set.difference]` | 3 | as ¶1 | Effects | answered | Over unique keys, so the multiset counts of the second sentence reduce to the set operation. |
| `[set.difference]` | 4 | as ¶1 | Returns | answered | The iterator-returning overloads; the output-range overloads of the parallel `ranges` algorithms are not in any standard library the suite builds with. |
| `[set.difference]` | 5 | as ¶1 | Complexity | answered | As `[includes]/4`. |
| `[set.difference]` | 6 | as ¶1 | Remarks | declined | As `[set.union]/6`. |
| `[set.symmetric.difference]` | 1 | `set_symmetric_difference(first1, last1, first2, last2, result)`, and the `comp`, execution-policy and `ranges` overloads | — | answered | Checked as the overloads without `comp` answering as the ones given `less{}`. |
| `[set.symmetric.difference]` | 2 | as ¶1 | Preconditions | answered | As `[includes]/2`; the result is written to a buffer of its own. |
| `[set.symmetric.difference]` | 3 | as ¶1 | Effects | answered | Over unique keys, so the multiset counts of the second sentence reduce to the set operation. |
| `[set.symmetric.difference]` | 4 | as ¶1 | Returns | answered | The iterator-returning overloads; the output-range overloads of the parallel `ranges` algorithms are not in any standard library the suite builds with. |
| `[set.symmetric.difference]` | 5 | as ¶1 | Complexity | answered | As `[includes]/4`. |
| `[set.symmetric.difference]` | 6 | as ¶1 | Remarks | declined | As `[set.union]/6`. |
