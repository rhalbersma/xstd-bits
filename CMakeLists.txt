#          Copyright Rein Halbersma 2014-2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.30)
project(
    xstd-bits
    VERSION 0.1.0
    DESCRIPTION "Bitwise extensions to the C++ Standard Library"
    # Spelled out: project()'s own arguments expand before it sets PROJECT_NAME, so ${PROJECT_NAME} here is empty.
    HOMEPAGE_URL https://github.com/rhalbersma/xstd-bits
    LANGUAGES CXX
)

include(GNUInstallDirs)

# The project's own name, spelled once. TOUPPER alone leaves XSTD-BITS, which no cache option can be called; MAKE_C_IDENTIFIER alone leaves the lowercase form. Both, in this order, and test/ and benchmark/ inherit them.
string(MAKE_C_IDENTIFIER ${PROJECT_NAME} project_id)            # xstd_bits
string(TOUPPER ${project_id} project_option_prefix)             # XSTD_BITS

# Warnings are errors while this library is being developed, which is what the presets ask for. Declared here rather than under test/, as xstd and tabula do, because benchmark/ carries its own warning set and has to be gated by the same switch.
option(${project_option_prefix}_WARNINGS_AS_ERRORS "Treat warnings as errors in ${PROJECT_NAME} tests and benchmarks" ${PROJECT_IS_TOP_LEVEL})

# Prefer an installed xstd-ints so install(EXPORT) can record it as a dependency; a FetchContent build tree cannot be exported.
find_package(xstd-ints 0.1.0 CONFIG QUIET)
if(NOT xstd-ints_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        xstd-ints
        GIT_REPOSITORY https://github.com/rhalbersma/xstd-ints.git
        # Pinned to a commit on xstd-ints' main, not a branch head; move consumption.yml's dependency_repos with it.
        GIT_TAG 790a41c78867c3116b07fa85c6b1b886d961eaf1
    )
    FetchContent_MakeAvailable(xstd-ints)
endif()

# xstd-misc supplies conditional_data_member_t and XSTD_NO_UNIQUE_ADDRESS, which bit_block_container uses to carry a run-time width without paying for it under a compile-time one, and simple_allocator and container_compatible_range, which the owners constrain their constructors and deduction guides by, and hash_algorithm and hash_algorithm_is_avalanching, which bit_hash and bit_digest are built on.
find_package(xstd-misc 0.1.0 CONFIG QUIET)
if(NOT xstd-misc_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        xstd-misc
        GIT_REPOSITORY https://github.com/rhalbersma/xstd-misc.git
        # Pinned to a commit on xstd-misc's main, not a branch head; move consumption.yml's dependency_repos with it.
        GIT_TAG 6473bbf60186bdc72091f6556ba613ba7a99b3de
    )
    FetchContent_MakeAvailable(xstd-misc)
endif()

# Boost.Hash2 is a public dependency of this library's headers, not just of the tests, so it is declared explicitly.
find_package(boost_hash2 CONFIG REQUIRED)

# Boost.Container likewise: the ext/ column's small_vector, opt-in to include, and the inplace column's blocks where the standard library has no std::inplace_vector.
find_package(boost_container CONFIG REQUIRED)

add_library(
    ${PROJECT_NAME} INTERFACE
)

# Consumed as xstd::bits, following Boost: the package keeps the repository's name (find_package(xstd-bits)) while the target is a component of one umbrella namespace, so xstd::bits sits beside xstd::ints the way Boost::hash2 sits beside Boost::mp11. EXPORT_NAME is what lets the exported name differ from the target name; the alias below must spell the same thing, or an add_subdirectory consumer and a find_package consumer would end up with two different names for one library.
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME bits)
add_library(xstd::bits ALIAS ${PROJECT_NAME})

