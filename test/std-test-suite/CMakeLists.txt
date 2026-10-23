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

std_test_suite(libcxx MAPPING ${CMAKE_CURRENT_SOURCE_DIR}/libcxx.toml LINK_LIBRARIES xstd::bits)
