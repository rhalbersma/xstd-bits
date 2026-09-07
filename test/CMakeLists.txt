#          Copyright Rein Halbersma 2014-2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

set(Boost_USE_STATIC_LIBS ON)
find_package(
    Boost CONFIG REQUIRED COMPONENTS
    unit_test_framework
)
find_package(fmt REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

# clang-cl reports CXX_COMPILER_ID Clang but understands only MSVC-style driver flags.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(xstd_bits_is_clang_cl ON)
else()
    set(xstd_bits_is_clang_cl OFF)
endif()

set(cxx_compile_definitions
    BOOST_ALL_NO_LIB
    BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
    BOOST_TEST_MAIN
    $<$<CXX_COMPILER_ID:MSVC>:
        _SCL_SECURE_NO_WARNINGS
    >
)

set(cxx_compile_options_warnings
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /permissive-
        /bigobj
    >
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<NOT:$<BOOL:${xstd_bits_is_clang_cl}>>>:
        -pedantic-errors
    >
    $<$<CXX_COMPILER_ID:Clang>:
        -Weverything
        -Wno-unknown-warning-option         # forward compatibility for new Clang warnings
        -Wno-lifetime-safety-intra-tu-suggestions             # Clang trunk (23-SVN); suggests [[clang::lifetimebound]] across xstd::proxy::bidirectional and xstd::bitset - not yet acted on
        -Wno-lifetime-safety-intra-tu-constructor-suggestions # ditto, for constructors
        -Wno-lifetime-safety-invalidation                     # ditto; false positive on a.clear() in test/include/test/set/primitives.hpp treating the container itself as invalidated
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-c++20-compat
        -Wno-c++20-compat-pedantic
        -Wno-nrvo
        -Wno-unsafe-buffer-usage
        -Wno-disabled-macro-expansion       # triggered by Boost.Test
        -Wno-global-constructors            # triggered by Boost.Test
        -Wno-used-but-marked-unused         # triggered by Boost.Test
        -Wno-c2y-extensions                 # triggered by Boost.Test's use of __COUNTER__
        -Wno-ctad-maybe-unsupported         # std::reverse_iterator(it) uses the standard's own deduction guide
        -Wno-zero-as-null-pointer-constant  # triggered by <=> comparisons against 0
    >
    $<$<CXX_COMPILER_ID:GNU>:
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion                        # NOTE: possibly triggered by range-v3 views::zip overload resolution
        -Wnrvo
        -Wshadow
        -Wsign-compare
        -Wsign-conversion
        -Wsign-promo
    >
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Release>>:
        -Wno-array-bounds                   # triggered by Boost.DynamicBitset for gcc >= 12 in Release mode
        -Wno-stringop-overflow              # triggered by Boost.DynamicBitset for gcc >= 12 in Release mode
        -Wno-maybe-uninitialized            # false positive on detail::array's std::array member (WinLibs gcc 15.3.0, Release mode)
        -Wno-restrict                       # false positive on std::vector's copy inside block_sequence's, inlined into basic_bitset's non-member operators (gcc 17-SVN, Release mode)
        -Wno-stringop-overread              # the same copy inlined into basic_bit_set's non-member operators, under the name gcc 17-SVN gives it there (Release mode)
    >
    $<$<AND:$<BOOL:${XSTD_BITS_WARNINGS_AS_ERRORS}>,$<CXX_COMPILER_ID:MSVC>>:/WX>
    $<$<AND:$<BOOL:${XSTD_BITS_WARNINGS_AS_ERRORS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>>:-Werror>
)

set(cxx_compile_options_optimization
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -march=native
    >
    # MinGW emits 32-byte stores to a 16-byte aligned stack; the assembler rewrites them unaligned (#54, GCC PR target/54412).
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<PLATFORM_ID:Windows>>:
        -Wa$<COMMA>-muse-unaligned-vector-move
    >
)

# Load-bearing in every configuration: without it two of three MinGW rungs cannot assemble these objects at all (#47).
set(cxx_compile_options_mbig_obj
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<PLATFORM_ID:Windows>>:
	    -Wa,-mbig-obj
    >
)

# Each public header compiled as its own translation unit, proving it self-sufficient; this is what Clang-Tidy analyzes.
set(public_header_dir ${PROJECT_SOURCE_DIR}/include)
file(
    GLOB_RECURSE public_headers
    CONFIGURE_DEPENDS
    RELATIVE ${public_header_dir}
    ${public_header_dir}/xstd/*.hpp
)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/header_self_sufficiency)

foreach(h ${public_headers})
    string(REPLACE "/" "." header_test_id ${h})
    string(REGEX REPLACE "[.]hpp$" "" header_test_id ${header_test_id})
    set(header_test_source ${CMAKE_CURRENT_BINARY_DIR}/header_self_sufficiency/${header_test_id}.cpp)
    # An IWYU pragma because clang-tidy runs misc-include-cleaner over these
    # generated sources too, and the one include is by construction unused; a
    # trailing return type because modernize-use-trailing-return-type flags
    # "int main()".
    file(
        WRITE ${header_test_source}
        "#include <${h}> // IWYU pragma: keep\nauto main() -> int {}\n"
    )

    add_executable(header.${header_test_id} ${header_test_source})

    # Boost::headers because the ext/ self-sufficiency units adapt a header-only Boost library; the target carries only hash2.
    target_link_libraries(
        header.${header_test_id} PRIVATE
        ${CMAKE_PROJECT_NAME}
        Boost::headers
    )

    target_compile_definitions(
        header.${header_test_id} PRIVATE
        ${cxx_compile_definitions}
    )

    target_compile_options(
        header.${header_test_id} PRIVATE
        ${cxx_compile_options_warnings}
        ${cxx_compile_options_optimization}
        ${cxx_compile_options_mbig_obj}
    )

    add_test(NAME header.${header_test_id} COMMAND header.${header_test_id})
endforeach()

# Every public header is answered by one test source at the same path, detail/ excepted; enforced here so it cannot drift.
file(
    GLOB_RECURSE mirrored_headers
    CONFIGURE_DEPENDS
    RELATIVE ${public_header_dir}
    ${public_header_dir}/xstd/*.hpp
)
list(FILTER mirrored_headers EXCLUDE REGEX "/detail/")

set(unmirrored_headers)
foreach(h ${mirrored_headers})
    # xstd/bits/bit_set_view.hpp is answered by test/src/bits/bit_set_view.cpp.
    string(REGEX REPLACE "^xstd/" "" mirrored ${h})
    string(REGEX REPLACE "[.]hpp$" ".cpp" mirrored ${mirrored})
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/${mirrored})
        list(APPEND unmirrored_headers "    include/${h}\n      -> expected test/src/${mirrored}")
    endif()
endforeach()

if(unmirrored_headers)
    list(JOIN unmirrored_headers "\n" unmirrored_report)
    message(
        FATAL_ERROR
        "These headers have no mirrored test source:\n${unmirrored_report}\n"
        "Every public header is mirrored by exactly one test source; add it, "
        "or the mirror is no longer a rule."
    )
endif()

set(current_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(current_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Anchored to current_src_dir: an unanchored *.cpp resolves against test/ and swept up consumer/main.cpp.
file(GLOB_RECURSE targets RELATIVE ${current_src_dir} ${current_src_dir}/*.cpp)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME target_source_dir)
foreach(t ${targets})
    cmake_path(GET t PARENT_PATH target_parent_path)
    cmake_path(GET t STEM        target_stem       )
    # A source directly under src/ has an empty parent path, which used to leave test/src/bits.cpp as test..bits.
    string(REPLACE "/" "." target_id ${target_source_dir}/${target_parent_path}/${target_stem})
    string(REGEX REPLACE "[.]+" "." target_id ${target_id})
    string(REGEX REPLACE "^[.]" "" target_id ${target_id})

    add_executable(${target_id} src/${t})

    target_link_libraries(
        ${target_id} PRIVATE
        ${CMAKE_PROJECT_NAME}
        Boost::headers
        Boost::unit_test_framework
        fmt::fmt
        range-v3::range-v3
    )

    target_include_directories(
        ${target_id} PRIVATE
        ${current_include_dir}
    )

    target_compile_definitions(
        ${target_id} PRIVATE
        ${cxx_compile_definitions}
    )

    target_compile_options(
        ${target_id} PRIVATE
        ${cxx_compile_options_warnings}
        ${cxx_compile_options_optimization}
        ${cxx_compile_options_mbig_obj}
    )

    add_test(${target_id} ${target_id})
endforeach()
