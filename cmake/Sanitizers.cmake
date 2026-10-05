# SPDX-FileCopyrightText: © 2020 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

# TODO: Look into the following:
# - https://clang.llvm.org/docs/SafeStack.html
# - https://clang.llvm.org/docs/ControlFlowIntegrity.html
# - https://clang.llvm.org/docs/DataFlowSanitizer.html

function(enable_sanitizers project_name)
    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" OFF)
    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)

    set(SANITIZERS "")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")

        if(${ENABLE_SANITIZER_ADDRESS})
            list(APPEND SANITIZERS "address")
        endif()

        if(${ENABLE_SANITIZER_LEAK})
            list(APPEND SANITIZERS "leak")
        endif()

        if(${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR})
            list(APPEND SANITIZERS "undefined")
        endif()

        if(${ENABLE_SANITIZER_THREAD})
            if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
                message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
            else()
                list(APPEND SANITIZERS "thread")
            endif()
        endif()

        if(${ENABLE_SANITIZER_MEMORY} AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            message(WARNING "Memory sanitizer requires all the code (including libc++) to be MSan-instrumented otherwise it reports false positives")
            if("address" IN_LIST SANITIZERS
                    OR "thread" IN_LIST SANITIZERS
                    OR "leak" IN_LIST SANITIZERS)
                message(WARNING "Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled")
            else()
                list(APPEND SANITIZERS "memory")
            endif()
        endif()
    elseif(MSVC)
        if(${ENABLE_SANITIZER_ADDRESS})
            list(APPEND SANITIZERS "address")
        endif()
        if(${ENABLE_SANITIZER_LEAK}
                OR ${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}
                OR ${ENABLE_SANITIZER_THREAD}
                OR ${ENABLE_SANITIZER_MEMORY})
            message(WARNING "MSVC only supports address sanitizer")
        endif()
    endif()

    list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)

    if(LIST_OF_SANITIZERS)
        if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
            if(NOT MSVC)
                target_compile_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS} -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment)
                target_link_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS} -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment)
            else()
                target_compile_options(${project_name} INTERFACE /fsanitize=${LIST_OF_SANITIZERS} /Zi /INCREMENTAL:NO)
                target_compile_definitions(${project_name} INTERFACE _DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION)
                target_link_options(${project_name} INTERFACE /INCREMENTAL:NO)
            endif()
        endif()
    endif()

endfunction()
