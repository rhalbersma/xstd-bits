#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Hoisted out of test/ so benchmark/ can reach the same two targets. It has to live above both: the top-level
# lists add_subdirectory(benchmark) BEFORE add_subdirectory(test), so a target declared in test/ does not exist
# yet when the benchmarks are configured, and benchmark/src/set/blocks.cpp grades over exactly these Blocks.
#
# The cache options keep their _TEST_ spelling, which now reads narrower than what they gate. Renaming them
# would change a user-visible cache variable for no functional gain, and nothing in .github/ sets either, so
# both keep the name they were introduced under.

# The two 128-bit integer CLASSES the Block lists reach for, and neither a hard dependency: test/ext_int128.hpp
# detects each by __has_include, so a build without them simply drops it from the lists. They earn their place
# by being classes -- the builtin xstd::uint128 is a scalar the compiler lowers, while an operator on these is
# an ordinary function returning class type, which is what catches a container assuming a Block is a scalar.
# The declarations and tags mirror xstd-ints, which adapts the same two. [design.md#uint128-support]
option(${project_option_prefix}_TEST_FETCH_BOOST_INT128 "Fetch Boost.Int128 when no installed copy is found" ON)

find_package(boost_int128 CONFIG QUIET)
if(NOT TARGET Boost::int128 AND ${project_option_prefix}_TEST_FETCH_BOOST_INT128)
    include(FetchContent)
    FetchContent_Declare(
        boost_int128
        GIT_REPOSITORY https://github.com/cppalliance/int128.git
        # A development commit rather than v3.1.0, the types having lost their
        # _t suffix after it was cut. Pinned rather than tracking a branch.
        GIT_TAG f437657470186b79f1f314bc2c76a112bcc2529c
        # Its CMakeLists.txt builds its own test suite under BUILD_TESTING;
        # naming a subdirectory without one keeps the download and drops that.
        SOURCE_SUBDIR does-not-exist
    )
    FetchContent_MakeAvailable(boost_int128)

    # SYSTEM because these headers are not written to this project's warning
    # level; an installed copy arrives as an imported target, already system.
    add_library(Boost::int128 INTERFACE IMPORTED)
    target_include_directories(Boost::int128 SYSTEM INTERFACE ${boost_int128_SOURCE_DIR}/include)
endif()

if(TARGET Boost::int128)
    set(${project_id}_test_boost_int128 Boost::int128)
else()
    set(${project_id}_test_boost_int128)
endif()

option(${project_option_prefix}_TEST_FETCH_ABSL_INT128 "Fetch Abseil when no installed copy is found" ON)

find_package(absl CONFIG QUIET)
if(NOT TARGET absl::int128 AND ${project_option_prefix}_TEST_FETCH_ABSL_INT128)
    include(FetchContent)
    FetchContent_Declare(
        absl_int128
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        # A release tag, the spelling of these two types being settled.
        GIT_TAG 20260526.0
        # Abseil's CMakeLists.txt configures the whole library and its tests;
        # naming a subdirectory without one keeps the download and drops that,
        # leaving the one translation unit these two types need.
        SOURCE_SUBDIR does-not-exist
    )
    FetchContent_MakeAvailable(absl_int128)

    # int128.cc is where operator/ and operator% live on a platform with no
    # 128-bit intrinsic to lower them to; the rest of the type is header-only.
    add_library(${project_id}_test_absl_int128_lib STATIC ${absl_int128_SOURCE_DIR}/absl/numeric/int128.cc)

    # Built to the same standard as the tests, not to the compiler's default:
    # absl::strong_ordering is std::strong_ordering only where the library
    # sees C++20's <compare>, and the two spellings must not meet at a link.
    target_compile_features(${project_id}_test_absl_int128_lib PUBLIC cxx_std_${${project_option_prefix}_CXX_STANDARD})

    # SYSTEM for the reason Boost::int128's includes are.
    target_include_directories(${project_id}_test_absl_int128_lib SYSTEM PUBLIC ${absl_int128_SOURCE_DIR})

    add_library(absl::int128 ALIAS ${project_id}_test_absl_int128_lib)
endif()

if(TARGET absl::int128)
    set(${project_id}_test_absl_int128 absl::int128)
else()
    set(${project_id}_test_absl_int128)
endif()
