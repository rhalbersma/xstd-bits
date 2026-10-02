#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# libFuzzer ships with clang and nothing else; clang-cl drives it through other flags, so it is refused as well.
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    message(FATAL_ERROR "${project_option_prefix}_BUILD_FUZZERS needs clang with its GNU-style driver, which is what links libFuzzer; this is ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.")
endif()

# The bitset target's model; the library itself never names Boost.DynamicBitset.
find_package(boost_dynamic_bitset CONFIG REQUIRED)

# The defaults are a self-contained local build. ClusterFuzzLite clears the first, its CXXFLAGS carrying the sanitizer and the instrumentation, and sets the second to its LIB_FUZZING_ENGINE.
set(${project_option_prefix}_FUZZ_COMPILE_OPTIONS "-fsanitize=fuzzer,address,undefined;-fno-sanitize-recover=undefined;-O1;-g" CACHE STRING "Compile options for the fuzz targets")
set(${project_option_prefix}_FUZZ_LINK_OPTIONS "-fsanitize=fuzzer,address,undefined" CACHE STRING "Link options for the fuzz targets, which bring in the fuzzing engine")

foreach(target_id fuzz_bitset fuzz_set fuzz_vector)
    add_executable(${target_id} ${target_id}.cpp)

    target_include_directories(
        ${target_id} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(
        ${target_id} PRIVATE
        xstd::bits
        Boost::dynamic_bitset
    )

    target_compile_definitions(
        ${target_id} PRIVATE
        BOOST_ALL_NO_LIB
    )

    target_compile_options(
        ${target_id} PRIVATE
        -Wall
        -Wextra
        ${${project_option_prefix}_FUZZ_COMPILE_OPTIONS}
    )

    target_link_options(
        ${target_id} PRIVATE
        ${${project_option_prefix}_FUZZ_LINK_OPTIONS}
    )

    # The seed corpus replayed once, without mutation: a regression check that every seed still runs clean.
    if(BUILD_TESTING)
        add_test(NAME fuzz.${target_id} COMMAND ${target_id} -runs=0 ${CMAKE_CURRENT_SOURCE_DIR}/corpus/${target_id})
    endif()
endforeach()
