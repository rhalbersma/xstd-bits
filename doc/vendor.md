# Standard library test suites

Each standard library's own tests of the containers this library generalizes, and what became of each one when it ran
against each of them. A section is one suite against one subject. `tools/vendor.py` writes it from the manifest that
rhalbersma/cpp-ci's adapter leaves in the build, and each suite job's `std-test-suite.<suite>-<subject>.table` test fails
when the section no longer matches it.

- **Mappings:** `test/std-test-suite/<suite>-<directory>.toml` name the directories a suite takes and each departure
  from their tests, and `<subject>.toml` the rewrites onto one of this library's types.
- **Hashes:** the first twelve hexadecimal digits of the test file's SHA-256 at the pinned tag.
- **Taken from:** each suite's own tests of `vector<bool>` and `set`, and its tests elsewhere that use `vector<bool>`: the
  algorithms, and libc++'s utilities or libstdc++'s iterators, range adaptors and hash.

The other containers this library generalizes have nothing at the pins for a subject to take. Neither suite tests
`inplace_vector`. Their `array` tests are over `int` and the like, and none over `bool`. Their `span` tests have no
`span<bool>` but libstdc++'s `span<const bool>` over a `bool` array, which no bit span views. Their `valarray` tests use
`valarray<bool>` only as the mask a `valarray<int>` is subscripted by or the result its comparisons answer. A pin that
brings such a test is reviewed when it moves.

A row's `outcome` is one of:

| outcome | meaning |
| :--- | :--- |
| `runs` | built and run as written, after the rewrite |
| `compiles` | a test of declarations alone, built as written |
| `runs, adapted` | built and run with the lines the note lists changed or dropped, each for the departure it names |
| `compiles, adapted` | as `compiles`, with lines changed as for `runs, adapted` |
| `builds` | built and linked as written, never run |
| `verified` | a test of diagnostics, compiled under Clang's `-verify`, which reports every diagnostic its markers expect |
| `rejected` | a test of diagnostics, which fails to compile with every error its `dg-error` markers name |
| `fails to compile` | expected not to compile, for the paragraph or reason the note gives |
| `fails at run time` | expected to fail when run, for the paragraph or reason the note gives |
| `not run` | left out, for the reason the note gives |

## libc++ against bit_vector

