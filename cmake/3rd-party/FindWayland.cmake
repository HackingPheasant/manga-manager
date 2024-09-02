# FindWayland
# Finds Wayland, the modern display server protocol.
# Note: This FindWayland specifically skips over finding Wayland EGL stuff
# since this project has it's own vulkan renderer.

## Imported Targets
# This module defines the following IMPORTED targets if found:

# Wayland::client
# Headers and libraries for a client.

# Wayland::cursor
# Headers and libraries for the cursor API.


# Wayland::scanner
# Excutable used to convert wayland protocols

# Wayland::server
# Headers and libraries for a server.

## Wayland protocols
# Location of wayland.xml
# wayland_DATADIR

# Location of specifications of extended Wayland protocols
# wayland_protocols_DATADIR

include(FindPackageHandleStandardArgs)
include(FeatureSummary)

find_package(PkgConfig QUIET REQUIRED)


pkg_check_modules(wayland_client QUIET REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(wayland_cursor QUIET IMPORTED_TARGET wayland-cursor)
pkg_check_modules(wayland_protocols QUIET wayland-protocols)
pkg_get_variable(wayland_protocols_DATADIR wayland-protocols pkgdatadir)
pkg_check_modules(wayland_scanner QUIET wayland-scanner)
pkg_get_variable(wayland_scanner_EXECUTABLE wayland-scanner wayland_scanner)
pkg_get_variable(wayland_DATADIR wayland-scanner pkgdatadir)
pkg_check_modules(wayland_server QUIET IMPORTED_TARGET wayland-server)

message(STATUS "Wayland Scanner: ${wayland_scanner_EXECUTABLE}")

add_library(Wayland::client ALIAS PkgConfig::wayland_client)
add_library(Wayland::cursor ALIAS PkgConfig::wayland_cursor)
add_library(Wayland::server ALIAS PkgConfig::wayland_server)

# Wayland::scanner
mark_as_advanced(wayland_scanner_EXECUTABLE)
if(wayland_scanner_EXECUTABLE)
    set(wayland_scanner_FOUND TRUE)
else()
    set(wayland_scanner_FOUND FALSE)
endif()

if(wayland_scanner_EXECUTABLE AND NOT TARGET Wayland::scanner)
    add_executable(Wayland::scanner IMPORTED)
    set_property(TARGET Wayland::scanner PROPERTY IMPORTED_LOCATION "${wayland_scanner_EXECUTABLE}")
endif()

# Following functions/macros are based on the below merge request from
# Raul Tambre (@tambre) <raul@tambre.ee> to CMake (BSD 3-clause Licensed)
# https://gitlab.kitware.com/cmake/cmake/-/merge_requests/4473/diffs
# https://gitlab.kitware.com/cmake/cmake/-/blob/master/Copyright.txt

# wayland_generate(<type> <input> <output>)
# Runs wayland-scanner. <input> is relative to wayland-protocols pkgdatadir (wayland_protocols_DATADIR).
# Examples:
# wayland_generate(client-header unstable/xdg-decoration/xdg-decoration-unstable-v1.xml /opt/xdg-decoration-client-protocol.h)
# wayland_generate(private-code unstable/xdg-decoration/xdg-decoration-unstable-v1.xml /opt/xdg-decoration-protocol.c)
function(wayland_generate type input output)
    add_custom_command(
        OUTPUT ${output}
        COMMAND Wayland::scanner ${type} ${wayland_protocols_DATADIR}/${input} ${output}
    )
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

macro(wayland_generate_code type)
    if(status STREQUAL unstable)
        set(code ${binary_directory}/${name}-protocol-v${version}.c)
    else()
        set(code ${binary_directory}/${name}-protocol.c)
    endif()
    wayland_generate(${type}-code ${input} ${code})
endmacro()

macro(wayland_generate_header type)
    if(status STREQUAL unstable)
        set(header ${binary_directory}/${name}-${type}-protocol-v${version}.h)
    else()
        set(header ${binary_directory}/${name}-${type}-protocol.h)
    endif()
    set(header_target ${name}_${type}_header)

    wayland_generate(client-header ${input} ${header})
    add_custom_target(${header_target} DEPENDS "${header}")
    add_dependencies(${target} ${header_target})
endmacro()

# Note: These are building C libraries, and our project is being built as CXX 
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
        target_include_directories(${target} PUBLIC ${binary_directory})
        target_link_libraries(${target} PUBLIC Wayland::client)

        wayland_generate_header(client)
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
        target_include_directories(${target} PUBLIC ${binary_directory})
        target_link_libraries(${target} PUBLIC Wayland::server)

        wayland_generate_header(server)
    endif()
endfunction()