# No target_include_directories: the HEADERS file set supplies both the build and the install interface.
target_sources(
    ${PROJECT_NAME} INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/xstd/bits.hpp
        include/xstd/bits/detail/sequence_adaptor.hpp
        include/xstd/bits/detail/sequence_erasure.hpp
        include/xstd/bits/bit_set/detail/set_adaptor.hpp
        include/xstd/bits/bit_set/detail/erase_if.hpp
        include/xstd/bits/bit_valarray/detail/valarray_adaptor.hpp
        include/xstd/bits/bit_valarray/detail/valarray_proxy.hpp
        include/xstd/bits/bit_valarray/detail/gslice_index.hpp
        include/xstd/bits/bit_valarray/detail/block_compress_expand.hpp
        include/xstd/bits/detail/borrowed_bits.hpp
        include/xstd/bits/bit_algorithm.hpp
        include/xstd/bits/bit_algorithm/detail/block_includes.hpp
        include/xstd/bits/bit_algorithm/detail/blit.hpp
        include/xstd/bits/bit_algorithm/detail/comparable.hpp
        include/xstd/bits/bit_algorithm/detail/compare.hpp
        include/xstd/bits/bit_algorithm/detail/count.hpp
        include/xstd/bits/bit_algorithm/detail/find.hpp
        include/xstd/bits/bit_algorithm/detail/mismatch.hpp
        include/xstd/bits/bit_algorithm/detail/sequence.hpp
        include/xstd/bits/bit_algorithm/detail/walk.hpp
        include/xstd/bits/bit_algorithm/bit_all_of.hpp
        include/xstd/bits/bit_algorithm/bit_any_of.hpp
        include/xstd/bits/bit_algorithm/bit_copy.hpp
        include/xstd/bits/bit_algorithm/bit_copy_backward.hpp
        include/xstd/bits/bit_algorithm/bit_count.hpp
        include/xstd/bits/bit_algorithm/bit_disjoint.hpp
        include/xstd/bits/bit_algorithm/bit_equal.hpp
        include/xstd/bits/bit_algorithm/bit_fill.hpp
        include/xstd/bits/bit_algorithm/bit_find.hpp
        include/xstd/bits/bit_algorithm/bit_find_last.hpp
        include/xstd/bits/bit_algorithm/bit_for_each.hpp
        include/xstd/bits/bit_algorithm/bit_includes.hpp
        include/xstd/bits/bit_algorithm/bit_intersects.hpp
        include/xstd/bits/bit_algorithm/bit_is_partitioned.hpp
        include/xstd/bits/bit_algorithm/bit_is_sorted.hpp
        include/xstd/bits/bit_algorithm/bit_lexicographical_compare_three_way.hpp
        include/xstd/bits/bit_algorithm/bit_mismatch.hpp
        include/xstd/bits/bit_algorithm/bit_move.hpp
        include/xstd/bits/bit_algorithm/bit_none_of.hpp
        include/xstd/bits/bit_algorithm/bit_partition.hpp
        include/xstd/bits/bit_algorithm/bit_partition_point.hpp
        include/xstd/bits/bit_algorithm/bit_reverse.hpp
        include/xstd/bits/bit_algorithm/bit_reverse_copy.hpp
        include/xstd/bits/bit_algorithm/bit_rotate.hpp
        include/xstd/bits/bit_algorithm/bit_rotate_copy.hpp
        include/xstd/bits/bit_algorithm/bit_set_difference.hpp
        include/xstd/bits/bit_algorithm/bit_set_intersection.hpp
        include/xstd/bits/bit_algorithm/bit_set_symmetric_difference.hpp
        include/xstd/bits/bit_algorithm/bit_set_union.hpp
        include/xstd/bits/bit_algorithm/bit_shift_left.hpp
        include/xstd/bits/bit_algorithm/bit_shift_right.hpp
        include/xstd/bits/bit_algorithm/bit_sort.hpp
        include/xstd/bits/bit_algorithm/bit_swap_ranges.hpp
        include/xstd/bits/bit_algorithm/bit_transform.hpp
        include/xstd/bits/bit_array.hpp
        include/xstd/bits/bit_array/bit_array.hpp
        include/xstd/bits/bit_array/to_array.hpp
        include/xstd/bits/bit_array/tuple_element.hpp
        include/xstd/bits/bit_array/hash.hpp
        include/xstd/bits/bit_array/container_hash.hpp
        include/xstd/bits/bit.hpp
        include/xstd/bits/bit/bit_convert.hpp
        include/xstd/bits/bit_concepts.hpp
        include/xstd/bits/bit_concepts/bit_blocks.hpp
        include/xstd/bits/bit_concepts/bit_constructible_from.hpp
        include/xstd/bits/bit_concepts/bit_convertible_to.hpp
        include/xstd/bits/bit_concepts/bit_index_mapping.hpp
        include/xstd/bits/bit_concepts/bit_mask_mapping.hpp
        include/xstd/bits/bit_concepts/sized_bit_index_mapping.hpp
        include/xstd/bits/bit_inplace_vector.hpp
        include/xstd/bits/bit_inplace_vector/bit_inplace_vector.hpp
        include/xstd/bits/bit_inplace_vector/erase_if.hpp
        include/xstd/bits/bit_inplace_vector/hash.hpp
        include/xstd/bits/bit_inplace_vector/container_hash.hpp
        include/xstd/bits/bit_functional.hpp
        include/xstd/bits/bit_functional/bit_compare_three_way.hpp
        include/xstd/bits/bit_functional/bit_digest.hpp
        include/xstd/bits/bit_functional/bit_equal_to.hpp
        include/xstd/bits/bit_functional/bit_greater.hpp
        include/xstd/bits/bit_functional/bit_greater_equal.hpp
        include/xstd/bits/bit_functional/bit_hash.hpp
        include/xstd/bits/bit_functional/bit_less.hpp
        include/xstd/bits/bit_functional/bit_less_equal.hpp
        include/xstd/bits/bit_functional/bit_not_equal_to.hpp
        include/xstd/bits/bit_functional/detail/bit_hasher.hpp
        include/xstd/bits/bit_iterator.hpp
        include/xstd/bits/bit_iterator/bit_advance.hpp
        include/xstd/bits/bit_iterator/bit_distance.hpp
        include/xstd/bits/bit_iterator/bit_next.hpp
        include/xstd/bits/bit_iterator/bit_prev.hpp
        include/xstd/bits/bit_iterator/detail/concepts.hpp
        include/xstd/bits/bit_iterator/detail/select.hpp
        include/xstd/bits/bit_ranges.hpp
        include/xstd/bits/bit_ranges/from_blocks.hpp
        include/xstd/bits/bit_set.hpp
        include/xstd/bits/bit_set/bit_inplace_set.hpp
        include/xstd/bits/bit_set/bit_enum_set.hpp
        include/xstd/bits/bit_set/bit_flag_set.hpp
        include/xstd/bits/bit_set/bit_flag_mapping.hpp
        include/xstd/bits/bit_set/bit_key_mapping.hpp
        include/xstd/bits/bit_set/bit_set.hpp
        include/xstd/bits/bit_set/bit_set_view.hpp
        include/xstd/bits/bit_set/bit_fixed_set.hpp
        include/xstd/bits/bit_set/erase_if.hpp
        include/xstd/bits/bit_set/hash.hpp
        include/xstd/bits/bit_set/container_hash.hpp
        include/xstd/bits/bit_span.hpp
        include/xstd/bits/bit_span/bit_span.hpp
        include/xstd/bits/bit_span/bit_subspan.hpp
        include/xstd/bits/bit_type_traits.hpp
        include/xstd/bits/bit_type_traits/bit_align.hpp
        include/xstd/bits/bit_type_traits/bit_blocks_extent.hpp
        include/xstd/bits/bit_type_traits/bit_fast.hpp
        include/xstd/bits/bit_type_traits/bit_least.hpp
        include/xstd/bits/bit_type_traits/bit_rebind.hpp
        include/xstd/bits/bit_type_traits/bit_resize.hpp
        include/xstd/bits/bit_type_traits/bit_underlying.hpp
        include/xstd/bits/bit_valarray.hpp
        include/xstd/bits/bit_valarray/bit_valarray.hpp
        include/xstd/bits/bit_valarray/bit_fixed_valarray.hpp
        include/xstd/bits/bit_valarray/bit_inplace_valarray.hpp
        include/xstd/bits/bit_valarray/bit_slice.hpp
        include/xstd/bits/bit_valarray/bit_gslice.hpp
        include/xstd/bits/bit_valarray/bit_slice_array.hpp
        include/xstd/bits/bit_valarray/bit_gslice_array.hpp
        include/xstd/bits/bit_valarray/bit_mask_array.hpp
        include/xstd/bits/bit_valarray/bit_indirect_array.hpp
        include/xstd/bits/bit_vector.hpp
        include/xstd/bits/bit_vector/bit_vector.hpp
        include/xstd/bits/bit_vector/erase_if.hpp
        include/xstd/bits/bit_vector/hash.hpp
        include/xstd/bits/bit_vector/container_hash.hpp
        include/xstd/bits/detail/adapted_bits.hpp
        include/xstd/bits/detail/allocator_base_type.hpp
        include/xstd/bits/detail/assert.hpp
        include/xstd/bits/detail/inplace_blocks.hpp
        include/xstd/bits/detail/bit_block_container.hpp
        include/xstd/bits/detail/block_intersects.hpp
        include/xstd/bits/detail/comparisons.hpp
        include/xstd/bits/detail/borrowed_block_span.hpp
        include/xstd/bits/detail/hash.hpp
        include/xstd/bits/detail/intrinsic.hpp
        include/xstd/bits/bit_set/detail/flag_block.hpp
        include/xstd/bits/detail/ordinal.hpp
        include/xstd/bits/bit_set/detail/is_key.hpp
        include/xstd/bits/ext/boost.hpp
        include/xstd/bits/ext/boost/bit_small_set.hpp
        include/xstd/bits/ext/boost/bit_small_vector.hpp
        include/xstd/bits/ext/boost/bit_small_valarray.hpp
        include/xstd/bits/ext/boost/dynamic_bitset.hpp
        include/xstd/bits/ext/boost/detail/dynamic_bitset.hpp
        include/xstd/bits/detail/bit_layout.hpp
        include/xstd/bits/bit/detail/bit_convertible.hpp
        include/xstd/bits/detail/bit_blocks_capacity.hpp
        include/xstd/bits/detail/bit_block_range.hpp
        include/xstd/bits/detail/owned_bit_blocks.hpp
        include/xstd/bits/detail/resizable_bit_blocks.hpp
        include/xstd/bits/bit/detail/bit_width.hpp
        include/xstd/bits/detail/range_const_reference.hpp
        include/xstd/bits/detail/shift.hpp
        include/xstd/bits/detail/storage_ptr.hpp
        include/xstd/bits/detail/views.hpp
        include/xstd/bits/detail/blocks.hpp
        include/xstd/bits/bit_type_traits/detail/blocks_extent.hpp
        include/xstd/bits/detail/non_volatile.hpp
        include/xstd/bits/detail/num_blocks.hpp
        include/xstd/bits/detail/owner_members.hpp
        include/xstd/bits/detail/key_order.hpp
        include/xstd/bits/detail/ownership.hpp
        include/xstd/bits/detail/rebind.hpp
)