- **Pinned:** [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) at `llvmorg-22.1.8`.
- **Mappings:** the suite's `test/std-test-suite/libcxx-*.toml` for the directories it takes, and `bit-vector.toml` for the type.
- **Not selected:** 1734 files that do not match `vector<bool>`, left to the standard library.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/copy.pass.cpp` | `614068644d03` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/copy_backward.pass.cpp` | `f4f932c22877` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/copy_n.pass.cpp` | `c32b2236b2e7` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/ranges.copy.pass.cpp` | `a147c2b6bbe8` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/ranges.copy_backward.pass.cpp` | `b69b066b69cf` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/ranges.copy_n.pass.cpp` | `28009871e32d` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.fill/fill.pass.cpp` | `39a9758aa361` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.fill/fill_n.pass.cpp` | `9abaa5be2713` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.fill/ranges.fill.pass.cpp` | `f7e87bdffc70` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.fill/ranges.fill_n.pass.cpp` | `dd7f339c7f61` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.move/move.pass.cpp` | `5d534cbcc2cb` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.move/move_backward.pass.cpp` | `f83bbfacaaa1` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.move/ranges.move.pass.cpp` | `c6dde1649b8c` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.move/ranges.move_backward.pass.cpp` | `30d121905638` | runs |  |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.rotate/ranges_rotate.pass.cpp` | `c14ff24d631e` | runs, adapted | static_assert(test()); (libc++'s word-wise algorithms over its own __bit_iterator) |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.rotate/rotate.pass.cpp` | `dc5893911fb3` | runs, adapted | static_assert(test()); (libc++'s word-wise algorithms over its own __bit_iterator) |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.swap/ranges.swap_ranges.pass.cpp` | `e3e399398efa` | runs, adapted | static_assert(test()); (libc++'s word-wise algorithms over its own __bit_iterator) |
| `libcxx/test/std/algorithms/alg.modifying.operations/alg.swap/swap_ranges.pass.cpp` | `31d6f02713da` | runs |  |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.contains/ranges.contains.pass.cpp` | `ce93ac32c52e` | runs |  |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.count/count.pass.cpp` | `1ac8db305ae4` | runs, adapted | static_assert(test()); (libc++'s word-wise algorithms over its own __bit_iterator) |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.count/ranges.count.pass.cpp` | `417e99545def` | runs, adapted | static_assert(test()); (libc++'s word-wise algorithms over its own __bit_iterator) |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.equal/equal.pass.cpp` | `f3fac75c6ada` | runs |  |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.equal/ranges.equal.pass.cpp` | `a147e7c0ae67` | runs |  |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.find/find.pass.cpp` | `d5564da7c254` | runs |  |
| `libcxx/test/std/algorithms/alg.nonmodifying/alg.find/ranges.find.pass.cpp` | `7e7310cd53a3` | runs |  |
| `libcxx/test/std/algorithms/alg.sorting/alg.three.way/lexicographical_compare_three_way.pass.cpp` | `852950fb21d6` | runs |  |
| `libcxx/test/std/algorithms/alg.sorting/alg.three.way/lexicographical_compare_three_way_comp.pass.cpp` | `5538a26c6bbe` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/append_range.pass.cpp` | `a7bb6ff8b52a` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/assign_copy.pass.cpp` | `786caa6714a6` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/assign_initializer_list.pass.cpp` | `8ac695a13daf` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/assign_iter_iter.pass.cpp` | `bf33e7e26c13` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/assign_move.pass.cpp` | `a0d3785c2f48` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/assign_range.pass.cpp` | `2597e4d51293` | runs, adapted | assert(v == (std::vector{true, true, false})); (no == between a bit vector and std::vector<bool>) |
| `libcxx/test/std/containers/sequences/vector.bool/assign_size_value.pass.cpp` | `c1c029389fc2` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/at.pass.cpp` | `c16bb2a0fbe3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/at_const.pass.cpp` | `1ad4a6be52c4` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/capacity.pass.cpp` | `bd0cdf5c5148` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/compare.pass.cpp` | `611e6f3d6591` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/compare.three_way.pass.cpp` | `84536e3f356e` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/const_reference.pass.cpp` | `b2c54fc371d8` | runs, adapted | ASSERT_SAME_TYPE(CRefT, std::__bit_const_reference<std::vector<bool> >); (const_reference is the bit vector's own proxy) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_default.pass.cpp` | `91e7235d968d` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()); LIBCPP_ASSERT(c1.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_from_range.pass.cpp` | `ae75de83eac2` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/construct_iter_iter.pass.cpp` | `c858cf3457f9` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_iter_iter_alloc.pass.cpp` | `20896f7a3a51` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_size.pass.cpp` | `cadd143ad1be` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_size_value.pass.cpp` | `a95b41f6af6a` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_size_value_alloc.pass.cpp` | `239cc590980b` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/copy.pass.cpp` | `099b47ccca0a` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/copy_alloc.pass.cpp` | `85df1926257b` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()) |
| `libcxx/test/std/containers/sequences/vector.bool/ctor_exceptions.pass.cpp` | `f0a3662fb27d` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/default_noexcept.pass.cpp` | `9322089620ca` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/dtor_noexcept.pass.cpp` | `6b7ff6d79584` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/emplace.pass.cpp` | `a95b4892a7f1` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/emplace_back.pass.cpp` | `f5c231ec0d5b` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/empty.pass.cpp` | `c02fbea895f1` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/empty.verify.cpp` | `1782f8452118` | verified |  |
| `libcxx/test/std/containers/sequences/vector.bool/enabled_hash.pass.cpp` | `f3922952e6c6` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/erase_iter.pass.cpp` | `1626d6e39a3b` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/erase_iter_iter.pass.cpp` | `4308b6117e67` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/find.pass.cpp` | `ce82dff5fad1` | runs, adapted | static_assert(tests()); (libc++'s word-wise find over its own __bit_iterator) |
| `libcxx/test/std/containers/sequences/vector.bool/flip.pass.cpp` | `9d448c597d57` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/get_allocator.pass.cpp` | `a2c1f841bc86` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/initializer_list.pass.cpp` | `3925ac3909fe` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/initializer_list_alloc.pass.cpp` | `a1a77b5186d1` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_initializer_list.pass.cpp` | `8eac48e5232b` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_iter_iter.pass.cpp` | `fa5e3a732705` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_size_value.pass.cpp` | `c1141d3da82b` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_value.pass.cpp` | `9c67396f68b1` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/insert_range.pass.cpp` | `74eee090d500` | runs, adapted | assert(v == (std::vector{true, true, false})); (no == between a bit vector and std::vector<bool>) |
| `libcxx/test/std/containers/sequences/vector.bool/iterator_concept_conformance.compile.pass.cpp` | `877d7324a1f2` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/iterators.pass.cpp` | `4b0f26352d9b` | runs, adapted | ASSERT_SAME_TYPE(ConstIterRefT, std::__bit_const_reference<std::vector<bool> >); (const_reference is the bit vector's own proxy) |
| `libcxx/test/std/containers/sequences/vector.bool/max_size.pass.cpp` | `54d60eacfd4e` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/move.pass.cpp` | `2b5de716ce17` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/move_alloc.pass.cpp` | `c39401b2f483` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/move_assign_noexcept.pass.cpp` | `ff0e0c366b0a` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/move_noexcept.pass.cpp` | `a5e20790f775` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/op_equal_initializer_list.pass.cpp` | `13a3e3eb7f46` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/push_back.pass.cpp` | `11b111d47b14` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/range_concept_conformance.compile.pass.cpp` | `64bbf9b43b1b` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference.swap.pass.cpp` | `61152ce772dc` | runs, adapted | static_assert((noexcept(v.swap(r1, r2))), ""); ([vector.bool.pspc]/11 swaps two references through a hidden friend of reference (P3612R1); the test calls the static member it replaced); v.swap(r1, r2); ([vector.bool.pspc]/11 swaps two references through a hidden friend of reference (P3612R1); the test calls the static member it replaced) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/assign_bool.pass.cpp` | `1345f2bbf123` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference/assign_copy.pass.cpp` | `2147006250fc` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference/ctor_copy.pass.cpp` | `3b5e218787b7` | runs, adapted | ref.flip(); ([vector.bool.pspc]/10: no reference::flip(); an element is negated by assignment) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/ctor_no_default.compile.pass.cpp` | `6046838b48bc` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference/flip.pass.cpp` | `14c46e44ed31` | fails to compile | [vector.bool.pspc]/10: no reference::flip(); an element is negated by assignment |
| `libcxx/test/std/containers/sequences/vector.bool/reference/operator_bool.pass.cpp` | `4ef15a3c29e8` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference/triviality.compile.pass.cpp` | `84c3f15dab61` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/reserve.pass.cpp` | `a8eceb876f60` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/resize_size.pass.cpp` | `de77f47f77c1` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/resize_size_value.pass.cpp` | `64ec1aac4726` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/shrink_to_fit.pass.cpp` | `0728d15aaa56` | runs, adapted | unsigned bits_per_word = static_cast<unsigned>(sizeof(C::__storage_type) * CHAR_BIT); (libc++'s internal __storage_type, which is the bit vector's block_container_type::value_type) |
| `libcxx/test/std/containers/sequences/vector.bool/size.pass.cpp` | `47bd59fb0f7f` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/small_allocator_size.pass.cpp` | `8d8e42571436` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/swap.pass.cpp` | `314da3d047e1` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/swap_noexcept.pass.cpp` | `5a653ac42f5d` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/types.pass.cpp` | `174730a46512` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.functions.format.pass.cpp` | `17dea9b8044b` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.functions.tests.h` | `c4f78270d64f` | not run | not a test source |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.functions.vformat.pass.cpp` | `2cc492786004` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.pass.cpp` | `816e848537be` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/parse.pass.cpp` | `1e8daf4e26c8` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/types.compile.pass.cpp` | `22eeadf5632f` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/vector_bool.pass.cpp` | `7dd9a484a116` | runs |  |
| `libcxx/test/std/utilities/format/format.formattable/concept.formattable.compile.pass.cpp` | `77256003513a` | compiles |  |
| `libcxx/test/std/utilities/format/format.formatter/format.formatter.locking/enable_nonlocking_formatter_optimization.compile.pass.cpp` | `4f31cc88c669` | compiles |  |
| `libcxx/test/std/utilities/variant/variant.variant/variant.assign/T.pass.cpp` | `5fb4429ca96b` | runs |  |
| `libcxx/test/std/utilities/variant/variant.variant/variant.ctor/T.pass.cpp` | `28c308a17d14` | runs |  |

## libc++ against bit_small_vector

- **Pinned:** [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) at `llvmorg-22.1.8`.
- **Mappings:** the suite's `test/std-test-suite/libcxx-*.toml` for the directories it takes, and `bit-small-vector.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libcxx/test/std/containers/sequences/vector.bool/append_range.pass.cpp` | `a7bb6ff8b52a` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/assign_copy.pass.cpp` | `786caa6714a6` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/assign_initializer_list.pass.cpp` | `8ac695a13daf` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/assign_iter_iter.pass.cpp` | `bf33e7e26c13` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/assign_move.pass.cpp` | `a0d3785c2f48` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/assign_range.pass.cpp` | `2597e4d51293` | runs, adapted | assert(v == (std::vector{true, true, false})); (no == between a bit vector and std::vector<bool>); static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/assign_size_value.pass.cpp` | `c1c029389fc2` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/at.pass.cpp` | `c16bb2a0fbe3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/at_const.pass.cpp` | `1ad4a6be52c4` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/capacity.pass.cpp` | `bd0cdf5c5148` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression); assert(v.capacity() == 0); (an empty small vector holds its first 64 bits inside it, so its capacity starts at 64) |
| `libcxx/test/std/containers/sequences/vector.bool/compare.pass.cpp` | `611e6f3d6591` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/compare.three_way.pass.cpp` | `84536e3f356e` | runs, adapted | static_assert(test_sequence_container_spaceship_vectorbool()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/const_reference.pass.cpp` | `b2c54fc371d8` | runs, adapted | ASSERT_SAME_TYPE(CRefT, std::__bit_const_reference<std::vector<bool> >); (const_reference is the bit vector's own proxy); static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_default.pass.cpp` | `91e7235d968d` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()); LIBCPP_ASSERT(c1.__invariants()); (libc++'s internal __invariants()); static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_from_range.pass.cpp` | `ae75de83eac2` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_iter_iter.pass.cpp` | `c858cf3457f9` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()); static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_iter_iter_alloc.pass.cpp` | `20896f7a3a51` | fails to compile | [container.alloc.reqmts]: the allocator is the storage's, Boost's small_vector allocator of blocks, and takes none of the test's allocators of bool |
| `libcxx/test/std/containers/sequences/vector.bool/construct_size.pass.cpp` | `cadd143ad1be` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()); static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_size_value.pass.cpp` | `a95b41f6af6a` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()); static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/construct_size_value_alloc.pass.cpp` | `239cc590980b` | fails to compile | [container.alloc.reqmts]: the allocator is the storage's, Boost's small_vector allocator of blocks, and takes none of the test's allocators of bool |
| `libcxx/test/std/containers/sequences/vector.bool/copy.pass.cpp` | `099b47ccca0a` | runs, adapted | LIBCPP_ASSERT(c.__invariants()); (libc++'s internal __invariants()); static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/copy_alloc.pass.cpp` | `85df1926257b` | fails to compile | [container.alloc.reqmts]: the allocator is the storage's, Boost's small_vector allocator of blocks, and takes none of the test's allocators of bool |
| `libcxx/test/std/containers/sequences/vector.bool/ctor_exceptions.pass.cpp` | `f0a3662fb27d` | fails to compile | [container.alloc.reqmts]: the allocator is the storage's, Boost's small_vector allocator of blocks, and takes none of the test's allocators of bool |
| `libcxx/test/std/containers/sequences/vector.bool/default_noexcept.pass.cpp` | `9322089620ca` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/dtor_noexcept.pass.cpp` | `6b7ff6d79584` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/emplace.pass.cpp` | `a95b4892a7f1` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/emplace_back.pass.cpp` | `f5c231ec0d5b` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/empty.pass.cpp` | `c02fbea895f1` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/empty.verify.cpp` | `1782f8452118` | verified |  |
| `libcxx/test/std/containers/sequences/vector.bool/enabled_hash.pass.cpp` | `f3922952e6c6` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/erase_iter.pass.cpp` | `1626d6e39a3b` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/erase_iter_iter.pass.cpp` | `4308b6117e67` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/find.pass.cpp` | `ce82dff5fad1` | runs, adapted | static_assert(tests()); (libc++'s word-wise find over its own __bit_iterator) |
| `libcxx/test/std/containers/sequences/vector.bool/flip.pass.cpp` | `9d448c597d57` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/get_allocator.pass.cpp` | `a2c1f841bc86` | fails to compile | [container.alloc.reqmts]: the allocator is the storage's, Boost's small_vector allocator of blocks, and takes none of the test's allocators of bool |
| `libcxx/test/std/containers/sequences/vector.bool/initializer_list.pass.cpp` | `3925ac3909fe` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/initializer_list_alloc.pass.cpp` | `a1a77b5186d1` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_initializer_list.pass.cpp` | `8eac48e5232b` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_iter_iter.pass.cpp` | `fa5e3a732705` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_size_value.pass.cpp` | `c1141d3da82b` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/insert_iter_value.pass.cpp` | `9c67396f68b1` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/insert_range.pass.cpp` | `74eee090d500` | runs, adapted | assert(v == (std::vector{true, true, false})); (no == between a bit vector and std::vector<bool>); static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/iterator_concept_conformance.compile.pass.cpp` | `877d7324a1f2` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/iterators.pass.cpp` | `4b0f26352d9b` | runs, adapted | ASSERT_SAME_TYPE(ConstIterRefT, std::__bit_const_reference<std::vector<bool> >); (const_reference is the bit vector's own proxy) |
| `libcxx/test/std/containers/sequences/vector.bool/max_size.pass.cpp` | `54d60eacfd4e` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/move.pass.cpp` | `2b5de716ce17` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/move_alloc.pass.cpp` | `c39401b2f483` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/move_assign_noexcept.pass.cpp` | `ff0e0c366b0a` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/move_noexcept.pass.cpp` | `a5e20790f775` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/op_equal_initializer_list.pass.cpp` | `13a3e3eb7f46` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/push_back.pass.cpp` | `11b111d47b14` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/range_concept_conformance.compile.pass.cpp` | `64bbf9b43b1b` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference.swap.pass.cpp` | `61152ce772dc` | runs, adapted | static_assert((noexcept(v.swap(r1, r2))), ""); ([vector.bool.pspc]/11 swaps two references through a hidden friend of reference (P3612R1); the test calls the static member it replaced); v.swap(r1, r2); ([vector.bool.pspc]/11 swaps two references through a hidden friend of reference (P3612R1); the test calls the static member it replaced); static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/assign_bool.pass.cpp` | `1345f2bbf123` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/assign_copy.pass.cpp` | `2147006250fc` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/ctor_copy.pass.cpp` | `3b5e218787b7` | runs, adapted | ref.flip(); ([vector.bool.pspc]/10: no reference::flip(); an element is negated by assignment); static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/ctor_no_default.compile.pass.cpp` | `6046838b48bc` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/reference/flip.pass.cpp` | `14c46e44ed31` | fails to compile | [vector.bool.pspc]/10: no reference::flip(); an element is negated by assignment |
| `libcxx/test/std/containers/sequences/vector.bool/reference/operator_bool.pass.cpp` | `4ef15a3c29e8` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/reference/triviality.compile.pass.cpp` | `84c3f15dab61` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/reserve.pass.cpp` | `a8eceb876f60` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/resize_size.pass.cpp` | `de77f47f77c1` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/resize_size_value.pass.cpp` | `64ec1aac4726` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/shrink_to_fit.pass.cpp` | `0728d15aaa56` | runs, adapted | unsigned bits_per_word = static_cast<unsigned>(sizeof(C::__storage_type) * CHAR_BIT); (libc++'s internal __storage_type, which is the bit vector's block_container_type::value_type); static_assert(tests()); (Boost's small_vector is not usable in a constant expression); LIBCPP_ASSERT(v.capacity() == 0); // libc++ honors the shrink_to_fit request as a QOI matter (an empty small vector holds its first 64 bits inside it, so shrink_to_fit leaves it at 64) |
| `libcxx/test/std/containers/sequences/vector.bool/size.pass.cpp` | `47bd59fb0f7f` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/small_allocator_size.pass.cpp` | `8d8e42571436` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/swap.pass.cpp` | `314da3d047e1` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |
| `libcxx/test/std/containers/sequences/vector.bool/swap_noexcept.pass.cpp` | `5a653ac42f5d` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/types.pass.cpp` | `174730a46512` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.functions.format.pass.cpp` | `17dea9b8044b` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.functions.tests.h` | `c4f78270d64f` | not run | not a test source |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.functions.vformat.pass.cpp` | `2cc492786004` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/format.pass.cpp` | `816e848537be` | runs |  |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/parse.pass.cpp` | `1e8daf4e26c8` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/sequences/vector.bool/vector.bool.fmt/types.compile.pass.cpp` | `22eeadf5632f` | compiles |  |
| `libcxx/test/std/containers/sequences/vector.bool/vector_bool.pass.cpp` | `7dd9a484a116` | runs, adapted | static_assert(tests()); (Boost's small_vector is not usable in a constant expression) |

## libc++ against bit_set

- **Pinned:** [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) at `llvmorg-22.1.8`.
- **Mappings:** the suite's `test/std-test-suite/libcxx-*.toml` for the directories it takes, and `bit-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libcxx/test/std/containers/associative/set/allocator_mismatch.verify.cpp` | `652e404d93f1` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/clear.pass.cpp` | `868ab0b8ffa2` | runs |  |
| `libcxx/test/std/containers/associative/set/contains.pass.cpp` | `65f44690079c` | runs |  |
| `libcxx/test/std/containers/associative/set/contains_transparent.pass.cpp` | `53e63173cb89` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/count.pass.cpp` | `5584b1aee741` | runs |  |
| `libcxx/test/std/containers/associative/set/count_transparent.pass.cpp` | `cc6c8ec6805e` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/emplace.pass.cpp` | `a97e7837b0a8` | runs |  |
| `libcxx/test/std/containers/associative/set/emplace_hint.pass.cpp` | `c34b803be01e` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.pass.cpp` | `867fff1dd206` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.verify.cpp` | `41e7e25fd205` | verified |  |
| `libcxx/test/std/containers/associative/set/equal_range.pass.cpp` | `4583b139b1cd` | runs |  |
| `libcxx/test/std/containers/associative/set/equal_range_transparent.pass.cpp` | `da7036c6599d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/erase_iter.pass.cpp` | `909c375d1481` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_iter_iter.pass.cpp` | `62ef8c22ae43` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_key.pass.cpp` | `60869d49ba8b` | runs |  |
| `libcxx/test/std/containers/associative/set/extract_iterator.pass.cpp` | `9128487e04ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/extract_key.pass.cpp` | `772ad5bc7540` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/find.pass.cpp` | `ade8c52b1955` | runs |  |
| `libcxx/test/std/containers/associative/set/gcc_workaround.compile.pass.cpp` | `634150905da9` | compiles |  |
| `libcxx/test/std/containers/associative/set/get_allocator.pass.cpp` | `fc4089ed8899` | runs |  |
| `libcxx/test/std/containers/associative/set/incomplete_type.pass.cpp` | `b97dd11b85dc` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_and_emplace_allocator_requirements.pass.cpp` | `c30872880534` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_cv.pass.cpp` | `e97308343c7a` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_initializer_list.pass.cpp` | `0bc5f2a9ce75` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_cv.pass.cpp` | `f66a4fa7d485` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_iter.pass.cpp` | `a3718a9c57f0` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_iter_rv.pass.cpp` | `27478c59caea` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_node_type.pass.cpp` | `b35cd8ad9585` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_node_type_hint.pass.cpp` | `bfa4b3367805` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_range.pass.cpp` | `5ae2955e9078` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_rv.pass.cpp` | `4304561cb95c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/iterator.pass.cpp` | `819b7372dfe6` | runs |  |
| `libcxx/test/std/containers/associative/set/iterator_concept_conformance.compile.pass.cpp` | `a2025a27b5c2` | compiles |  |
| `libcxx/test/std/containers/associative/set/lower_bound.pass.cpp` | `4f7a2603a0f6` | runs |  |
| `libcxx/test/std/containers/associative/set/max_size.pass.cpp` | `ef6ff179df58` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/merge.pass.cpp` | `658c775174c6` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libcxx/test/std/containers/associative/set/range_concept_conformance.compile.pass.cpp` | `44fbd721985e` | compiles |  |
| `libcxx/test/std/containers/associative/set/set.cons/alloc.pass.cpp` | `aa43e9fcb26c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/assign_initializer_list.pass.cpp` | `f964d9f95928` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/compare.pass.cpp` | `0d8f91617a0f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_alloc.pass.cpp` | `6a6cfa91e0e2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_copy_constructible.compile.fail.cpp` | `ef04ba8973f2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy.pass.cpp` | `fb263a52b8d7` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_alloc.pass.cpp` | `08bc9f7177b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.addressof.compile.pass.cpp` | `42e4ed735d3c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.pass.cpp` | `980c27da3575` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.pass.cpp` | `67892454971e` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.verify.cpp` | `1bf1b172533f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/default.pass.cpp` | `e9b906681a03` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/default_noexcept.pass.cpp` | `c830d62c2869` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/dtor_noexcept.pass.cpp` | `3c74373bb576` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/from_range.pass.cpp` | `5c08a8ebab24` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list.pass.cpp` | `6c6ac53143bf` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare.pass.cpp` | `a69c0795ff30` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare_alloc.pass.cpp` | `c40cd541e87c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter.pass.cpp` | `bd55ceb68288` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_alloc.pass.cpp` | `703e27ee0053` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_comp.pass.cpp` | `f127416c851d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move.pass.cpp` | `d7d0d1fb2af5` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_alloc.pass.cpp` | `63dc391910eb` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign.pass.cpp` | `2e0f50070d13` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign_noexcept.compile.pass.cpp` | `3a2b32c868d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_noexcept.pass.cpp` | `5e61f09993b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.erasure/erase_if.pass.cpp` | `bf06247af20e` | runs, adapted | ASSERT_SAME_TYPE(typename S::size_type, decltype(std::erase_if(s, p))); (erase_if is xstd's, found by argument-dependent lookup); assert(expected_erased_count == std::erase_if(s, p)); (erase_if is xstd's, found by argument-dependent lookup) |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.pass.cpp` | `83d05f3bef66` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.verify.cpp` | `5a78a712cc54` | verified |  |
| `libcxx/test/std/containers/associative/set/set.nonmember/op_compare.pass.cpp` | `db0f2ccd8606` | runs |  |
| `libcxx/test/std/containers/associative/set/set.observers/comp.pass.cpp` | `7a51eb983905` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/member_swap.pass.cpp` | `e6db2e8ded52` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/non_member_swap.pass.cpp` | `e83b62b3de55` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/swap_noexcept.pass.cpp` | `9d0330c20bd4` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/size.pass.cpp` | `fb4ba712d3f1` | runs |  |
| `libcxx/test/std/containers/associative/set/types.pass.cpp` | `4b2d43c4a72a` | runs, adapted | static_assert((std::is_same<C::key_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::key_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::allocator_type, std::allocator<int> >::value), ""); (allocator_type is the block container's allocator); static_assert((std::is_same<C::reference, int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::const_reference, const int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::pointer, int*>::value), ""); (pointer is the set's own proxy); static_assert((std::is_same<C::const_pointer, const int*>::value), ""); (pointer is the set's own proxy) |
| `libcxx/test/std/containers/associative/set/upper_bound.pass.cpp` | `c8e7efe86b1d` | runs |  |

## libc++ against bit_fixed_set

- **Pinned:** [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) at `llvmorg-22.1.8`.
- **Mappings:** the suite's `test/std-test-suite/libcxx-*.toml` for the directories it takes, and `bit-fixed-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libcxx/test/std/containers/associative/set/allocator_mismatch.verify.cpp` | `652e404d93f1` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/clear.pass.cpp` | `868ab0b8ffa2` | runs |  |
| `libcxx/test/std/containers/associative/set/contains.pass.cpp` | `65f44690079c` | runs |  |
| `libcxx/test/std/containers/associative/set/contains_transparent.pass.cpp` | `53e63173cb89` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/count.pass.cpp` | `5584b1aee741` | runs |  |
| `libcxx/test/std/containers/associative/set/count_transparent.pass.cpp` | `cc6c8ec6805e` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/emplace.pass.cpp` | `a97e7837b0a8` | runs |  |
| `libcxx/test/std/containers/associative/set/emplace_hint.pass.cpp` | `c34b803be01e` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.pass.cpp` | `867fff1dd206` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.verify.cpp` | `41e7e25fd205` | verified |  |
| `libcxx/test/std/containers/associative/set/equal_range.pass.cpp` | `4583b139b1cd` | runs |  |
| `libcxx/test/std/containers/associative/set/equal_range_transparent.pass.cpp` | `da7036c6599d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/erase_iter.pass.cpp` | `909c375d1481` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_iter_iter.pass.cpp` | `62ef8c22ae43` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_key.pass.cpp` | `60869d49ba8b` | runs |  |
| `libcxx/test/std/containers/associative/set/extract_iterator.pass.cpp` | `9128487e04ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/extract_key.pass.cpp` | `772ad5bc7540` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/find.pass.cpp` | `ade8c52b1955` | runs |  |
| `libcxx/test/std/containers/associative/set/gcc_workaround.compile.pass.cpp` | `634150905da9` | compiles |  |
| `libcxx/test/std/containers/associative/set/get_allocator.pass.cpp` | `fc4089ed8899` | fails to compile | [container.alloc.reqmts]: a fixed set allocates nothing, so it has no allocator to return |
| `libcxx/test/std/containers/associative/set/incomplete_type.pass.cpp` | `b97dd11b85dc` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_and_emplace_allocator_requirements.pass.cpp` | `c30872880534` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_cv.pass.cpp` | `e97308343c7a` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_initializer_list.pass.cpp` | `0bc5f2a9ce75` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_cv.pass.cpp` | `f66a4fa7d485` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_iter.pass.cpp` | `a3718a9c57f0` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_iter_rv.pass.cpp` | `27478c59caea` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_node_type.pass.cpp` | `b35cd8ad9585` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_node_type_hint.pass.cpp` | `bfa4b3367805` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_range.pass.cpp` | `5ae2955e9078` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_rv.pass.cpp` | `4304561cb95c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/iterator.pass.cpp` | `819b7372dfe6` | runs |  |
| `libcxx/test/std/containers/associative/set/iterator_concept_conformance.compile.pass.cpp` | `a2025a27b5c2` | compiles |  |
| `libcxx/test/std/containers/associative/set/lower_bound.pass.cpp` | `4f7a2603a0f6` | runs |  |
| `libcxx/test/std/containers/associative/set/max_size.pass.cpp` | `ef6ff179df58` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/merge.pass.cpp` | `658c775174c6` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libcxx/test/std/containers/associative/set/range_concept_conformance.compile.pass.cpp` | `44fbd721985e` | compiles |  |
| `libcxx/test/std/containers/associative/set/set.cons/alloc.pass.cpp` | `aa43e9fcb26c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/assign_initializer_list.pass.cpp` | `f964d9f95928` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/compare.pass.cpp` | `0d8f91617a0f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_alloc.pass.cpp` | `6a6cfa91e0e2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_copy_constructible.compile.fail.cpp` | `ef04ba8973f2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy.pass.cpp` | `fb263a52b8d7` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_alloc.pass.cpp` | `08bc9f7177b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.addressof.compile.pass.cpp` | `42e4ed735d3c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.pass.cpp` | `980c27da3575` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.pass.cpp` | `67892454971e` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.verify.cpp` | `1bf1b172533f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/default.pass.cpp` | `e9b906681a03` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/default_noexcept.pass.cpp` | `c830d62c2869` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/dtor_noexcept.pass.cpp` | `3c74373bb576` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/from_range.pass.cpp` | `5c08a8ebab24` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list.pass.cpp` | `6c6ac53143bf` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare.pass.cpp` | `a69c0795ff30` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare_alloc.pass.cpp` | `c40cd541e87c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter.pass.cpp` | `bd55ceb68288` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_alloc.pass.cpp` | `703e27ee0053` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_comp.pass.cpp` | `f127416c851d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move.pass.cpp` | `d7d0d1fb2af5` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_alloc.pass.cpp` | `63dc391910eb` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign.pass.cpp` | `2e0f50070d13` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign_noexcept.compile.pass.cpp` | `3a2b32c868d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_noexcept.pass.cpp` | `5e61f09993b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.erasure/erase_if.pass.cpp` | `bf06247af20e` | runs, adapted | ASSERT_SAME_TYPE(typename S::size_type, decltype(std::erase_if(s, p))); (erase_if is xstd's, found by argument-dependent lookup); assert(expected_erased_count == std::erase_if(s, p)); (erase_if is xstd's, found by argument-dependent lookup) |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.pass.cpp` | `83d05f3bef66` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.verify.cpp` | `5a78a712cc54` | verified |  |
| `libcxx/test/std/containers/associative/set/set.nonmember/op_compare.pass.cpp` | `db0f2ccd8606` | runs |  |
| `libcxx/test/std/containers/associative/set/set.observers/comp.pass.cpp` | `7a51eb983905` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/member_swap.pass.cpp` | `e6db2e8ded52` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/non_member_swap.pass.cpp` | `e83b62b3de55` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/swap_noexcept.pass.cpp` | `9d0330c20bd4` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/size.pass.cpp` | `fb4ba712d3f1` | runs |  |
| `libcxx/test/std/containers/associative/set/types.pass.cpp` | `4b2d43c4a72a` | runs, adapted | static_assert((std::is_same<C::key_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::key_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::allocator_type, std::allocator<int> >::value), ""); (allocator_type is the block container's allocator); static_assert((std::is_same<C::reference, int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::const_reference, const int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::pointer, int*>::value), ""); (pointer is the set's own proxy); static_assert((std::is_same<C::const_pointer, const int*>::value), ""); (pointer is the set's own proxy) |
| `libcxx/test/std/containers/associative/set/upper_bound.pass.cpp` | `c8e7efe86b1d` | runs |  |

## libc++ against bit_inplace_set

- **Pinned:** [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) at `llvmorg-22.1.8`.
- **Mappings:** the suite's `test/std-test-suite/libcxx-*.toml` for the directories it takes, and `bit-inplace-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libcxx/test/std/containers/associative/set/allocator_mismatch.verify.cpp` | `652e404d93f1` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/clear.pass.cpp` | `868ab0b8ffa2` | runs |  |
| `libcxx/test/std/containers/associative/set/contains.pass.cpp` | `65f44690079c` | runs |  |
| `libcxx/test/std/containers/associative/set/contains_transparent.pass.cpp` | `53e63173cb89` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/count.pass.cpp` | `5584b1aee741` | runs |  |
| `libcxx/test/std/containers/associative/set/count_transparent.pass.cpp` | `cc6c8ec6805e` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/emplace.pass.cpp` | `a97e7837b0a8` | runs |  |
| `libcxx/test/std/containers/associative/set/emplace_hint.pass.cpp` | `c34b803be01e` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.pass.cpp` | `867fff1dd206` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.verify.cpp` | `41e7e25fd205` | verified |  |
| `libcxx/test/std/containers/associative/set/equal_range.pass.cpp` | `4583b139b1cd` | runs |  |
| `libcxx/test/std/containers/associative/set/equal_range_transparent.pass.cpp` | `da7036c6599d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/erase_iter.pass.cpp` | `909c375d1481` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_iter_iter.pass.cpp` | `62ef8c22ae43` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_key.pass.cpp` | `60869d49ba8b` | runs |  |
| `libcxx/test/std/containers/associative/set/extract_iterator.pass.cpp` | `9128487e04ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/extract_key.pass.cpp` | `772ad5bc7540` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/find.pass.cpp` | `ade8c52b1955` | runs |  |
| `libcxx/test/std/containers/associative/set/gcc_workaround.compile.pass.cpp` | `634150905da9` | compiles |  |
| `libcxx/test/std/containers/associative/set/get_allocator.pass.cpp` | `fc4089ed8899` | fails to compile | [container.alloc.reqmts]: an inplace set allocates nothing, so it has no allocator to return |
| `libcxx/test/std/containers/associative/set/incomplete_type.pass.cpp` | `b97dd11b85dc` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_and_emplace_allocator_requirements.pass.cpp` | `c30872880534` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_cv.pass.cpp` | `e97308343c7a` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_initializer_list.pass.cpp` | `0bc5f2a9ce75` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_cv.pass.cpp` | `f66a4fa7d485` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_iter.pass.cpp` | `a3718a9c57f0` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_iter_rv.pass.cpp` | `27478c59caea` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_node_type.pass.cpp` | `b35cd8ad9585` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_node_type_hint.pass.cpp` | `bfa4b3367805` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_range.pass.cpp` | `5ae2955e9078` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_rv.pass.cpp` | `4304561cb95c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/iterator.pass.cpp` | `819b7372dfe6` | runs |  |
| `libcxx/test/std/containers/associative/set/iterator_concept_conformance.compile.pass.cpp` | `a2025a27b5c2` | compiles |  |
| `libcxx/test/std/containers/associative/set/lower_bound.pass.cpp` | `4f7a2603a0f6` | runs |  |
| `libcxx/test/std/containers/associative/set/max_size.pass.cpp` | `ef6ff179df58` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/merge.pass.cpp` | `658c775174c6` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libcxx/test/std/containers/associative/set/range_concept_conformance.compile.pass.cpp` | `44fbd721985e` | compiles |  |
| `libcxx/test/std/containers/associative/set/set.cons/alloc.pass.cpp` | `aa43e9fcb26c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/assign_initializer_list.pass.cpp` | `f964d9f95928` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/compare.pass.cpp` | `0d8f91617a0f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_alloc.pass.cpp` | `6a6cfa91e0e2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_copy_constructible.compile.fail.cpp` | `ef04ba8973f2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy.pass.cpp` | `fb263a52b8d7` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_alloc.pass.cpp` | `08bc9f7177b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.addressof.compile.pass.cpp` | `42e4ed735d3c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.pass.cpp` | `980c27da3575` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.pass.cpp` | `67892454971e` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.verify.cpp` | `1bf1b172533f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/default.pass.cpp` | `e9b906681a03` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/default_noexcept.pass.cpp` | `c830d62c2869` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/dtor_noexcept.pass.cpp` | `3c74373bb576` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/from_range.pass.cpp` | `5c08a8ebab24` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list.pass.cpp` | `6c6ac53143bf` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare.pass.cpp` | `a69c0795ff30` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare_alloc.pass.cpp` | `c40cd541e87c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter.pass.cpp` | `bd55ceb68288` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_alloc.pass.cpp` | `703e27ee0053` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_comp.pass.cpp` | `f127416c851d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move.pass.cpp` | `d7d0d1fb2af5` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_alloc.pass.cpp` | `63dc391910eb` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign.pass.cpp` | `2e0f50070d13` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign_noexcept.compile.pass.cpp` | `3a2b32c868d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_noexcept.pass.cpp` | `5e61f09993b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.erasure/erase_if.pass.cpp` | `bf06247af20e` | runs, adapted | ASSERT_SAME_TYPE(typename S::size_type, decltype(std::erase_if(s, p))); (erase_if is xstd's, found by argument-dependent lookup); assert(expected_erased_count == std::erase_if(s, p)); (erase_if is xstd's, found by argument-dependent lookup) |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.pass.cpp` | `83d05f3bef66` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.verify.cpp` | `5a78a712cc54` | verified |  |
| `libcxx/test/std/containers/associative/set/set.nonmember/op_compare.pass.cpp` | `db0f2ccd8606` | runs |  |
| `libcxx/test/std/containers/associative/set/set.observers/comp.pass.cpp` | `7a51eb983905` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/member_swap.pass.cpp` | `e6db2e8ded52` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/non_member_swap.pass.cpp` | `e83b62b3de55` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/swap_noexcept.pass.cpp` | `9d0330c20bd4` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/size.pass.cpp` | `fb4ba712d3f1` | runs |  |
| `libcxx/test/std/containers/associative/set/types.pass.cpp` | `4b2d43c4a72a` | runs, adapted | static_assert((std::is_same<C::key_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::key_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::allocator_type, std::allocator<int> >::value), ""); (allocator_type is the block container's allocator); static_assert((std::is_same<C::reference, int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::const_reference, const int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::pointer, int*>::value), ""); (pointer is the set's own proxy); static_assert((std::is_same<C::const_pointer, const int*>::value), ""); (pointer is the set's own proxy) |
| `libcxx/test/std/containers/associative/set/upper_bound.pass.cpp` | `c8e7efe86b1d` | runs |  |

## libc++ against bit_small_set

- **Pinned:** [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) at `llvmorg-22.1.8`.
- **Mappings:** the suite's `test/std-test-suite/libcxx-*.toml` for the directories it takes, and `bit-small-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libcxx/test/std/containers/associative/set/allocator_mismatch.verify.cpp` | `652e404d93f1` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/clear.pass.cpp` | `868ab0b8ffa2` | runs |  |
| `libcxx/test/std/containers/associative/set/contains.pass.cpp` | `65f44690079c` | runs |  |
| `libcxx/test/std/containers/associative/set/contains_transparent.pass.cpp` | `53e63173cb89` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/count.pass.cpp` | `5584b1aee741` | runs |  |
| `libcxx/test/std/containers/associative/set/count_transparent.pass.cpp` | `cc6c8ec6805e` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/emplace.pass.cpp` | `a97e7837b0a8` | runs |  |
| `libcxx/test/std/containers/associative/set/emplace_hint.pass.cpp` | `c34b803be01e` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.pass.cpp` | `867fff1dd206` | runs |  |
| `libcxx/test/std/containers/associative/set/empty.verify.cpp` | `41e7e25fd205` | verified |  |
| `libcxx/test/std/containers/associative/set/equal_range.pass.cpp` | `4583b139b1cd` | runs |  |
| `libcxx/test/std/containers/associative/set/equal_range_transparent.pass.cpp` | `da7036c6599d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/erase_iter.pass.cpp` | `909c375d1481` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_iter_iter.pass.cpp` | `62ef8c22ae43` | runs |  |
| `libcxx/test/std/containers/associative/set/erase_key.pass.cpp` | `60869d49ba8b` | runs |  |
| `libcxx/test/std/containers/associative/set/extract_iterator.pass.cpp` | `9128487e04ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/extract_key.pass.cpp` | `772ad5bc7540` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/find.pass.cpp` | `ade8c52b1955` | runs |  |
| `libcxx/test/std/containers/associative/set/gcc_workaround.compile.pass.cpp` | `634150905da9` | compiles |  |
| `libcxx/test/std/containers/associative/set/get_allocator.pass.cpp` | `fc4089ed8899` | fails to compile | [container.alloc.reqmts]: the set's allocator is its storage's, Boost's small_vector allocator of blocks, not std::allocator<int> |
| `libcxx/test/std/containers/associative/set/incomplete_type.pass.cpp` | `b97dd11b85dc` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_and_emplace_allocator_requirements.pass.cpp` | `c30872880534` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_cv.pass.cpp` | `e97308343c7a` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_initializer_list.pass.cpp` | `0bc5f2a9ce75` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_cv.pass.cpp` | `f66a4fa7d485` | runs |  |
| `libcxx/test/std/containers/associative/set/insert_iter_iter.pass.cpp` | `a3718a9c57f0` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_iter_rv.pass.cpp` | `27478c59caea` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_node_type.pass.cpp` | `b35cd8ad9585` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_node_type_hint.pass.cpp` | `bfa4b3367805` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or insert |
| `libcxx/test/std/containers/associative/set/insert_range.pass.cpp` | `5ae2955e9078` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/insert_rv.pass.cpp` | `4304561cb95c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/iterator.pass.cpp` | `819b7372dfe6` | runs |  |
| `libcxx/test/std/containers/associative/set/iterator_concept_conformance.compile.pass.cpp` | `a2025a27b5c2` | compiles |  |
| `libcxx/test/std/containers/associative/set/lower_bound.pass.cpp` | `4f7a2603a0f6` | runs |  |
| `libcxx/test/std/containers/associative/set/max_size.pass.cpp` | `ef6ff179df58` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/merge.pass.cpp` | `658c775174c6` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libcxx/test/std/containers/associative/set/range_concept_conformance.compile.pass.cpp` | `44fbd721985e` | compiles |  |
| `libcxx/test/std/containers/associative/set/set.cons/alloc.pass.cpp` | `aa43e9fcb26c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/assign_initializer_list.pass.cpp` | `f964d9f95928` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/compare.pass.cpp` | `0d8f91617a0f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_alloc.pass.cpp` | `6a6cfa91e0e2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/compare_copy_constructible.compile.fail.cpp` | `ef04ba8973f2` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy.pass.cpp` | `fb263a52b8d7` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_alloc.pass.cpp` | `08bc9f7177b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.addressof.compile.pass.cpp` | `42e4ed735d3c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/copy_assign.pass.cpp` | `980c27da3575` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.pass.cpp` | `67892454971e` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libcxx/test/std/containers/associative/set/set.cons/deduct.verify.cpp` | `1bf1b172533f` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/default.pass.cpp` | `e9b906681a03` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/default_noexcept.pass.cpp` | `c830d62c2869` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/dtor_noexcept.pass.cpp` | `3c74373bb576` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/from_range.pass.cpp` | `5c08a8ebab24` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list.pass.cpp` | `6c6ac53143bf` | runs |  |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare.pass.cpp` | `a69c0795ff30` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/initializer_list_compare_alloc.pass.cpp` | `c40cd541e87c` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter.pass.cpp` | `bd55ceb68288` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_alloc.pass.cpp` | `703e27ee0053` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/iter_iter_comp.pass.cpp` | `f127416c851d` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move.pass.cpp` | `d7d0d1fb2af5` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_alloc.pass.cpp` | `63dc391910eb` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign.pass.cpp` | `2e0f50070d13` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_assign_noexcept.compile.pass.cpp` | `3a2b32c868d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.cons/move_noexcept.pass.cpp` | `5e61f09993b3` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.erasure/erase_if.pass.cpp` | `bf06247af20e` | runs, adapted | ASSERT_SAME_TYPE(typename S::size_type, decltype(std::erase_if(s, p))); (erase_if is xstd's, found by argument-dependent lookup); assert(expected_erased_count == std::erase_if(s, p)); (erase_if is xstd's, found by argument-dependent lookup) |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.pass.cpp` | `83d05f3bef66` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/set.nonmember/compare.three_way.verify.cpp` | `5a78a712cc54` | verified |  |
| `libcxx/test/std/containers/associative/set/set.nonmember/op_compare.pass.cpp` | `db0f2ccd8606` | runs |  |
| `libcxx/test/std/containers/associative/set/set.observers/comp.pass.cpp` | `7a51eb983905` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/member_swap.pass.cpp` | `e6db2e8ded52` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/non_member_swap.pass.cpp` | `e83b62b3de55` | runs |  |
| `libcxx/test/std/containers/associative/set/set.special/swap_noexcept.pass.cpp` | `9d0330c20bd4` | not run | no rewrite reaches it, so it tests the standard library |
| `libcxx/test/std/containers/associative/set/size.pass.cpp` | `fb4ba712d3f1` | runs |  |
| `libcxx/test/std/containers/associative/set/types.pass.cpp` | `4b2d43c4a72a` | runs, adapted | static_assert((std::is_same<C::key_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_type, int>::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::key_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::value_compare, std::less<int> >::value), ""); (the key the rewrite reads std::set<int> as, std::size_t); static_assert((std::is_same<C::allocator_type, std::allocator<int> >::value), ""); (allocator_type is the block container's allocator); static_assert((std::is_same<C::reference, int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::const_reference, const int&>::value), ""); (reference is the set's own proxy); static_assert((std::is_same<C::pointer, int*>::value), ""); (pointer is the set's own proxy); static_assert((std::is_same<C::const_pointer, const int*>::value), ""); (pointer is the set's own proxy) |
| `libcxx/test/std/containers/associative/set/upper_bound.pass.cpp` | `c8e7efe86b1d` | runs |  |

## libstdc++ against bit_vector

- **Pinned:** [https://github.com/gcc-mirror/gcc](https://github.com/gcc-mirror/gcc) at `releases/gcc-15.3.0`.
- **Mappings:** the suite's `test/std-test-suite/libstdcxx-*.toml` for the directories it takes, and `bit-vector.toml` for the type.
- **Not selected:** 1096 files that do not match `vector<bool>`, left to the standard library.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libstdc++-v3/testsuite/20_util/hash/84998.cc` | `27d02cad19c6` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/1.cc` | `b239d71e27f2` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/110807.cc` | `be1300b70a53` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/115098.cc` | `269eaf59548f` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/21244.cc` | `e84a5e45ba07` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/23632.cc` | `426ca1c086c6` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/6886.cc` | `9a3f630ea854` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/72847.cc` | `3fa84e8e0d9d` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/80893.cc` | `0c99b0b89fe8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/82558.cc` | `a125794728da` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/115854.cc` | `ae988fb519cc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/copy.cc` | `5944d3420346` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/copy_assign.cc` | `97855bc6f48a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/default_init.cc` | `98920de35e68` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/ext_ptr.cc` | `01416592b713` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/minimal.cc` | `0a85831ad8cc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/move.cc` | `dd30143bfff1` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/move_assign.cc` | `2089c513c356` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/noexcept.cc` | `921ac7262aa5` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/swap.cc` | `f275c3c69a8b` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/1.cc` | `25cbb017aab2` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/110498.cc` | `3e25b1ea157f` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/114758.cc` | `3e53f4132b5f` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/29134.cc` | `c9cd04e3b13f` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/constexpr.cc` | `f379b0056969` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/shrink_to_fit.cc` | `b49f915776e1` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/clear_allocator.cc` | `31ef8a5b6d38` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cmp_c++20.cc` | `1804758f43e8` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/1.cc` | `9c5f5ab72c9a` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/2.cc` | `72819fbb826a` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/87809.cc` | `5bc4c8d78579` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/constexpr.cc` | `851108289086` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/from_range.cc` | `aa8df55bdebb` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/lwg3778.cc` | `bb08ee37faf0` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/noexcept_move_construct.cc` | `c2ae85a02695` | compiles, adapted | static_assert( std::is_nothrow_constructible<vbtype, ([vector.overview] declares the move constructor with an allocator without noexcept, which libstdc++ adds as an extension) |
| `libstdc++-v3/testsuite/23_containers/vector/bool/element_access/1.cc` | `411c76dc9f86` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/element_access/constexpr.cc` | `a5004d73da49` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/emplace.cc` | `5ff6b81c9f10` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/emplace_cxx17_return.cc` | `0f5832cb9801` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/emplace_rvalue.cc` | `ec86a3c0fdab` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/format.cc` | `674b7cd4b4d5` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/hash/1.cc` | `1b0c9ac21dcc` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/iterator_c++20.cc` | `7e810e62f3e2` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/assign/1.cc` | `1d66a207486d` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/assign/assign_range.cc` | `b0ff689382a3` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/assign/constexpr.cc` | `5097184503d9` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/constexpr.cc` | `748b4a39e0bd` | compiles, adapted | v.flip(); ([vector.bool.pspc]/12: no flip(); the negation of every element is written element by element); v[2].flip(); ([vector.bool.pspc]/10: no reference::flip(); an element is negated by assignment); auto n = std::erase(e, false); ([vector.erasure]'s erase is std's for std::vector; a bit vector's is xstd's, found by argument-dependent lookup); n = std::erase_if(e, [](std::vector<bool>::reference val) { return !val; }); ([vector.erasure]'s erase_if is std's for std::vector; a bit vector's is xstd's, found by argument-dependent lookup) |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/erase/1.cc` | `7718fe8af97d` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/erase/54577.cc` | `39ed01e4b81c` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/1.cc` | `d823df99a0f5` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/104559.cc` | `e8d165705405` | not run | dg-require-normal-mode |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/31370.cc` | `92ca76abf9e9` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/append_range.cc` | `aaa3ca66e33a` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/const_iterator.cc` | `f00f67e0186c` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/insert_range.cc` | `7e37b4870485` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/swap/1.cc` | `cb5c28334c21` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/swap/2.cc` | `ebe64973f01f` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/swap/constexpr.cc` | `830691ce9681` | compiles, adapted | std::vector<bool>::swap(v1[0], v1[1]); ([vector.bool.pspc]/11 swaps two references through a hidden friend of reference (P3612R1); the test calls the static member it replaced) |
| `libstdc++-v3/testsuite/23_containers/vector/bool/requirements/citerators.cc` | `2f4a277f428a` | fails to compile | none of the bit vector's: libstdc++'s __gnu_test::citerator reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/vector/bool/requirements/dr438/constructor.cc` | `3fc8cf8bfb56` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/swap.cc` | `842f03de2a14` | runs |  |
| `libstdc++-v3/testsuite/24_iterators/const_iterator/1.cc` | `83bb53643f29` | runs, adapted | test01<std::vector<bool>::const_iterator, true>(); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); test02<std::vector<bool>, false, true>(); (const_reference is the bit vector's own proxy, so const_iterator_t is basic_const_iterator<const_iterator>, neither iterator_t<const R> nor basic_const_iterator<iterator>); test02<const std::vector<bool>, true>(); (const_reference is the bit vector's own proxy, so a const bit vector is no constant_range) |
| `libstdc++-v3/testsuite/24_iterators/contiguous/concept.cc` | `771d85e7bd6b` | compiles |  |
| `libstdc++-v3/testsuite/24_iterators/move_iterator/dr2106.cc` | `fb2dbfe6dc3b` | compiles, adapted | static_assert(std::is_same<MIt::reference, Ref>::value,""); (iter_move answers the bool, so a move_iterator's reference is bool rather than the proxy); static_assert(std::is_same<MCIt::reference, CRef>::value,""); (iter_move answers the bool, so a move_iterator's reference is bool rather than the proxy) |
| `libstdc++-v3/testsuite/25_algorithms/adjacent_find/vectorbool.cc` | `74612885f4f4` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/fill/bvector/1.cc` | `a830ac806c2d` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/find/vectorbool.cc` | `1f3b0788f453` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/find_end/vectorbool.cc` | `a9318a42ec78` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/find_first_of/vectorbool.cc` | `259f0dafee32` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/find_if/vectorbool.cc` | `1f3b0788f453` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/find_if_not/vectorbool.cc` | `cfc31f9060a3` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/heap/vectorbool.cc` | `905d0162284e` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/is_permutation/vectorbool.cc` | `3892fe38d6d7` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/iter_swap/20577.cc` | `c8a748e46430` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/sort/vectorbool.cc` | `b0ed19fb9b70` | runs |  |
| `libstdc++-v3/testsuite/25_algorithms/stable_sort/3.cc` | `26d4f78595b9` | runs |  |
| `libstdc++-v3/testsuite/std/ranges/adaptors/chunk_by/1.cc` | `e3dc711cf4a3` | runs |  |

## libstdc++ against bit_small_vector

- **Pinned:** [https://github.com/gcc-mirror/gcc](https://github.com/gcc-mirror/gcc) at `releases/gcc-15.3.0`.
- **Mappings:** the suite's `test/std-test-suite/libstdcxx-*.toml` for the directories it takes, and `bit-small-vector.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libstdc++-v3/testsuite/23_containers/vector/bool/1.cc` | `b239d71e27f2` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/110807.cc` | `be1300b70a53` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/115098.cc` | `269eaf59548f` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/21244.cc` | `e84a5e45ba07` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/23632.cc` | `426ca1c086c6` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/6886.cc` | `9a3f630ea854` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/72847.cc` | `3fa84e8e0d9d` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/80893.cc` | `0c99b0b89fe8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/82558.cc` | `a125794728da` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/115854.cc` | `ae988fb519cc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/copy.cc` | `5944d3420346` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/copy_assign.cc` | `97855bc6f48a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/default_init.cc` | `98920de35e68` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/ext_ptr.cc` | `01416592b713` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/minimal.cc` | `0a85831ad8cc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/move.cc` | `dd30143bfff1` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/move_assign.cc` | `2089c513c356` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/noexcept.cc` | `921ac7262aa5` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/allocator/swap.cc` | `f275c3c69a8b` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/1.cc` | `25cbb017aab2` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/110498.cc` | `3e25b1ea157f` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/114758.cc` | `3e53f4132b5f` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/29134.cc` | `c9cd04e3b13f` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/constexpr.cc` | `f379b0056969` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/capacity/shrink_to_fit.cc` | `b49f915776e1` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/clear_allocator.cc` | `31ef8a5b6d38` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cmp_c++20.cc` | `1804758f43e8` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/1.cc` | `9c5f5ab72c9a` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/2.cc` | `72819fbb826a` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/87809.cc` | `5bc4c8d78579` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/constexpr.cc` | `851108289086` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/from_range.cc` | `aa8df55bdebb` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/lwg3778.cc` | `bb08ee37faf0` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/cons/noexcept_move_construct.cc` | `c2ae85a02695` | compiles, adapted | static_assert( std::is_nothrow_constructible<vbtype, ([vector.overview] declares the move constructor with an allocator without noexcept, which libstdc++ adds as an extension) |
| `libstdc++-v3/testsuite/23_containers/vector/bool/element_access/1.cc` | `411c76dc9f86` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/element_access/constexpr.cc` | `a5004d73da49` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/emplace.cc` | `5ff6b81c9f10` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/emplace_cxx17_return.cc` | `0f5832cb9801` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/emplace_rvalue.cc` | `ec86a3c0fdab` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/format.cc` | `674b7cd4b4d5` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/hash/1.cc` | `1b0c9ac21dcc` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/iterator_c++20.cc` | `7e810e62f3e2` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/assign/1.cc` | `1d66a207486d` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/assign/assign_range.cc` | `b0ff689382a3` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/assign/constexpr.cc` | `5097184503d9` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/constexpr.cc` | `748b4a39e0bd` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/erase/1.cc` | `7718fe8af97d` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/erase/54577.cc` | `39ed01e4b81c` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/1.cc` | `d823df99a0f5` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/104559.cc` | `e8d165705405` | not run | dg-require-normal-mode |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/31370.cc` | `92ca76abf9e9` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/append_range.cc` | `aaa3ca66e33a` | runs, adapted | static_assert( test_constexpr() ); (Boost's small_vector is not usable in a constant expression) |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/const_iterator.cc` | `f00f67e0186c` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/insert/insert_range.cc` | `7e37b4870485` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/swap/1.cc` | `cb5c28334c21` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/swap/2.cc` | `ebe64973f01f` | runs |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/modifiers/swap/constexpr.cc` | `830691ce9681` | fails to compile | [containers]: Boost's small_vector is not usable in a constant expression, so neither is a bit vector over it |
| `libstdc++-v3/testsuite/23_containers/vector/bool/requirements/citerators.cc` | `2f4a277f428a` | fails to compile | none of the bit vector's: libstdc++'s __gnu_test::citerator reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/vector/bool/requirements/dr438/constructor.cc` | `3fc8cf8bfb56` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/vector/bool/swap.cc` | `842f03de2a14` | runs |  |

## libstdc++ against bit_set

- **Pinned:** [https://github.com/gcc-mirror/gcc](https://github.com/gcc-mirror/gcc) at `releases/gcc-15.3.0`.
- **Mappings:** the suite's `test/std-test-suite/libstdcxx-*.toml` for the directories it takes, and `bit-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libstdc++-v3/testsuite/23_containers/set/14340.cc` | `8ca7c4598091` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/18604.cc` | `e75fd7a7647e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/23781_neg.cc` | `385b02b013d2` | rejected |  |
| `libstdc++-v3/testsuite/23_containers/set/48101-2_neg.cc` | `b1ae60d36967` | not run | dg-add-options strict_std, which this adapter does not interpret |
| `libstdc++-v3/testsuite/23_containers/set/48101-3_neg.cc` | `2801895e66c1` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/48101_neg.cc` | `61e673eef398` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/58764.cc` | `d30ac2979c4b` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/85965.cc` | `46628f3c5814` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/92124.cc` | `b525b7998dca` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/103501.cc` | `af062909f7f8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/71964.cc` | `762472679152` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy.cc` | `c4a9538337e0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy_assign.cc` | `926a979c7386` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/default_init.cc` | `4e1155f45ce2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/ext_ptr.cc` | `ac3be6c483d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/init-list.cc` | `dcd3153c0811` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/minimal.cc` | `695b1cd30388` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move.cc` | `4f332bbeb2a8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_assign.cc` | `f43319b474da` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_cons.cc` | `464cd0ee9360` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/noexcept.cc` | `687f399e7d2f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/swap.cc` | `df4f1f6a2f38` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/capacity/29134.cc` | `1b0c36dcf586` | runs, adapted | VERIFY( s.max_size() == __gnu_test::max_size(a) ); (max_size(), which [container.reqmts] leaves to the implementation, is libstdc++'s tree node count here) |
| `libstdc++-v3/testsuite/23_containers/set/check_construct_destroy.cc` | `9cd7336b0b12` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/61023.cc` | `04cb8b499db4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/deduction.cc` | `31f823d5b496` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libstdc++-v3/testsuite/23_containers/set/cons/from_range.cc` | `c55f94862e89` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/moveable.cc` | `dd6e842f78a8` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_default_construct.cc` | `72ce52d65d70` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_move_construct.cc` | `c85c45fb3570` | compiles, adapted | static_assert( std::is_nothrow_constructible<stype, ([set.overview] declares the move constructor with an allocator without noexcept, which libstdc++ adds as an extension) |
| `libstdc++-v3/testsuite/23_containers/set/cons/self_move.cc` | `2dc2c7e0fd78` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/debug/114316.cc` | `38a5d4b4c86f` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/60499.cc` | `c979b567b671` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/90102.cc` | `53b22a10de5d` | fails to compile | none of the set's: a test of libstdc++'s debug-mode std::__debug::set |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct1_neg.cc` | `986a90652e00` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct2_neg.cc` | `4f059f7eee3c` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct3_neg.cc` | `b979292cd0fb` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct4_neg.cc` | `18a5dcc9e58a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct5_neg.cc` | `b9f98b92c1fc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert1_neg.cc` | `edceb683c54a` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert2_neg.cc` | `7c6a47d8221b` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert3_neg.cc` | `b518e60ea0db` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert4_neg.cc` | `f02fab0f4f67` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/1.cc` | `916785fa1b1f` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/2.cc` | `9e5c00d1e9e0` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_assign_neg.cc` | `2529bce4d6a3` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_neg.cc` | `00761113b543` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/erasure.cc` | `f1448b4a42c4` | runs, adapted | auto num = std::erase_if(s, is_odd); (erase_if is xstd's, found by argument-dependent lookup); std::erase_if(s, pred); (erase_if is xstd's, found by argument-dependent lookup) |
| `libstdc++-v3/testsuite/23_containers/set/init-list.cc` | `1a32d769c68f` | runs, adapted | __gnu_test::set_memory_limits(); (none of the set's: the memory limit is set by libstdc++'s test support library, which no consumer links) |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/114401.cc` | `f52e0a46d3c9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/16728.cc` | `34251006fef4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/17948.cc` | `65b521036b4c` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/dr130.cc` | `e54e82e18827` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/1.cc` | `68de43084794` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/92878_92947.cc` | `388b69763da2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/51142.cc` | `4c12ac79a9a9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/abi_tag.cc` | `4172e524714b` | not run | dg-require-normal-mode |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/dr130-linkage-check.cc` | `79abb42457bf` | not run | dg-require-effective-target ia32 |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/extract.cc` | `3898dc149ffb` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/1.cc` | `425016942069` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/2.cc` | `777671a9096e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/3.cc` | `1093d434d546` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/87194.cc` | `bae69d6f2d40` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/insert_range.cc` | `d627bab8eb3f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/merge.cc` | `64cd1d48163c` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/node_swap.cc` | `63fba8bfd4ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/1.cc` | `7f3e45d73237` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/2.cc` | `c512b7aaaff0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/3.cc` | `5f03aec3b103` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/adl.cc` | `945cabfbddb7` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/move_comparison.cc` | `8a049456d86d` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operations/1.cc` | `6ddb29a55397` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/2.cc` | `4a06d7411f9b` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/contains.cc` | `945a453d9816` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/count.cc` | `9c059115dcfa` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/equal_range_adl.cc` | `91eae0255523` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/1_neg.cc` | `2a8d8b8fde22` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/cmp_c++20.cc` | `97c181291623` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs.cc` | `bd4cdd0b6bea` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs_debug.cc` | `5b648ccf1e94` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/range_access.cc` | `da4c0f5f4a00` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/citerators.cc` | `582b06fee64b` | fails to compile | none of the set's: libstdc++'s __gnu_test::citerator reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/basic.cc` | `50b601537097` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/generation_prohibited.cc` | `abd082c2ca8f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/propagation_consistent.cc` | `040f3f6e3656` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/1.cc` | `9fd0346fc798` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/2.cc` | `498cab28880a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/3.cc` | `d11a4f73b9b9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/4.cc` | `a6dd32be2501` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/5.cc` | `92b8be19bb23` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr.cc` | `01619de7cecc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr_ignored.cc` | `8c9492ccfc61` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/partial_specialization/1.cc` | `779da47c0c5a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/typedefs.cc` | `0a0a7a569ecb` | fails to compile | none of the set's: libstdc++'s __gnu_test::types reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/version.cc` | `55ad5d0e4e12` | not run | dg-do preprocess, which builds no test |

## libstdc++ against bit_fixed_set

- **Pinned:** [https://github.com/gcc-mirror/gcc](https://github.com/gcc-mirror/gcc) at `releases/gcc-15.3.0`.
- **Mappings:** the suite's `test/std-test-suite/libstdcxx-*.toml` for the directories it takes, and `bit-fixed-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libstdc++-v3/testsuite/23_containers/set/14340.cc` | `8ca7c4598091` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/18604.cc` | `e75fd7a7647e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/23781_neg.cc` | `385b02b013d2` | rejected |  |
| `libstdc++-v3/testsuite/23_containers/set/48101-2_neg.cc` | `b1ae60d36967` | not run | dg-add-options strict_std, which this adapter does not interpret |
| `libstdc++-v3/testsuite/23_containers/set/48101-3_neg.cc` | `2801895e66c1` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/48101_neg.cc` | `61e673eef398` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/58764.cc` | `d30ac2979c4b` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/85965.cc` | `46628f3c5814` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/92124.cc` | `b525b7998dca` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/103501.cc` | `af062909f7f8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/71964.cc` | `762472679152` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy.cc` | `c4a9538337e0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy_assign.cc` | `926a979c7386` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/default_init.cc` | `4e1155f45ce2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/ext_ptr.cc` | `ac3be6c483d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/init-list.cc` | `dcd3153c0811` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/minimal.cc` | `695b1cd30388` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move.cc` | `4f332bbeb2a8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_assign.cc` | `f43319b474da` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_cons.cc` | `464cd0ee9360` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/noexcept.cc` | `687f399e7d2f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/swap.cc` | `df4f1f6a2f38` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/capacity/29134.cc` | `1b0c36dcf586` | runs, adapted | VERIFY( s.max_size() == __gnu_test::max_size(a) ); (max_size(), which [container.reqmts] leaves to the implementation, is libstdc++'s tree node count here) |
| `libstdc++-v3/testsuite/23_containers/set/check_construct_destroy.cc` | `9cd7336b0b12` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/61023.cc` | `04cb8b499db4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/deduction.cc` | `31f823d5b496` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libstdc++-v3/testsuite/23_containers/set/cons/from_range.cc` | `c55f94862e89` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/moveable.cc` | `dd6e842f78a8` | runs, adapted | VERIFY( b.find(2) == b.end()); ([container.reqmts] leaves a moved-from container's value unspecified: libstdc++'s nodes leave it empty, and a fixed set's array of blocks moves by copying) |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_default_construct.cc` | `72ce52d65d70` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_move_construct.cc` | `c85c45fb3570` | fails to compile | [container.alloc.reqmts]: a fixed set allocates nothing, so it has no allocator_type to move with |
| `libstdc++-v3/testsuite/23_containers/set/cons/self_move.cc` | `2dc2c7e0fd78` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/debug/114316.cc` | `38a5d4b4c86f` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/60499.cc` | `c979b567b671` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/90102.cc` | `53b22a10de5d` | fails to compile | none of the set's: a test of libstdc++'s debug-mode std::__debug::set |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct1_neg.cc` | `986a90652e00` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct2_neg.cc` | `4f059f7eee3c` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct3_neg.cc` | `b979292cd0fb` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct4_neg.cc` | `18a5dcc9e58a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct5_neg.cc` | `b9f98b92c1fc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert1_neg.cc` | `edceb683c54a` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert2_neg.cc` | `7c6a47d8221b` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert3_neg.cc` | `b518e60ea0db` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert4_neg.cc` | `f02fab0f4f67` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/1.cc` | `916785fa1b1f` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/2.cc` | `9e5c00d1e9e0` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_assign_neg.cc` | `2529bce4d6a3` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_neg.cc` | `00761113b543` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/erasure.cc` | `f1448b4a42c4` | runs, adapted | auto num = std::erase_if(s, is_odd); (erase_if is xstd's, found by argument-dependent lookup); std::erase_if(s, pred); (erase_if is xstd's, found by argument-dependent lookup) |
| `libstdc++-v3/testsuite/23_containers/set/init-list.cc` | `1a32d769c68f` | runs, adapted | __gnu_test::set_memory_limits(); (none of the set's: the memory limit is set by libstdc++'s test support library, which no consumer links) |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/114401.cc` | `f52e0a46d3c9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/16728.cc` | `34251006fef4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/17948.cc` | `65b521036b4c` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/dr130.cc` | `e54e82e18827` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/1.cc` | `68de43084794` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/92878_92947.cc` | `388b69763da2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/51142.cc` | `4c12ac79a9a9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/abi_tag.cc` | `4172e524714b` | not run | dg-require-normal-mode |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/dr130-linkage-check.cc` | `79abb42457bf` | not run | dg-require-effective-target ia32 |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/extract.cc` | `3898dc149ffb` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/1.cc` | `425016942069` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/2.cc` | `777671a9096e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/3.cc` | `1093d434d546` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/87194.cc` | `bae69d6f2d40` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/insert_range.cc` | `d627bab8eb3f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/merge.cc` | `64cd1d48163c` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/node_swap.cc` | `63fba8bfd4ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/1.cc` | `7f3e45d73237` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/2.cc` | `c512b7aaaff0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/3.cc` | `5f03aec3b103` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/adl.cc` | `945cabfbddb7` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/move_comparison.cc` | `8a049456d86d` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operations/1.cc` | `6ddb29a55397` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/2.cc` | `4a06d7411f9b` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/contains.cc` | `945a453d9816` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/count.cc` | `9c059115dcfa` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/equal_range_adl.cc` | `91eae0255523` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/1_neg.cc` | `2a8d8b8fde22` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/cmp_c++20.cc` | `97c181291623` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs.cc` | `bd4cdd0b6bea` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs_debug.cc` | `5b648ccf1e94` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/range_access.cc` | `da4c0f5f4a00` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/citerators.cc` | `582b06fee64b` | fails to compile | none of the set's: libstdc++'s __gnu_test::citerator reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/basic.cc` | `50b601537097` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/generation_prohibited.cc` | `abd082c2ca8f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/propagation_consistent.cc` | `040f3f6e3656` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/1.cc` | `9fd0346fc798` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/2.cc` | `498cab28880a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/3.cc` | `d11a4f73b9b9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/4.cc` | `a6dd32be2501` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/5.cc` | `92b8be19bb23` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr.cc` | `01619de7cecc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr_ignored.cc` | `8c9492ccfc61` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/partial_specialization/1.cc` | `779da47c0c5a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/typedefs.cc` | `0a0a7a569ecb` | fails to compile | none of the set's: libstdc++'s __gnu_test::types reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/version.cc` | `55ad5d0e4e12` | not run | dg-do preprocess, which builds no test |

## libstdc++ against bit_inplace_set

- **Pinned:** [https://github.com/gcc-mirror/gcc](https://github.com/gcc-mirror/gcc) at `releases/gcc-15.3.0`.
- **Mappings:** the suite's `test/std-test-suite/libstdcxx-*.toml` for the directories it takes, and `bit-inplace-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libstdc++-v3/testsuite/23_containers/set/14340.cc` | `8ca7c4598091` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/18604.cc` | `e75fd7a7647e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/23781_neg.cc` | `385b02b013d2` | rejected |  |
| `libstdc++-v3/testsuite/23_containers/set/48101-2_neg.cc` | `b1ae60d36967` | not run | dg-add-options strict_std, which this adapter does not interpret |
| `libstdc++-v3/testsuite/23_containers/set/48101-3_neg.cc` | `2801895e66c1` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/48101_neg.cc` | `61e673eef398` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/58764.cc` | `d30ac2979c4b` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/85965.cc` | `46628f3c5814` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/92124.cc` | `b525b7998dca` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/103501.cc` | `af062909f7f8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/71964.cc` | `762472679152` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy.cc` | `c4a9538337e0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy_assign.cc` | `926a979c7386` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/default_init.cc` | `4e1155f45ce2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/ext_ptr.cc` | `ac3be6c483d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/init-list.cc` | `dcd3153c0811` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/minimal.cc` | `695b1cd30388` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move.cc` | `4f332bbeb2a8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_assign.cc` | `f43319b474da` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_cons.cc` | `464cd0ee9360` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/noexcept.cc` | `687f399e7d2f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/swap.cc` | `df4f1f6a2f38` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/capacity/29134.cc` | `1b0c36dcf586` | runs, adapted | VERIFY( s.max_size() == __gnu_test::max_size(a) ); (max_size(), which [container.reqmts] leaves to the implementation, is libstdc++'s tree node count here) |
| `libstdc++-v3/testsuite/23_containers/set/check_construct_destroy.cc` | `9cd7336b0b12` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/61023.cc` | `04cb8b499db4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/deduction.cc` | `31f823d5b496` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libstdc++-v3/testsuite/23_containers/set/cons/from_range.cc` | `c55f94862e89` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/moveable.cc` | `dd6e842f78a8` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_default_construct.cc` | `72ce52d65d70` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_move_construct.cc` | `c85c45fb3570` | fails to compile | [container.alloc.reqmts]: an inplace set allocates nothing, so it has no allocator_type to move with |
| `libstdc++-v3/testsuite/23_containers/set/cons/self_move.cc` | `2dc2c7e0fd78` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/debug/114316.cc` | `38a5d4b4c86f` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/60499.cc` | `c979b567b671` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/90102.cc` | `53b22a10de5d` | fails to compile | none of the set's: a test of libstdc++'s debug-mode std::__debug::set |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct1_neg.cc` | `986a90652e00` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct2_neg.cc` | `4f059f7eee3c` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct3_neg.cc` | `b979292cd0fb` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct4_neg.cc` | `18a5dcc9e58a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct5_neg.cc` | `b9f98b92c1fc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert1_neg.cc` | `edceb683c54a` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert2_neg.cc` | `7c6a47d8221b` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert3_neg.cc` | `b518e60ea0db` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert4_neg.cc` | `f02fab0f4f67` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/1.cc` | `916785fa1b1f` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/2.cc` | `9e5c00d1e9e0` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_assign_neg.cc` | `2529bce4d6a3` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_neg.cc` | `00761113b543` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/erasure.cc` | `f1448b4a42c4` | runs, adapted | auto num = std::erase_if(s, is_odd); (erase_if is xstd's, found by argument-dependent lookup); std::erase_if(s, pred); (erase_if is xstd's, found by argument-dependent lookup) |
| `libstdc++-v3/testsuite/23_containers/set/init-list.cc` | `1a32d769c68f` | runs, adapted | __gnu_test::set_memory_limits(); (none of the set's: the memory limit is set by libstdc++'s test support library, which no consumer links) |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/114401.cc` | `f52e0a46d3c9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/16728.cc` | `34251006fef4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/17948.cc` | `65b521036b4c` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/dr130.cc` | `e54e82e18827` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/1.cc` | `68de43084794` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/92878_92947.cc` | `388b69763da2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/51142.cc` | `4c12ac79a9a9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/abi_tag.cc` | `4172e524714b` | not run | dg-require-normal-mode |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/dr130-linkage-check.cc` | `79abb42457bf` | not run | dg-require-effective-target ia32 |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/extract.cc` | `3898dc149ffb` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/1.cc` | `425016942069` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/2.cc` | `777671a9096e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/3.cc` | `1093d434d546` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/87194.cc` | `bae69d6f2d40` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/insert_range.cc` | `d627bab8eb3f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/merge.cc` | `64cd1d48163c` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/node_swap.cc` | `63fba8bfd4ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/1.cc` | `7f3e45d73237` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/2.cc` | `c512b7aaaff0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/3.cc` | `5f03aec3b103` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/adl.cc` | `945cabfbddb7` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/move_comparison.cc` | `8a049456d86d` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operations/1.cc` | `6ddb29a55397` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/2.cc` | `4a06d7411f9b` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/contains.cc` | `945a453d9816` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/count.cc` | `9c059115dcfa` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/equal_range_adl.cc` | `91eae0255523` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/1_neg.cc` | `2a8d8b8fde22` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/cmp_c++20.cc` | `97c181291623` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs.cc` | `bd4cdd0b6bea` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs_debug.cc` | `5b648ccf1e94` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/range_access.cc` | `da4c0f5f4a00` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/citerators.cc` | `582b06fee64b` | fails to compile | none of the set's: libstdc++'s __gnu_test::citerator reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/basic.cc` | `50b601537097` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/generation_prohibited.cc` | `abd082c2ca8f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/propagation_consistent.cc` | `040f3f6e3656` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/1.cc` | `9fd0346fc798` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/2.cc` | `498cab28880a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/3.cc` | `d11a4f73b9b9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/4.cc` | `a6dd32be2501` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/5.cc` | `92b8be19bb23` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr.cc` | `01619de7cecc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr_ignored.cc` | `8c9492ccfc61` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/partial_specialization/1.cc` | `779da47c0c5a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/typedefs.cc` | `0a0a7a569ecb` | fails to compile | none of the set's: libstdc++'s __gnu_test::types reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/version.cc` | `55ad5d0e4e12` | not run | dg-do preprocess, which builds no test |

## libstdc++ against bit_small_set

- **Pinned:** [https://github.com/gcc-mirror/gcc](https://github.com/gcc-mirror/gcc) at `releases/gcc-15.3.0`.
- **Mappings:** the suite's `test/std-test-suite/libstdcxx-*.toml` for the directories it takes, and `bit-small-set.toml` for the type.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `libstdc++-v3/testsuite/23_containers/set/14340.cc` | `8ca7c4598091` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/18604.cc` | `e75fd7a7647e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/23781_neg.cc` | `385b02b013d2` | rejected |  |
| `libstdc++-v3/testsuite/23_containers/set/48101-2_neg.cc` | `b1ae60d36967` | not run | dg-add-options strict_std, which this adapter does not interpret |
| `libstdc++-v3/testsuite/23_containers/set/48101-3_neg.cc` | `2801895e66c1` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/48101_neg.cc` | `61e673eef398` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/58764.cc` | `d30ac2979c4b` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/85965.cc` | `46628f3c5814` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/92124.cc` | `b525b7998dca` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/103501.cc` | `af062909f7f8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/71964.cc` | `762472679152` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy.cc` | `c4a9538337e0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/copy_assign.cc` | `926a979c7386` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/default_init.cc` | `4e1155f45ce2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/ext_ptr.cc` | `ac3be6c483d9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/init-list.cc` | `dcd3153c0811` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/minimal.cc` | `695b1cd30388` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move.cc` | `4f332bbeb2a8` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_assign.cc` | `f43319b474da` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/move_cons.cc` | `464cd0ee9360` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/noexcept.cc` | `687f399e7d2f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/allocator/swap.cc` | `df4f1f6a2f38` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/capacity/29134.cc` | `1b0c36dcf586` | runs, adapted | VERIFY( s.max_size() == __gnu_test::max_size(a) ); (max_size(), which [container.reqmts] leaves to the implementation, is libstdc++'s tree node count here) |
| `libstdc++-v3/testsuite/23_containers/set/check_construct_destroy.cc` | `9cd7336b0b12` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/61023.cc` | `04cb8b499db4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/deduction.cc` | `31f823d5b496` | fails to compile | [associative.reqmts.general]/181: the rewrite names the subject where std::set has arguments; a deduced std::set stays std::set<int>, which the test compares with the subject |
| `libstdc++-v3/testsuite/23_containers/set/cons/from_range.cc` | `c55f94862e89` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/cons/moveable.cc` | `dd6e842f78a8` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_default_construct.cc` | `72ce52d65d70` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/cons/noexcept_move_construct.cc` | `c85c45fb3570` | compiles, adapted | static_assert( std::is_nothrow_constructible<stype, ([set.overview] declares the move constructor with an allocator without noexcept, which libstdc++ adds as an extension) |
| `libstdc++-v3/testsuite/23_containers/set/cons/self_move.cc` | `2dc2c7e0fd78` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/debug/114316.cc` | `38a5d4b4c86f` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/60499.cc` | `c979b567b671` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/90102.cc` | `53b22a10de5d` | fails to compile | none of the set's: a test of libstdc++'s debug-mode std::__debug::set |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct1_neg.cc` | `986a90652e00` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct2_neg.cc` | `4f059f7eee3c` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct3_neg.cc` | `b979292cd0fb` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct4_neg.cc` | `18a5dcc9e58a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/construct5_neg.cc` | `b9f98b92c1fc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert1_neg.cc` | `edceb683c54a` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert2_neg.cc` | `7c6a47d8221b` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert3_neg.cc` | `b518e60ea0db` | not run | dg-require-debug-mode |
| `libstdc++-v3/testsuite/23_containers/set/debug/insert4_neg.cc` | `f02fab0f4f67` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/1.cc` | `916785fa1b1f` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/invalidation/2.cc` | `9e5c00d1e9e0` | fails to compile | none of the set's: a test of libstdc++'s debug-mode iterator invalidation |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_assign_neg.cc` | `2529bce4d6a3` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/debug/move_neg.cc` | `00761113b543` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/erasure.cc` | `f1448b4a42c4` | runs, adapted | auto num = std::erase_if(s, is_odd); (erase_if is xstd's, found by argument-dependent lookup); std::erase_if(s, pred); (erase_if is xstd's, found by argument-dependent lookup) |
| `libstdc++-v3/testsuite/23_containers/set/init-list.cc` | `1a32d769c68f` | runs, adapted | __gnu_test::set_memory_limits(); (none of the set's: the memory limit is set by libstdc++'s test support library, which no consumer links) |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/114401.cc` | `f52e0a46d3c9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/16728.cc` | `34251006fef4` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/17948.cc` | `65b521036b4c` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/dr130.cc` | `e54e82e18827` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/1.cc` | `68de43084794` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/emplace/92878_92947.cc` | `388b69763da2` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/51142.cc` | `4c12ac79a9a9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/abi_tag.cc` | `4172e524714b` | not run | dg-require-normal-mode |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/erase/dr130-linkage-check.cc` | `79abb42457bf` | not run | dg-require-effective-target ia32 |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/extract.cc` | `3898dc149ffb` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/1.cc` | `425016942069` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/2.cc` | `777671a9096e` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/3.cc` | `1093d434d546` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/87194.cc` | `bae69d6f2d40` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/insert/insert_range.cc` | `d627bab8eb3f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/merge.cc` | `64cd1d48163c` | fails to compile | [associative.reqmts.general]/112-117: no nodes to splice; a \|= b inserts every key of b and leaves b unchanged |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/node_swap.cc` | `63fba8bfd4ad` | fails to compile | [associative.reqmts.general]/17: no nodes; a key is a bit in a block, so there is no node_type to extract or swap |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/1.cc` | `7f3e45d73237` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/2.cc` | `c512b7aaaff0` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/3.cc` | `5f03aec3b103` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/modifiers/swap/adl.cc` | `945cabfbddb7` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/move_comparison.cc` | `8a049456d86d` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operations/1.cc` | `6ddb29a55397` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/2.cc` | `4a06d7411f9b` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/contains.cc` | `945a453d9816` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/count.cc` | `9c059115dcfa` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/operations/equal_range_adl.cc` | `91eae0255523` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/1_neg.cc` | `2a8d8b8fde22` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/operators/cmp_c++20.cc` | `97c181291623` | runs |  |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs.cc` | `bd4cdd0b6bea` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/pmr_typedefs_debug.cc` | `5b648ccf1e94` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/range_access.cc` | `da4c0f5f4a00` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/citerators.cc` | `582b06fee64b` | fails to compile | none of the set's: libstdc++'s __gnu_test::citerator reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/basic.cc` | `50b601537097` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/generation_prohibited.cc` | `abd082c2ca8f` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/exception/propagation_consistent.cc` | `040f3f6e3656` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/1.cc` | `9fd0346fc798` | compiles |  |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/2.cc` | `498cab28880a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/3.cc` | `d11a4f73b9b9` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/4.cc` | `a6dd32be2501` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/5.cc` | `92b8be19bb23` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr.cc` | `01619de7cecc` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/explicit_instantiation/alloc_ptr_ignored.cc` | `8c9492ccfc61` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/partial_specialization/1.cc` | `779da47c0c5a` | not run | no rewrite reaches it, so it tests the standard library |
| `libstdc++-v3/testsuite/23_containers/set/requirements/typedefs.cc` | `0a0a7a569ecb` | fails to compile | none of the set's: libstdc++'s __gnu_test::types reads __gnu_test::traits, which describes the standard containers alone |
| `libstdc++-v3/testsuite/23_containers/set/version.cc` | `55ad5d0e4e12` | not run | dg-do preprocess, which builds no test |

## MSVC STL against bit_vector

- **Pinned:** [https://github.com/microsoft/STL](https://github.com/microsoft/STL) at `vs-2022-17.14`.
- **Mappings:** the suite's `test/std-test-suite/msvc-stl-*.toml` for the directories it takes, and `bit-vector.toml` for the type.
- **Not selected:** 1657 files that do not match `vector<bool>`, left to the standard library.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `tests/std/tests/Dev10_563443_empty_vector_begin_plus_zero/test.cpp` | `aa8374c89c38` | runs |  |
| `tests/std/tests/Dev10_567556_move_from_empty_list/test.cpp` | `542aa9329fd1` | runs |  |
| `tests/std/tests/Dev10_689595_back_inserter_vector_bool/test.cpp` | `925fd499ca3c` | runs |  |
| `tests/std/tests/Dev10_709168_marking_iterators_as_checked/test.compile.pass.cpp` | `1b28f193ecf3` | compiles, adapted | STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked); STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::const_iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked); STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::reverse_iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked); STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::const_reverse_iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked) |
| `tests/std/tests/Dev10_816787_swap_vector_bool_elements/test.cpp` | `a55840c127a8` | runs |  |
| `tests/std/tests/Dev11_0000000_null_forward_iterators/test.cpp` | `011e4c52769c` | runs |  |
| `tests/std/tests/Dev11_0000000_user_defined_literals/test.cpp` | `1147e8aa2c1c` | runs |  |
| `tests/std/tests/Dev11_0135139_vector_bool_comparisons/test.cpp` | `7e586b4f2a2c` | runs, adapted | x.back().flip(); (no reference::flip()); y.back().flip(); (no reference::flip()) |
| `tests/std/tests/Dev11_0437519_container_requirements/test.compile.pass.cpp` | `324b9cdcd611` | compiles |  |
| `tests/std/tests/Dev11_0483851_vector_debug_allocator_use/test.compile.pass.cpp` | `28ec0ef3a3e2` | compiles |  |
| `tests/std/tests/GH_000625_vector_bool_optimization/test.cpp` | `f8a75dba8e34` | runs, adapted | static_assert(blockSize == _VBITS, "Invalid block size"); (the MSVC STL's internal _VBITS, the width of its words; the test's data crosses word boundaries at any width); input_true[length + 1].flip(); (no reference::flip()); input_false[length + 1].flip(); (no reference::flip()); input_true[0].flip(); (no reference::flip()); input_false[0].flip(); (no reference::flip()); void print_vec(const char* const name, const vector<T>& v) { (the test's diagnostic helper takes a std::vector<T>, which a bit vector is not, so it deduces the container instead) |
| `tests/std/tests/GH_001638_dllexport_derived_classes/test.compile.pass.cpp` | `8f5be5f6ee2b` | fails to compile | none of the bit vector's: MSVC rejects exporting a class whose base instantiates a static data member template, reported as rhalbersma/xstd-bits#529 |
| `tests/std/tests/GH_005090_stl_hardening/test.cpp` | `5ad23c13f246` | runs |  |
| `tests/std/tests/GH_005315_destructor_tombstones/test.cpp` | `60086bed0724` | runs |  |
| `tests/std/tests/P0009R18_mdspan_mdspan/test.cpp` | `7a9d4f5ed8c1` | runs |  |
| `tests/std/tests/P0088R3_variant/test.cpp` | `dc168121c680` | not run | a copy of a libc++ test, whose msvc_stdlib_force_include.h lives in the STL's LLVM submodule, which this adapter does not fetch |
| `tests/std/tests/P0433R2_deduction_guides/test.cpp` | `c5fde5817992` | runs, adapted | static_assert(is_same_v<decltype(vb1), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector); static_assert(is_same_v<decltype(vb3), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector); static_assert(is_same_v<decltype(vb5), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector); static_assert(is_same_v<decltype(vb8), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector) |
| `tests/std/tests/P0513R0_poisoning_the_hash/test.cpp` | `242167bcf11d` | runs, adapted | STATIC_ASSERT(standard_hash_enabled<vector<bool>>()); (argument_type and result_type, deprecated in C++17 and removed in C++20; the rest of the check is kept) |
| `tests/std/tests/P0608R3_improved_variant_converting_constructor/test.cpp` | `396cb1f0662f` | runs |  |
| `tests/std/tests/P0896R4_ranges_range_machinery/test.cpp` | `7478f5a6fe72` | runs, adapted | static_assert(test_std_container<std::vector<bool>, std::random_access_iterator_tag>()); (const_reference is the bit vector's own proxy, so ranges::cbegin, cend, crbegin and crend wrap its iterators in basic_const_iterator; the container's other range accesses are kept) |
| `tests/std/tests/P1004R2_constexpr_vector_bool/test.cpp` | `5efb070d49bb` | runs |  |
| `tests/std/tests/P1206R7_vector_append_range/test.cpp` | `30d24c0a6778` | runs |  |
| `tests/std/tests/P1206R7_vector_assign_range/test.cpp` | `1f23ff2fc032` | runs |  |
| `tests/std/tests/P1206R7_vector_from_range/test.cpp` | `d009462ae64e` | runs |  |
| `tests/std/tests/P1206R7_vector_insert_range/test.cpp` | `07edc91415d6` | runs |  |
| `tests/std/tests/P1518R2_stop_overconstraining_allocators/test.compile.pass.cpp` | `730196c7f24c` | not run | no rewrite reaches it, so it tests the standard library |
| `tests/std/tests/P1614R2_spaceship/test.cpp` | `a07edb1d4840` | runs |  |
| `tests/std/tests/P2165R4_tuple_like_pair/test.cpp` | `368134487c67` | runs |  |
| `tests/std/tests/P2165R4_tuple_like_tuple_members/test.cpp` | `85226b2778e4` | runs |  |
| `tests/std/tests/P2278R4_ranges_const_iterator_machinery/test.compile.pass.cpp` | `d285f1178539` | compiles, adapted | static_assert(same_as<const_iterator<ConstIter>, ConstIter>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<const_sentinel<ConstIter>, ConstIter>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator) |
| `tests/std/tests/P2278R4_ranges_const_range_machinery/test.cpp` | `a6db4764729e` | runs, adapted | static_assert(same_as<ranges::const_iterator_t<Vb>, Vb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<ranges::const_sentinel_t<Vb>, Vb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<ranges::const_iterator_t<ConstVb>, ConstVb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<ranges::const_sentinel_t<ConstVb>, ConstVb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(ranges::constant_range<ConstVb>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator) |
| `tests/std/tests/P2286R8_text_formatting_formattable/test.compile.pass.cpp` | `8d9412e48940` | compiles |  |
| `tests/std/tests/P2286R8_text_formatting_header_vector/test.compile.pass.cpp` | `2ff1122b61e4` | compiles |  |
| `tests/std/tests/P2321R2_proxy_reference/test.cpp` | `9bc2bd6701e6` | runs |  |
| `tests/std/tests/P2502R2_generator_promise/test.cpp` | `d53b2a262a8b` | runs |  |
| `tests/std/tests/P3107R5_enabled_specializations/test.compile.pass.cpp` | `bb3888805987` | compiles |  |
| `tests/std/tests/VSO_0000000_instantiate_containers/test.compile.pass.cpp` | `690dfaa178c5` | compiles, adapted | std::erase(vb, true); (erase is xstd's, called qualified as the test calls std's); std::erase_if(value, pr1); (erase_if is xstd's, called qualified as the test calls std's, so that no argument-dependent lookup completes its predicate's type); std::erase_if(value, validating_falsity{}); (erase_if is xstd's, called qualified as the test calls std's, so that no argument-dependent lookup completes its predicate's type) |
| `tests/std/tests/VSO_0429900_fast_debug_range_based_for/test.cpp` | `c2ff8d8251a8` | runs |  |
| `tests/std/tests/VSO_0830211_container_debugging_range_checks/test.cpp` | `f37d5e2a0797` | runs |  |
| `tests/std/tests/VSO_2252142_wrong_C5046/test.compile.pass.cpp` | `adbb28235ae2` | compiles |  |

## MSVC STL against bit_small_vector

- **Pinned:** [https://github.com/microsoft/STL](https://github.com/microsoft/STL) at `vs-2022-17.14`.
- **Mappings:** the suite's `test/std-test-suite/msvc-stl-*.toml` for the directories it takes, and `bit-small-vector.toml` for the type.
- **Not selected:** 1657 files that do not match `vector<bool>`, left to the standard library.

| test | sha256 | outcome | note |
| :--- | :--- | :--- | :--- |
| `tests/std/tests/Dev10_563443_empty_vector_begin_plus_zero/test.cpp` | `aa8374c89c38` | runs |  |
| `tests/std/tests/Dev10_567556_move_from_empty_list/test.cpp` | `542aa9329fd1` | runs |  |
| `tests/std/tests/Dev10_689595_back_inserter_vector_bool/test.cpp` | `925fd499ca3c` | runs |  |
| `tests/std/tests/Dev10_709168_marking_iterators_as_checked/test.compile.pass.cpp` | `1b28f193ecf3` | compiles, adapted | STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked); STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::const_iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked); STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::reverse_iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked); STATIC_ASSERT(stl_checked == _Range_verifiable_v<vector<bool>::const_reverse_iterator>); (the MSVC STL's internal _Range_verifiable_v, true only for the iterators it marks as checked) |
| `tests/std/tests/Dev10_816787_swap_vector_bool_elements/test.cpp` | `a55840c127a8` | runs |  |
| `tests/std/tests/Dev11_0000000_null_forward_iterators/test.cpp` | `011e4c52769c` | runs |  |
| `tests/std/tests/Dev11_0000000_user_defined_literals/test.cpp` | `1147e8aa2c1c` | runs |  |
| `tests/std/tests/Dev11_0135139_vector_bool_comparisons/test.cpp` | `7e586b4f2a2c` | runs, adapted | x.back().flip(); (no reference::flip()); y.back().flip(); (no reference::flip()); vector<bool> v(47, allocator<bool>()); (the allocator is the storage's, Boost's small_vector allocator of blocks, not std::allocator<bool>) |
| `tests/std/tests/Dev11_0437519_container_requirements/test.compile.pass.cpp` | `324b9cdcd611` | compiles |  |
| `tests/std/tests/Dev11_0483851_vector_debug_allocator_use/test.compile.pass.cpp` | `28ec0ef3a3e2` | compiles |  |
| `tests/std/tests/GH_000625_vector_bool_optimization/test.cpp` | `f8a75dba8e34` | runs, adapted | static_assert(blockSize == _VBITS, "Invalid block size"); (the MSVC STL's internal _VBITS, the width of its words; the test's data crosses word boundaries at any width); input_true[length + 1].flip(); (no reference::flip()); input_false[length + 1].flip(); (no reference::flip()); input_true[0].flip(); (no reference::flip()); input_false[0].flip(); (no reference::flip()); void print_vec(const char* const name, const vector<T>& v) { (the test's diagnostic helper takes a std::vector<T>, which a bit vector is not, so it deduces the container instead) |
| `tests/std/tests/GH_001638_dllexport_derived_classes/test.compile.pass.cpp` | `8f5be5f6ee2b` | fails to compile | none of the bit vector's: MSVC rejects exporting a class whose base instantiates a static data member template, reported as rhalbersma/xstd-bits#529 |
| `tests/std/tests/GH_005090_stl_hardening/test.cpp` | `5ad23c13f246` | runs |  |
| `tests/std/tests/GH_005315_destructor_tombstones/test.cpp` | `60086bed0724` | runs, adapted | static_assert(test_constexpr()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P0009R18_mdspan_mdspan/test.cpp` | `7a9d4f5ed8c1` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P0088R3_variant/test.cpp` | `dc168121c680` | not run | a copy of a libc++ test, whose msvc_stdlib_force_include.h lives in the STL's LLVM submodule, which this adapter does not fetch |
| `tests/std/tests/P0433R2_deduction_guides/test.cpp` | `c5fde5817992` | runs, adapted | static_assert(is_same_v<decltype(vb1), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector); static_assert(is_same_v<decltype(vb3), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector); static_assert(is_same_v<decltype(vb5), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector); static_assert(is_same_v<decltype(vb8), vector<bool>>); (class template argument deduction names std::vector, which deduces std::vector<bool> where the rewrite names the bit vector) |
| `tests/std/tests/P0513R0_poisoning_the_hash/test.cpp` | `242167bcf11d` | runs, adapted | STATIC_ASSERT(standard_hash_enabled<vector<bool>>()); (argument_type and result_type, deprecated in C++17 and removed in C++20; the rest of the check is kept) |
| `tests/std/tests/P0608R3_improved_variant_converting_constructor/test.cpp` | `396cb1f0662f` | runs |  |
| `tests/std/tests/P0896R4_ranges_range_machinery/test.cpp` | `7478f5a6fe72` | runs, adapted | static_assert(test_std_container<std::vector<bool>, std::random_access_iterator_tag>()); (const_reference is the bit vector's own proxy, so ranges::cbegin, cend, crbegin and crend wrap its iterators in basic_const_iterator; the container's other range accesses are kept) |
| `tests/std/tests/P1004R2_constexpr_vector_bool/test.cpp` | `5efb070d49bb` | runs, adapted | static_assert(test_interface()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P1206R7_vector_append_range/test.cpp` | `30d24c0a6778` | runs, adapted | static_assert(test_copyable_view()); (Boost's small_vector is not usable in a constant expression); static_assert(test_move_only_view()); (Boost's small_vector is not usable in a constant expression); static_assert(test_c_array()); (Boost's small_vector is not usable in a constant expression); static_assert(test_lvalue_vector()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P1206R7_vector_assign_range/test.cpp` | `1f23ff2fc032` | runs, adapted | static_assert(test_copyable_views()); (Boost's small_vector is not usable in a constant expression); static_assert(test_move_only_views()); (Boost's small_vector is not usable in a constant expression); static_assert(test_c_array()); (Boost's small_vector is not usable in a constant expression); static_assert(test_lvalue_vector()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P1206R7_vector_from_range/test.cpp` | `d009462ae64e` | runs, adapted | static_assert(test_copyable_views()); (Boost's small_vector is not usable in a constant expression); static_assert(test_move_only_views()); (Boost's small_vector is not usable in a constant expression); static_assert(test_c_array()); (Boost's small_vector is not usable in a constant expression); static_assert(test_lvalue_vector()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P1206R7_vector_insert_range/test.cpp` | `07edc91415d6` | runs, adapted | static_assert(test_copyable_views()); (Boost's small_vector is not usable in a constant expression); static_assert(test_move_only_views()); (Boost's small_vector is not usable in a constant expression); static_assert(test_c_array()); (Boost's small_vector is not usable in a constant expression); static_assert(test_lvalue_vector()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P1518R2_stop_overconstraining_allocators/test.compile.pass.cpp` | `730196c7f24c` | not run | no rewrite reaches it, so it tests the standard library |
| `tests/std/tests/P1614R2_spaceship/test.cpp` | `a07edb1d4840` | runs, adapted | static_assert(test_vector()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P2165R4_tuple_like_pair/test.cpp` | `368134487c67` | runs, adapted | static_assert(test_pair_like_const_assignment<BinaryArray>()); (Boost's small_vector is not usable in a constant expression); static_assert(test_pair_like_const_assignment<BinaryTuple>()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P2165R4_tuple_like_tuple_members/test.cpp` | `85226b2778e4` | runs, adapted | static_assert(test_pair_like_const_assignment(Seq{})); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P2278R4_ranges_const_iterator_machinery/test.compile.pass.cpp` | `d285f1178539` | compiles, adapted | static_assert(same_as<const_iterator<ConstIter>, ConstIter>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<const_sentinel<ConstIter>, ConstIter>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator) |
| `tests/std/tests/P2278R4_ranges_const_range_machinery/test.cpp` | `a6db4764729e` | runs, adapted | static_assert(same_as<ranges::const_iterator_t<Vb>, Vb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<ranges::const_sentinel_t<Vb>, Vb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<ranges::const_iterator_t<ConstVb>, ConstVb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(same_as<ranges::const_sentinel_t<ConstVb>, ConstVb::const_iterator>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator); static_assert(ranges::constant_range<ConstVb>); (const_reference is the bit vector's own proxy, so const_iterator is no constant iterator) |
| `tests/std/tests/P2286R8_text_formatting_formattable/test.compile.pass.cpp` | `8d9412e48940` | compiles |  |
| `tests/std/tests/P2286R8_text_formatting_header_vector/test.compile.pass.cpp` | `2ff1122b61e4` | compiles |  |
| `tests/std/tests/P2321R2_proxy_reference/test.cpp` | `9bc2bd6701e6` | runs, adapted | static_assert(test()); (Boost's small_vector is not usable in a constant expression) |
| `tests/std/tests/P2502R2_generator_promise/test.cpp` | `d53b2a262a8b` | runs |  |
| `tests/std/tests/P3107R5_enabled_specializations/test.compile.pass.cpp` | `bb3888805987` | compiles |  |
| `tests/std/tests/VSO_0000000_instantiate_containers/test.compile.pass.cpp` | `690dfaa178c5` | compiles, adapted | std::erase(vb, true); (erase is xstd's, called qualified as the test calls std's); std::erase_if(value, pr1); (erase_if is xstd's, called qualified as the test calls std's, so that no argument-dependent lookup completes its predicate's type); std::erase_if(value, validating_falsity{}); (erase_if is xstd's, called qualified as the test calls std's, so that no argument-dependent lookup completes its predicate's type) |
| `tests/std/tests/VSO_0429900_fast_debug_range_based_for/test.cpp` | `c2ff8d8251a8` | runs |  |
| `tests/std/tests/VSO_0830211_container_debugging_range_checks/test.cpp` | `f37d5e2a0797` | runs |  |
| `tests/std/tests/VSO_2252142_wrong_C5046/test.compile.pass.cpp` | `adbb28235ae2` | compiles |  |
