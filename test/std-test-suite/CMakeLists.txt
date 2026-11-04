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

# A suite's tests, and a test that doc/vendor.md records what became of each of them in this build.
find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)
function(xstd_bits_std_test_suite suite)
    std_test_suite(${suite} MAPPING ${CMAKE_CURRENT_SOURCE_DIR}/${suite}.toml LINK_LIBRARIES xstd::bits)
    add_test(
        NAME std-test-suite.${suite}.table
        COMMAND ${Python3_EXECUTABLE} -I ${PROJECT_SOURCE_DIR}/tools/vendor.py check ${CMAKE_CURRENT_BINARY_DIR}/std-test-suite/${suite}/manifest.json
    )
    set_tests_properties(std-test-suite.${suite}.table PROPERTIES LABELS "std-test-suite;${suite}")
endfunction()

# Each library's own suite on the library it is written for, since each also checks its own library's choices.
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(_LIBCPP_VERSION version XSTD_BITS_STD_TEST_SUITE_LIBCPP)
if(XSTD_BITS_STD_TEST_SUITE_LIBCPP)
    xstd_bits_std_test_suite(libcxx)
else()
    xstd_bits_std_test_suite(libstdcxx)
endif()

# The MSVC STL's tests reach none of its internals, and no Linux leg has the MSVC STL, so they run beside either.
xstd_bits_std_test_suite(msvc-stl)