# The list above is hand-written, and it drifted: the three inplace headers reached include/ and bits.hpp but never this file set, so every installed consumer's <xstd/bits.hpp> named three headers that were not installed. No compiler leg could see it -- they all build from the source tree -- and the consumer translation unit that does build against the install tree had included one container header until it was made the interface gate. Checked here from now on, the way test/ checks that every public header is mirrored. Every header under include/, not just include/xstd/: include/ is exactly what a consumer gets, and the workloads that used to sit beside it now live in examples/. A stray directory here would otherwise ride the file set's BASE_DIRS onto every add_subdirectory consumer's include path while never being installed. HEADER_SET answers in absolute paths, so the glob asks for them that way and the message relativizes back.
get_target_property(declared_headers ${PROJECT_NAME} HEADER_SET)
file(
    GLOB_RECURSE headers_on_disk
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

set(undeclared_headers)
foreach(h ${headers_on_disk})
    if(NOT h IN_LIST declared_headers)
        file(RELATIVE_PATH undeclared ${CMAKE_CURRENT_SOURCE_DIR} ${h})
        list(APPEND undeclared_headers ${undeclared})
    endif()
endforeach()

if(undeclared_headers)
    list(SORT undeclared_headers)
    string(REPLACE ";" "\n  " undeclared_header_lines "${undeclared_headers}")
    message(FATAL_ERROR
        "These headers exist under include/ but are not in the HEADERS file set above, so they would not "
        "be installed:\n  ${undeclared_header_lines}\nAdd them there, or delete them."
    )
endif()

target_link_libraries(
    ${PROJECT_NAME} INTERFACE
    xstd::ints
    xstd::misc
    Boost::container
    Boost::hash2
)

target_compile_features(
    ${PROJECT_NAME} INTERFACE
    cxx_std_23
)

# Tests and benchmarks exist only when xstd-bits itself is being developed; consumers get the header-only target.
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if(BUILD_TESTING)
        # The library requires C++23 and says so above, in the INTERFACE a consumer inherits. The inplace column is held in std::inplace_vector only behind __cpp_lib_inplace_vector, which is C++26, and in boost::container::static_vector below it, so at the minimum that storage and the constant evaluation of the three inplace cells are never tested. Raising the standard here rather than in the INTERFACE keeps the requirement a consumer sees at 23 while letting a leg of the matrix test both storages.
        set(${project_option_prefix}_CXX_STANDARD 23 CACHE STRING "C++ standard for the tests and benchmarks: 23, or 26 to hold the inplace column in std::inplace_vector")
        set_property(CACHE ${project_option_prefix}_CXX_STANDARD PROPERTY STRINGS 23 26)

        # Asking for a standard CMake cannot spell for this compiler fails deep inside a try_compile, with a message blaming the compiler. CMAKE_CXX_COMPILE_FEATURES is what CMake actually knows, so ask it.
        if(NOT "cxx_std_${${project_option_prefix}_CXX_STANDARD}" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
            message(FATAL_ERROR
                "${project_option_prefix}_CXX_STANDARD=${${project_option_prefix}_CXX_STANDARD} is not something CMake ${CMAKE_VERSION} can ask "
                "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} for. CMake 3.30 is the first that knows C++26 "
                "for GCC and Clang, and this project requires it; a front end CMake has not been taught still answers "
                "no. The inplace column's std::inplace_vector storage needs that answer, nothing else does."
            )
        endif()

        set(CMAKE_CXX_STANDARD ${${project_option_prefix}_CXX_STANDARD})
        set(CMAKE_CXX_STANDARD_REQUIRED ON)

        # Strict, as xstd and tabula compile, except where strict mode would drop the widest Block: <bit>'s functions take std::unsigned_integral alone, and libstdc++ before 16 carries unsigned __int128 into that concept only outside __STRICT_ANSI__, where libc++ and libstdc++ 16 always do. The question is asked of the toolchain rather than of a version number, so a clang on libstdc++ 15 gets the answer that standard library gives, and MSVC and 32-bit targets, which have no unsigned __int128 to lose, stay strict.
        set(CMAKE_CXX_EXTENSIONS OFF)
        include(CheckCXXSourceCompiles)
        check_cxx_source_compiles([[
            #include <concepts>
            #if !defined(__SIZEOF_INT128__) || defined(_MSC_VER)
            #error "no unsigned __int128 to lose"
            #endif
            static_assert(not std::unsigned_integral<unsigned __int128>);
            auto main() -> int { return 0; }
        ]] ${project_option_prefix}_STRICT_DROPS_UINT128)
        if(${project_option_prefix}_STRICT_DROPS_UINT128)
            set(CMAKE_CXX_EXTENSIONS ON)
            message(STATUS "Tests and benchmarks compile as gnu++${CMAKE_CXX_STANDARD}: strict mode drops unsigned __int128 from std::unsigned_integral here")
        else()
            message(STATUS "Tests and benchmarks compile as c++${CMAKE_CXX_STANDARD}")
        endif()

        # Above both, and not inside test/: benchmark/ is configured first, so a target declared in test/ does not exist yet when the benchmarks ask for it. Both trees grade over the same two integer classes.
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/int128.cmake)

        # clang-cl reports CXX_COMPILER_ID Clang but understands only MSVC-style driver flags. Probed here, once for both trees that choose their warning flags by it, and inside this block, so an add_subdirectory consumer inherits nothing from it.
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            set(${project_id}_is_clang_cl ON)
        else()
            set(${project_id}_is_clang_cl OFF)
        endif()

        # Off by default, and the GCC ladder alone turns it on. A benchmark measures the library rather than being part of it, so seven compiler ladders, fifteen sanitizer legs and both analysers building one bought nothing the one ladder does not: its Release legs run the smoke test benchmark/CMakeLists.txt registers, which is what keeps this tree compiling and running against the current API. A local build that wants the benchmarks asks for them. The coverage job needs it off for a second reason, which its own stub records: gcovr drops benchmark/ as a source path, but a benchmark is not a source there -- it is a translation unit that INSTANTIATES the library's templates, and those instantiations live in include/ where they are counted. That job configures Debug, where no benchmark registers as a test, so they would compile and never run, and every branch of an instantiation no test TU also makes is uncovered by construction. One Block-width benchmark put nine such branches in the denominator and covered none.
        option(${project_option_prefix}_BUILD_BENCHMARKS "Build the benchmark tree alongside the tests" OFF)

        if(${project_option_prefix}_BUILD_BENCHMARKS)
            add_subdirectory(benchmark)
        endif()

        # On by default, and the coverage job alone turns it off. The hardened test builds the library with NDEBUG, so its instantiations differ from every Debug one by the asserts compiled out, and gcovr keys their branches apart from the Debug run's: one such translation unit put 31 branches in the denominator and covered none. What it checks, an abort where a precondition breaks, sits on the assert lines that job excludes anyway.
        option(${project_option_prefix}_BUILD_HARDENED_TESTS "Build the test that breaks each precondition a hardened release checks" ON)

        add_subdirectory(test)

        # Off by default, and one libc++ leg turns it on. The standard libraries' own test suites, written against the paragraphs by people who never saw this library, rewritten onto its spelling: a check whose authors have no stake in the answer. Needs git and Python 3.11.
        option(${project_option_prefix}_BUILD_STD_TEST_SUITES "Run the standard libraries' test suites against the containers they mirror" OFF)

        if(${project_option_prefix}_BUILD_STD_TEST_SUITES)
            add_subdirectory(test/std-test-suite)
        endif()
    endif()

    # Off by default, and outside BUILD_TESTING: ClusterFuzzLite builds the fuzz targets alone, without the test tree's Boost.Test and range-v3. Clang only, libFuzzer being clang's.
    option(${project_option_prefix}_BUILD_FUZZERS "Build the libFuzzer differential targets under fuzz/" OFF)

    if(${project_option_prefix}_BUILD_FUZZERS)
        add_subdirectory(fuzz)
    endif()
endif()

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    FILE_SET HEADERS
)
install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE xstd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Package config and version files, ARCH_INDEPENDENT because a header-only library has no architecture-specific binaries.
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    COMPATIBILITY SameMinorVersion
    ARCH_INDEPENDENT
)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
