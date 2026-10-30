#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

# Run as a script: compiles SOURCE to the assembly file ASM with the compiler CXX and the space-separated FLAGS, then
# reads it two ways. FileCheck checks the directives in SOURCE under the comma-separated PREFIXES, and must reject
# them instead where EXPECT is "mismatch". SAME_BODIES, comma-separated pairs of the form first=second, names
# functions that must compile to the same instructions: the same mnemonics as often, in whatever order the compiler
# schedules them and whichever registers it allocates.

cmake_minimum_required(VERSION 3.30)

separate_arguments(flags UNIX_COMMAND "${FLAGS}")
execute_process(
    COMMAND "${CXX}" ${flags} -S -o "${ASM}" "${SOURCE}"
    RESULT_VARIABLE compiled
    ERROR_VARIABLE diagnostics
)

# A mismatch is only evidence when the source compiled: a build error must never pass for an expected rejection.
if(NOT compiled EQUAL 0)
    message(FATAL_ERROR "${CXX} ${FLAGS} could not compile ${SOURCE}:\n${diagnostics}")
endif()

if(PREFIXES)
    execute_process(
        COMMAND "${FILECHECK}" "${SOURCE}" --check-prefixes "${PREFIXES}" --input-file "${ASM}"
        RESULT_VARIABLE checked
        OUTPUT_VARIABLE report
        ERROR_VARIABLE report
    )
    if(EXPECT STREQUAL "mismatch")
        if(checked EQUAL 0)
            message(FATAL_ERROR "FileCheck accepted ${ASM} under ${PREFIXES}, which these flags exist to make it reject.")
        endif()
        message(STATUS "FileCheck rejected ${ASM} under ${PREFIXES}, as expected:\n${report}")
    elseif(NOT checked EQUAL 0)
        message(FATAL_ERROR "FileCheck rejected ${ASM} under ${PREFIXES}:\n${report}")
    endif()
endif()

# The instructions from a function's label to the end of its body, without directives, labels, comments or endbr64,
# and their mnemonics sorted.
function(instructions_of lines name out_body out_mnemonics)
    set(inside FALSE)
    set(body "")
    set(mnemonics "")
    foreach(line IN LISTS lines)
        if(NOT inside)
            if(line MATCHES "^${name}:")
                set(inside TRUE)
            endif()
        elseif(line MATCHES "^[ \t]*\\.cfi_endproc" OR line MATCHES "^\\.Lfunc_end" OR line MATCHES "^[ \t]*\\.size[ \t]")
            break()
        elseif(line MATCHES "^[ \t]+[a-z]")
            # A comment is // or a # and a space; AArch64 writes an immediate as #1, which is an operand.
            string(REGEX REPLACE "[ \t]+(#[ \t]|//).*$" "" instruction "${line}")
            string(REGEX REPLACE "[ \t]+" " " instruction "${instruction}")
            string(STRIP "${instruction}" instruction)
            if(NOT instruction STREQUAL "endbr64")
                string(APPEND body "${instruction}\n")
                string(REGEX MATCH "^[^ ]+" mnemonic "${instruction}")
                list(APPEND mnemonics ${mnemonic})
            endif()
        endif()
    endforeach()
    if(NOT inside)
        message(FATAL_ERROR "No function ${name} in ${ASM}.")
    endif()
    list(SORT mnemonics)
    set(${out_body} "${body}" PARENT_SCOPE)
    set(${out_mnemonics} "${mnemonics}" PARENT_SCOPE)
endfunction()

if(SAME_BODIES)
    file(STRINGS "${ASM}" lines)
    string(REPLACE "," ";" pairs "${SAME_BODIES}")
    foreach(pair IN LISTS pairs)
        string(REPLACE "=" ";" names "${pair}")
        list(GET names 0 first)
        list(GET names 1 second)
        instructions_of("${lines}" ${first} first_body first_mnemonics)
        instructions_of("${lines}" ${second} second_body second_mnemonics)
        if(NOT first_mnemonics STREQUAL second_mnemonics)
            message(FATAL_ERROR "${first} and ${second} compile to different instructions.\n${first}:\n${first_body}\n${second}:\n${second_body}")
        endif()
        message(STATUS "${first} and ${second} compile to the same instructions.\n${first}:\n${first_body}\n${second}:\n${second_body}")
    endforeach()
endif()
