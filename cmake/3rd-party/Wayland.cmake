# SPDX-FileCopyrightText: © 2022 HackingPheasant <HackingPheasant@protonmail.com>
#
# SPDX-License-Identifier: MIT

# TODO See if we should be guarding agaisnt multiple inclusion of if find_package already handles that?
if(TARGET Wayland::client)
    return()
endif()

# This module provides the following functions to generate code via wayland-scanner:
#
# wayland_generate_client(<name> [version])
#
#   Generates client protocol header and private code. Defines IMPORTED target:
#   Wayland::client::${name}(::v${version}).
# 
# wayland_generate_server(<name> [version])
#
#   Generates server protocol header and private code. Defines IMPORTED target:
#   Wayland::server::${name}(::v${version})


# SPDX-SnippetBegin
# SPDX-SnippetCopyrightText: © 2020 Raul Tambre <raul@tambre.ee>
# SPDX-SnippetLicenseConcluded: BSD-3-Clause
# SPDX-SnippetLicenseComments: <text>The concluded license was taken from the 
# CMake repo, from which the snippet was copied into the current file. The 
# concluded license information was found in the LICENSE.rst file in the CMake repo.</text>
# SPDX-SnippetComment: <text>Some funcions/macros related to simplifying the
# use of wayland-scanner inside CMake related code. It was copied from a 
# unaccepted merge request (!4473) to the CMake repo, and then futher modified
# for use in this project.</text>

# Original source:
# https://gitlab.kitware.com/cmake/cmake/-/merge_requests/4473/diffs

# wayland_generate(<type> <input> <output>)
# Runs wayland-scanner.
# Examples:
# wayland_generate(client-header unstable/xdg-decoration/xdg-decoration-unstable-v1.xml /opt/xdg-decoration-client-protocol.h)
# wayland_generate(private-code unstable/xdg-decoration/xdg-decoration-unstable-v1.xml /opt/xdg-decoration-protocol.c)
function(wayland_generate type input output)
    set(PROTOCOL_DIR ${wayland-protocols_DATADIR})
    add_custom_command(
        OUTPUT ${output}
        COMMAND Wayland::scanner ${type} ${PROTOCOL_DIR}/${input} ${output}
        MAIN_DEPENDENCY ${PROTOCOL_DIR}/${input}
        DEPENDS ${PROTOCOL_DIR}/${input} Wayland::scanner
        CODEGEN)
endfunction()

macro(wayland_generate_parse_arguments type)
    if(${ARGV1})
        set(status unstable)
        set(version ${ARGV1})
        set(target wayland_${type}_${name}_v${version})
        set(alias Wayland::${type}::${name}::v${version})
        set(input ${status}/${name}/${name}-unstable-v${version}.xml)
    else()
        set(status stable)
        set(version stable)
        set(target wayland_${type}_${name})
        set(alias Wayland::${type}::${name})
        set(input ${status}/${name}/${name}.xml)
    endif()

    set(binary_directory ${CMAKE_CURRENT_BINARY_DIR}/wayland/${name}/${status})
endmacro()

# TODO change it so we don't hardcode the wayland-protocols datadir but can 
# pass in any arbitary xml file instead
macro(wayland_generate_code type)
    if(status STREQUAL unstable)
        set(code ${binary_directory}/${name}-protocol-v${version}.c)
    else()
        set(code ${binary_directory}/${name}-protocol.c)
    endif()
    wayland_generate("${type}-code" ${input} ${code})
endmacro()

macro(wayland_generate_header type)
    if(status STREQUAL unstable)
        set(header ${binary_directory}/${name}-${type}-protocol-v${version}.h)
    else()
        set(header ${binary_directory}/${name}-${type}-protocol.h)
    endif()
    set(header_target ${name}_${type}_header)

    wayland_generate("${type}-header" ${input} ${header})
    add_custom_target(${header_target} DEPENDS "${header}")
    add_dependencies(${target} ${header_target})
endmacro()

# Note: These are building C libraries, and our projecserver-headert is being built as CXX 
# so the top-most cmake project(...) should have C and CXX in the languages for
# the targets to build and link correctly. This oversight took a while to debug.

# wayland_generate_client(<name> [version])
# Generates client protocol header and private code. Defines IMPORTED target
# Wayland::client::${name}(::v${version}).
# Examples:
# wayland_generate_client(xdg-shell) # xdg-shell and Wayland::client::xdg-shell
# wayland_generate_client(xdg-shell 6) # xdg-shell-unstable-v6 and Wayland::client::xdg-shell::v6
function(wayland_generate_client name)
    wayland_generate_parse_arguments(client)

    if(NOT TARGET ${target})
        file(MAKE_DIRECTORY ${binary_directory})

        wayland_generate_code(private)
        add_library(${target} ${code})
        add_library(${alias} ALIAS ${target})

        wayland_generate_header(client)
        #target_include_directories(${target} PUBLIC ${binary_directory})
        target_sources(${target} PUBLIC
            FILE_SET "waylandGeneratedHeaders"
            TYPE HEADERS
            BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/wayland/"
            FILES "${header}")
        target_link_libraries(${target} PUBLIC Wayland::client)
    endif()
endfunction()

# wayland_generate_server(<name> [version])
# Generates server protocol header and private code. Defines IMPORTED target
# Wayland::server::${name}(::v${version})
function(wayland_generate_server name)
    wayland_generate_parse_arguments(server)

    if(NOT TARGET ${target})
        file(MAKE_DIRECTORY ${binary_directory})

        wayland_generate_code(private)
        add_library(${target} ${code})
        add_library(${alias} ALIAS ${target})

        wayland_generate_header(server)
        #target_include_directories(${target} PUBLIC ${binary_directory})
        target_sources(${target} PUBLIC
            FILE_SET "waylandGeneratedHeaders"
            TYPE HEADERS
            BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/wayland/"
            FILES "${header}")
        target_link_libraries(${target} PUBLIC Wayland::server)
    endif()
endfunction()

# SPDX-SnippetEnd
