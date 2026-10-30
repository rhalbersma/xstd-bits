#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# The expectations are written against GCC's and Clang's assembly, in Intel syntax on x86-64.
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    message(FATAL_ERROR "${project_option_prefix}_BUILD_CODEGEN_TESTS needs GCC or clang with its GNU-style driver; this is ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.")
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    message(FATAL_ERROR "${project_option_prefix}_BUILD_CODEGEN_TESTS has expectations for x86-64 only; this is ${CMAKE_SYSTEM_PROCESSOR}.")
endif()

# The Python port that tools/filecheck-requirements.txt pins, or LLVM's own, which reads the same directives.
find_program(
    ${project_option_prefix}_FILECHECK
    NAMES filecheck FileCheck
    HINTS $ENV{HOME}/.local/bin
    REQUIRED
)

# The include directories a consumer of the library target compiles with, xstd-ints', xstd-misc's and Boost's among
# them, as one -I per line of a response file the compiler reads with @.
set(includes_file ${CMAKE_CURRENT_BINARY_DIR}/includes.rsp)
file(
    GENERATE
    OUTPUT ${includes_file}
    CONTENT "$<JOIN:$<LIST:TRANSFORM,$<LIST:REMOVE_DUPLICATES,$<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>>,PREPEND,-I>,\n>\n"
)

# What a consumer builds: the strictly conforming dialect, optimized, with the library's asserts compiled out.
set(common_flags "-std=c++23 -O2 -DNDEBUG @${includes_file} -masm=intel")

# One test per source and target configuration. Every configuration reads CHECK, which labels each function.
function(add_codegen_test source config flags prefixes)
    cmake_parse_arguments(PARSE_ARGV 4 arg "" "EXPECT;SAME_BODIES" "")
    get_filename_component(stem ${source} NAME_WE)
    set(name ${stem}.${config})
    add_test(
        NAME codegen.${name}
        COMMAND
            ${CMAKE_COMMAND}
            -DCXX=${CMAKE_CXX_COMPILER}
            "-DFLAGS=${common_flags} ${flags}"
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${source}
            -DASM=${CMAKE_CURRENT_BINARY_DIR}/${name}.s
            -DFILECHECK=${${project_option_prefix}_FILECHECK}
            -DPREFIXES=${prefixes}
            -DEXPECT=${arg_EXPECT}
            -DSAME_BODIES=${arg_SAME_BODIES}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/check_codegen.cmake
    )
endfunction()

# The hash's expansion of a block into bools: the byte table, a PDEP per byte, or one masked store.
add_codegen_test(expand.cpp x86-64 "-march=x86-64" "CHECK,PORTABLE")
add_codegen_test(expand.cpp x86-64-v3 "-march=x86-64-v3" "CHECK,FAST-BMI2")
add_codegen_test(expand.cpp x86-64-v4 "-march=x86-64-v4" "CHECK,PORTABLE,AVX512BW")

# BMI2 is enabled on Zen 2, but PDEP is microcoded there: the table must answer, and the PDEP checks must fail.
add_codegen_test(expand.cpp znver2 "-march=znver2" "CHECK,PORTABLE")
add_codegen_test(expand.cpp bmi2-excluded "-march=znver2" "CHECK,FAST-BMI2" EXPECT mismatch)

# Each set operation beside the same operation on the words it holds: the library is no worse than writing it by hand.
set(
    block_pairs
    codegen_set1_and=codegen_word1_and
    codegen_set1_or=codegen_word1_or
    codegen_set1_xor=codegen_word1_xor
    codegen_set1_minus=codegen_word1_minus
    codegen_set1_size=codegen_word1_size
    codegen_set1_lookup=codegen_word1_lookup
    codegen_set1_pop_front=codegen_word1_pop_front
    codegen_set2_and=codegen_word2_and
    codegen_set2_or=codegen_word2_or
    codegen_set2_xor=codegen_word2_xor
    codegen_set2_minus=codegen_word2_minus
    codegen_set2_size=codegen_word2_size
)
list(JOIN block_pairs "," block_pairs)
foreach(march x86-64 x86-64-v3)
    add_codegen_test(blocks.cpp ${march} "-march=${march}" "CHECK,BRANCH-FREE" SAME_BODIES ${block_pairs})
endforeach()
