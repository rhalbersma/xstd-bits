#          Copyright Rein Halbersma 2014-2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.28)
project(
    xstd-bits
    VERSION 0.1.0
    DESCRIPTION "Rebooting the bits franchise"
    # Spelled out: project()'s own arguments expand before it sets PROJECT_NAME, so ${PROJECT_NAME} here is empty.
    HOMEPAGE_URL https://github.com/rhalbersma/xstd-bits
    LANGUAGES CXX
)

include(GNUInstallDirs)

# The project's own name, spelled once. TOUPPER alone leaves XSTD-BITS, which no cache option can be called;
# MAKE_C_IDENTIFIER alone leaves the lowercase form. Both, in this order, and test/ and benchmark/ inherit them.
string(MAKE_C_IDENTIFIER ${PROJECT_NAME} project_id)            # xstd_bits
string(TOUPPER ${project_id} project_option_prefix)             # XSTD_BITS

# Warnings are errors while this library is being developed, which is what the presets ask for.
# Declared here rather than under test/, as xstd and tabula do, because benchmark/ carries its
# own warning set and has to be gated by the same switch.
option(${project_option_prefix}_WARNINGS_AS_ERRORS "Treat warnings as errors in ${PROJECT_NAME} tests and benchmarks" ${PROJECT_IS_TOP_LEVEL})

# Prefer an installed xstd-ints so install(EXPORT) can record it as a dependency; a FetchContent build tree cannot be exported.
find_package(xstd-ints 0.1.0 CONFIG QUIET)
if(NOT xstd-ints_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        xstd-ints
        GIT_REPOSITORY https://github.com/rhalbersma/xstd-ints.git
        # Pinned to a commit on xstd-ints' main, not a branch head; move consumption.yml's dependency_repos with it.
        GIT_TAG a053f05062a12657eeaef47637b8c0be4483fc35
    )
    FetchContent_MakeAvailable(xstd-ints)
endif()

# xstd-misc supplies conditional_data_member_t and XSTD_NO_UNIQUE_ADDRESS, which
# contiguous_bit_container uses to carry a run-time width without paying for it under a compile-time one.
find_package(xstd-misc 0.1.0 CONFIG QUIET)
if(NOT xstd-misc_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        xstd-misc
        GIT_REPOSITORY https://github.com/rhalbersma/xstd-misc.git
        # Pinned to a commit on xstd-misc's main, not a branch head; move consumption.yml's dependency_repos with it.
        GIT_TAG 4ef4886c093afdb41f1979eda2a5cff51acf076b
    )
    FetchContent_MakeAvailable(xstd-misc)
endif()

# Boost.Hash2 is a public dependency of this library's headers, not just of the tests, so it is declared explicitly.
find_package(boost_hash2 CONFIG REQUIRED)

add_library(
    ${PROJECT_NAME} INTERFACE
)

# Consumed as xstd::bits, following Boost: the package keeps the repository's name
# (find_package(xstd-bits)) while the target is a component of one umbrella namespace,
# so xstd::bits sits beside xstd::ints the way Boost::hash2 sits beside Boost::mp11.
# EXPORT_NAME is what lets the exported name differ from the target name; the alias
# below must spell the same thing, or an add_subdirectory consumer and a find_package
# consumer would end up with two different names for one library.
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME bits)
add_library(xstd::bits ALIAS ${PROJECT_NAME})

