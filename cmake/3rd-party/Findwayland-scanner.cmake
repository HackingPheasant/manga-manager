# SPDX-FileCopyrightText: © 2024 HackingPheasant <HackingPheasant@protonmail.com>
#
# SPDX-License-Identifier: MIT

# Findwayland-scanner.cmake
#
# Finds wayland-scanner, a tool used to process XML files related to the 
# wayland protocol and generates code from them. It generates C headers and
# glue code.
#
#
# This will define the following variables:
#
#   wayland-scanner_FOUND
#   True if wayland-scanner is available
#
#   wayland-scanner_EXECUTABLE
#   The wayland-scannner executable
#
# If wayland-scanner_FOUND is TRUE, it will also define the following imported
# target:
#
#   Wayland::scanner
#   the wayland-scanner executable
#

find_program(wayland-scanner_EXECUTABLE NAMES wayland-scanner)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(wayland-scanner REQUIRED_VARS wayland-scanner_EXECUTABLE)

if (wayland-scanner_FOUND)
    mark_as_advanced(wayland-scanner_EXECUTABLE)
endif()

if(wayland-scanner_FOUND AND NOT TARGET Wayland::scanner)
    add_executable(wayland-scanner IMPORTED)
    add_executable(Wayland::scanner ALIAS wayland-scanner)
    set_target_properties(wayland-scanner PROPERTIES IMPORTED_LOCATION "${wayland-scanner_EXECUTABLE}")
endif()
