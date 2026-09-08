#          Copyright Rein Halbersma 2014-2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG REQUIRED)
find_package(benchmark REQUIRED)
find_package(fmt REQUIRED)

# clang-cl reports CXX_COMPILER_ID Clang but understands only MSVC-style driver flags.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(${project_id}_is_clang_cl ON)
else()
    set(${project_id}_is_clang_cl OFF)
endif()

set(cxx_compile_definitions
    BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
    BOOST_ALL_NO_LIB
    BOOST_TEST_MAIN
    $<$<CXX_COMPILER_ID:MSVC>:
        _SCL_SECURE_NO_WARNINGS
    >
)

set(cxx_compile_options_warnings
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /permissive-
    >
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<NOT:$<BOOL:${${project_id}_is_clang_cl}>>>:
        -pedantic-errors
    >
    $<$<CXX_COMPILER_ID:Clang>:
        -Weverything
        -Wno-unknown-warning-option         # forward compatibility for new Clang warnings
        -Wno-lifetime-safety-intra-tu-suggestions             # Clang trunk (23-SVN); suggests [[clang::lifetimebound]] across xstd::proxy::bidirectional and xstd::bitset - not yet acted on
        -Wno-lifetime-safety-intra-tu-constructor-suggestions # ditto, for constructors
        -Wno-lifetime-safety-invalidation                     # ditto; false positive on a.clear() in test/include/set/primitives.hpp treating the container itself as invalidated
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-c++20-compat
        -Wno-c++20-compat-pedantic
        -Wno-nrvo
        -Wno-unsafe-buffer-usage
        -Wno-disabled-macro-expansion       # triggered by Boost.Test
        -Wno-global-constructors            # triggered by Boost.Test
        -Wno-used-but-marked-unused         # triggered by Boost.Test
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
    >
    $<$<AND:$<BOOL:${${project_option_prefix}_WARNINGS_AS_ERRORS}>,$<CXX_COMPILER_ID:MSVC>>:/WX>
    $<$<AND:$<BOOL:${${project_option_prefix}_WARNINGS_AS_ERRORS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>>:-Werror>
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

get_property(${project_id}_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Load-bearing in every configuration: without it two of three MinGW rungs cannot assemble these objects at all (#47).
set(cxx_compile_options_mbig_obj
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<PLATFORM_ID:Windows>>:
	    -Wa,-mbig-obj
    >
)

set(current_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(current_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE targets RELATIVE ${current_src_dir} *.cpp)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME target_source_dir)
foreach(t ${targets})
    cmake_path(GET t PARENT_PATH target_parent_path)
    cmake_path(GET t STEM        target_stem       )
    string(REPLACE "/" "." target_id ${target_source_dir}/${target_parent_path}/${target_stem})
    string(REGEX REPLACE "^[.]" "" target_id ${target_id})

    add_executable(${target_id} src/${t})

    target_link_libraries(
        ${target_id} PRIVATE
        xstd::bits
        Boost::headers
        benchmark::benchmark
        fmt::fmt
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

    # Under ctest a benchmark is a smoke test: that it runs, not what it costs. Timing the full ladder here would
    # put minutes of std::set at 2^20 into every CI run, and the number would be worthless on a shared runner
    # anyway. A manual run takes the defaults and measures for real.
    set(bench_smoke_args --benchmark_min_time=1x)

    # Release only, and by CONFIGURATIONS rather than if(): neither spelling alone works on both generator kinds (#32).
    if(${project_id}_is_multi_config)
        add_test(NAME ${target_id} CONFIGURATIONS Release COMMAND ${target_id} ${bench_smoke_args})
    elseif(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_test(NAME ${target_id} COMMAND ${target_id} ${bench_smoke_args})
    endif()
endforeach()
