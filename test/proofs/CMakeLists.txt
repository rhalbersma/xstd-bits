#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# The proofs and the drift check are Python, with Z3's bindings from tools/z3-requirements.txt, over the word operations
# of the pinned xstd-ints' own proofs, which FetchContent put beside the build.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NOT DEFINED xstd-ints_SOURCE_DIR)
    message(FATAL_ERROR "The proofs import xstd-ints' test/proofs/operations.py from the pinned sources, which an installed xstd-ints does not ship; configure without it on the prefix path.")
endif()
set(ints_proofs ${xstd-ints_SOURCE_DIR}/test/proofs)

# The compiled block kernels, at every block width, for the drift check to hold the models against. GNU extensions keep
# the 128-bit block, which strict ISO mode on GCC 15 does not count as an integer type.
add_executable(proofs_drift drift.cpp)
target_link_libraries(proofs_drift PRIVATE xstd::bits)
target_include_directories(proofs_drift PRIVATE ${PROJECT_SOURCE_DIR}/test/include)
set_target_properties(proofs_drift PROPERTIES CXX_EXTENSIONS ON)
target_compile_options(
    proofs_drift PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra>
)

add_test(NAME proofs.drift COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/drift.py $<TARGET_FILE:proofs_drift> --ints-proofs ${ints_proofs})

# Every block width the library supports, whatever this build's: the proofs are about the kernels, not this build of them.
add_test(NAME proofs.z3 COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/prove.py --widths 8,16,32,64,128 --ints-proofs ${ints_proofs})

set_tests_properties(proofs.drift proofs.z3 PROPERTIES TIMEOUT 7200)
