# `[bitmask.types]` audit

Every numbered paragraph of `[bitmask.types]`, and what the tests under `test/` do with it.
`tools/audit.py` checks this table against the citations in the tests.

- **Source:** [cplusplus/draft `4d8e364814c9b8ea4ce8d719ad87942ed4c63e24`](https://github.com/cplusplus/draft/blob/4d8e364814c9b8ea4ce8d719ad87942ed4c63e24/source/lib-intro.tex), `source/lib-intro.tex`, `\rSec4[bitmask.types]`.
- **Cross-checked:** [eel.is/c++draft](https://eel.is/c++draft/bitmask.types), generated from Eelis/draft `c7015b485cc3db8efaa9dfb9ff0809c5394a4ed1`: the same four paragraphs.
- **Subjects:** `test::spec::bitmask::all`, the eleven bitmask types the standard names first: `std::ios_base::fmtflags`, `iostate` and `openmode`, `std::filesystem::perms`, `perm_options`, `copy_options` and `directory_options`, `std::regex_constants::syntax_option_type` and `match_flag_type`, `std::launch` and `std::chars_format`. Then `xstd::bit_flag_set` over each of them. Each type is checked over the elements and constants its own clause names, and no library's extensions.
- **Not audited:** what a flag type adds beyond the clause, its conversion with the mask and its operators taking a mask on either side, which `test/src/spec/xstd/flag_set/` checks as ours alone.

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
| `[bitmask.types]` | 1 | bitmask type | — | answered | Checked over the standard's types alone, each an enumeration or an integer type, as the library chooses; no library uses a `bitset`. A flag type is a class, a form the paragraph does not list, and is held to ¶2-4 by its operators. |
| `[bitmask.types]` | 2 | `operator&`, `operator\|`, `operator^`, `operator~`, `operator&=`, `operator\|=`, `operator^=` | — | answered | Checked as each operator's result type and as the bitwise operation on the mask's word, over every pair of the named values and their complements. `~` complements every bit that the empty value's complement holds: libc++ masks `std::launch` to its two elements. A compound assignment returns an lvalue referring to its left operand, which libstdc++'s stream flags make `const`. The operators are not checked to be `constexpr`. |
| `[bitmask.types]` | 3 | `C0`, `C1`, … | — | answered | Checked as each element nonzero, any two distinct and disjoint, none set in the empty value, and every constant a type's clause calls empty equal to it. libc++ gives `regex_constants::ECMAScript` the value zero, so it is held as a constant rather than an element. |
| `[bitmask.types]` | 4 | set, clear, is set | — | answered | Checked as the elements set after `X \|= Y` and `X &= ~Y` against those set in `X` and `Y`, and `X & Y` nonzero for exactly the elements a value holds. `regex_constants::multiline` is left out: MSVC's STL does not declare it, and libc++'s `~` on `syntax_option_type` drops it, so clearing any value there clears it too. |
