# SPDX-FileCopyrightText: © 2025 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

# Required:
# TARGET_NAME - The target to associate the resultant output with
#
# Optional:
# INTERFACE <files> - Any shader files specified here are marked as 'INTERFACE' in the shader FILE_SET
# PUBLIC <files> - Any shader files specified here are marked as 'PUBLIC' in the shader FILE_SET
# PRIVATE <files> - Any shader files specified here are marked as 'PRIVATE' in the shader FILE_SET
# COMPILE_OPTIONS <options> - These options are passed straight to the 'glslang' executable as is
#
# Example:
#
# target_compile_shaders(mytarget
#                          PRIVATE test.vert test.frag
#                          COMPILE_OPTIONS --target-env vulkan1.3 -x)

function(target_compile_shaders TARGET_NAME)
    set(OPTIONS)
    set(ONE_VALUE_ARGS)
    set(MULTI_VALUE_ARGS INTERFACE PUBLIC PRIVATE COMPILE_OPTIONS)

    cmake_parse_arguments(PARSE_ARGV 0 arg "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}")

    foreach(SHADER IN LISTS arg_INTERFACE)
        add_custom_command(COMMENT "Compiling ${SHADER} shader"
            OUTPUT ${SHADER}.glsl # Treated as relative to CMAKE_CURRENT_BINARY_DIR
            COMMAND glslang::glslang-standalone ${arg_COMPILE_OPTIONS} -V  -o ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.glsl ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            MAIN_DEPENDENCY ${SHADER}
            DEPENDS ${SHADER} glslang::glslang-standalone # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
            CODEGEN
            DEPENDS_EXPLICIT_ONLY)

        target_sources(${TARGET_NAME}
            INTERFACE
            FILE_SET interface_compiled_shaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
            FILES
            ${SHADER}.glsl)
    endforeach()
    
    foreach(SHADER IN LISTS arg_PUBLIC)
        add_custom_command(COMMENT "Compiling ${SHADER} shader"
            OUTPUT ${SHADER}.glsl # Treated as relative to CMAKE_CURRENT_BINARY_DIR
            COMMAND glslang::glslang-standalone ${arg_COMPILE_OPTIONS} -V  -o ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.glsl ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            MAIN_DEPENDENCY ${SHADER}
            DEPENDS ${SHADER} glslang::glslang-standalone # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
            CODEGEN
            DEPENDS_EXPLICIT_ONLY)

        target_sources(${TARGET_NAME}
            PUBLIC
            FILE_SET public_compiled_shaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
            FILES
            ${SHADER}.glsl)
    endforeach()

    foreach(SHADER IN LISTS arg_PRIVATE)
        add_custom_command(COMMENT "Compiling ${SHADER} shader"
            OUTPUT ${SHADER}.glsl # Treated as relative to CMAKE_CURRENT_BINARY_DIR
            COMMAND glslang::glslang-standalone ${arg_COMPILE_OPTIONS} -V  -o ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.glsl ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            MAIN_DEPENDENCY ${SHADER}
            DEPENDS ${SHADER} glslang::glslang-standalone # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
            CODEGEN
            DEPENDS_EXPLICIT_ONLY)

        target_sources(${TARGET_NAME}
            PRIVATE
            FILE_SET private_compiled_shaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
            FILES
            ${SHADER}.glsl)
    endforeach()
endfunction()
