#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# rhalbersma/cpp-ci's adapters, at the commit the workflows pin, read from one of them so the two cannot drift apart.
file(STRINGS ${PROJECT_SOURCE_DIR}/.github/workflows/clang-libc++.yml cpp_ci_line REGEX "rhalbersma/cpp-ci/[.]github/workflows/clang-libc[+][+][.]yml@[0-9a-f]+" LIMIT_COUNT 1)
string(REGEX MATCH "@([0-9a-f]+)" cpp_ci_match "${cpp_ci_line}")
set(cpp_ci_pin "${CMAKE_MATCH_1}")
if(NOT cpp_ci_pin)
    message(FATAL_ERROR "No rhalbersma/cpp-ci pin found in .github/workflows/clang-libc++.yml")
endif()

include(FetchContent)
FetchContent_Declare(
    cpp-ci
    GIT_REPOSITORY https://github.com/rhalbersma/cpp-ci.git
    GIT_TAG ${cpp_ci_pin}
)
FetchContent_MakeAvailable(cpp-ci)
include(${cpp-ci_SOURCE_DIR}/cmake/StdTestSuite.cmake)

# One suite's tests against one subject, from the suite's mappings of the directories it takes and the subject's mapping
# of the type it reads them against, and a test that doc/vendor.md records what became of each of them in this build.
# The mappings are written against the stable rung's suites. The development rung runs GCC trunk's against them as an
# early warning: an entry trunk has edited is reported rather than failing configuration, and doc/vendor.md records the
# stable rung alone.
find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)
set(XSTD_BITS_STD_TEST_SUITE_RUNG stable CACHE STRING "The cpp-ci rung whose pinned suites run: stable, or development")
set_property(CACHE XSTD_BITS_STD_TEST_SUITE_RUNG PROPERTY STRINGS stable development)
set(rung ${XSTD_BITS_STD_TEST_SUITE_RUNG})
function(xstd_bits_std_test_suite suite subject)
    set(name ${suite}-${subject})
    list(TRANSFORM ARGN PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/ OUTPUT_VARIABLE mappings)
    if(NOT rung STREQUAL "stable")
        std_test_suite(${suite} NAME ${name} MAPPING ${mappings} ${CMAKE_CURRENT_SOURCE_DIR}/${subject}.toml LINK_LIBRARIES xstd::bits RUNG ${rung} WARN_STALE)
        return()
    endif()
    std_test_suite(${suite} NAME ${name} MAPPING ${mappings} ${CMAKE_CURRENT_SOURCE_DIR}/${subject}.toml LINK_LIBRARIES xstd::bits)
    add_test(
        NAME std-test-suite.${name}.table
        COMMAND ${Python3_EXECUTABLE} -I ${PROJECT_SOURCE_DIR}/tools/vendor.py check ${CMAKE_CURRENT_BINARY_DIR}/std-test-suite/${name}/manifest.json
    )
    set_tests_properties(std-test-suite.${name}.table PROPERTIES LABELS "std-test-suite;${name}")
endfunction()

# Each library's own suite on the library it is written for, since each also checks its own library's choices.
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(_LIBCPP_VERSION version XSTD_BITS_STD_TEST_SUITE_LIBCPP)
if(XSTD_BITS_STD_TEST_SUITE_LIBCPP)
    set(suite libcxx)
else()
    set(suite libstdcxx)
endif()
xstd_bits_std_test_suite(${suite} bit-vector ${suite}-vector-bool.toml ${suite}-algorithms.toml)
xstd_bits_std_test_suite(${suite} bit-small-vector ${suite}-vector-bool.toml)
foreach(subject IN ITEMS bit-set bit-fixed-set bit-inplace-set bit-small-set)
    xstd_bits_std_test_suite(${suite} ${subject} ${suite}-set.toml)
endforeach()

# The MSVC STL's tests reach none of its internals, and no Linux leg has the MSVC STL, so they run beside either. cpp-ci
# pins them on the stable rung alone.
if(rung STREQUAL "stable")
    foreach(subject IN ITEMS bit-vector bit-small-vector)
        xstd_bits_std_test_suite(msvc-stl ${subject} msvc-stl-vector-bool.toml)
    endforeach()
endif()