# No target_include_directories: the HEADERS file set supplies both the build and the install interface.
target_sources(
    ${PROJECT_NAME} INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/xstd/bits.hpp
        include/xstd/bits/sequence_adaptor.hpp
        include/xstd/bits/set_adaptor.hpp
        include/xstd/bits/bitset_adaptor.hpp
        include/xstd/bits/bit_array.hpp
        include/xstd/bits/bit_inplace_set.hpp
        include/xstd/bits/bit_inplace_vector.hpp
        include/xstd/bits/inplace_bitset.hpp
        include/xstd/bits/bit_set.hpp
        include/xstd/bits/bit_set_view.hpp
        include/xstd/bits/bit_span.hpp
        include/xstd/bits/bitset.hpp
        include/xstd/bits/bit_static_set.hpp
        include/xstd/bits/bit_subspan.hpp
        include/xstd/bits/bit_vector.hpp
        include/xstd/bits/bit_traits.hpp
        include/xstd/bits/detail/allocator_typedef.hpp
        include/xstd/bits/detail/bidirectional.hpp
        include/xstd/bits/detail/contiguous_bit_array.hpp
        include/xstd/bits/detail/contiguous_bit_container.hpp
        include/xstd/bits/detail/contiguous_bit_inplace_vector.hpp
        include/xstd/bits/detail/contiguous_bit_vector.hpp
        include/xstd/bits/detail/hash.hpp
        include/xstd/bits/detail/intrin.hpp
        include/xstd/bits/dynamic_bitset.hpp
        include/xstd/bits/detail/pred.hpp
        include/xstd/bits/detail/random_access.hpp
        include/xstd/bits/detail/shift.hpp
        include/xstd/bits/ext/boost.hpp
        include/xstd/bits/ext/boost/dynamic_bitset.hpp
        include/xstd/bits/ext/std.hpp
        include/xstd/bits/ext/std/bitset.hpp
        include/xstd/bits/ownership.hpp
)

# The list above is hand-written, and it drifted: the three inplace headers reached include/ and bits.hpp but
# never this file set, so every installed consumer's <xstd/bits.hpp> named three headers that were not installed.
# No compiler leg could see it -- they all build from the source tree -- and the consumer translation unit that
# does build against the install tree had included one container header until it was made the interface gate.
# Checked here from now on, the way test/ checks that every public header is mirrored. [design.md#the-interface-line]
# Every header under include/, not just include/xstd/: include/ is exactly what a consumer gets, and the workloads
# that used to sit beside it now live in examples/. A stray directory here would otherwise ride the file set's
# BASE_DIRS onto every add_subdirectory consumer's include path while never being installed.
# [design.md#the-interface-line]
# HEADER_SET answers in absolute paths, so the glob asks for them that way and the message relativizes back.
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
        # The library requires C++23 and says so above, in the INTERFACE a consumer inherits. The inplace column
        # needs more: it is behind __cpp_lib_inplace_vector, which is C++26, so at the minimum those three of the
        # nine cells compile to nothing and are never tested. Raising the standard here rather than in the
        # INTERFACE keeps the requirement a consumer sees at 23 while letting a leg of the matrix build the whole
        # matrix. [design.md#the-inplace-column]
        set(${project_option_prefix}_CXX_STANDARD 23 CACHE STRING "C++ standard for the tests and benchmarks: 23, or 26 to reach the inplace column")
        set_property(CACHE ${project_option_prefix}_CXX_STANDARD PROPERTY STRINGS 23 26)

        # Asking for a standard CMake cannot spell for this compiler fails deep inside a try_compile, with a
        # message blaming the compiler. CMAKE_CXX_COMPILE_FEATURES is what CMake actually knows, so ask it.
        if(NOT "cxx_std_${${project_option_prefix}_CXX_STANDARD}" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
            message(FATAL_ERROR
                "${project_option_prefix}_CXX_STANDARD=${${project_option_prefix}_CXX_STANDARD} is not something CMake ${CMAKE_VERSION} can ask "
                "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} for. CMake 3.28 knows no C++26 for GCC or "
                "Clang at all; a newer CMake does. The inplace column needs it, the other six cells do not."
            )
        endif()

        set(CMAKE_CXX_STANDARD ${${project_option_prefix}_CXX_STANDARD})
        set(CMAKE_CXX_STANDARD_REQUIRED ON)

        # Left ON, which is CMake's default, so the matrix compiles as gnu++23 where xstd and tabula compile as
        # c++23. Stated rather than omitted, because the reason is specific and the difference is deliberate:
        # <bit>'s functions take std::unsigned_integral alone, and libstdc++ and libc++ carry xstd::uint128 into
        # that concept only outside __STRICT_ANSI__. Turning extensions off would drop the widest Block from
        # every instantiation list rather than fail, which is the worse of the two outcomes.
        # [design.md#uint128-support]
        set(CMAKE_CXX_EXTENSIONS ON)

        add_subdirectory(benchmark)
        add_subdirectory(test)
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
