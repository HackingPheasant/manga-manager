# SPDX-FileCopyrightText: © 2024 HackingPheasant <HackingPheasant@protonmail.com>
#
# SPDX-License-Identifier: MIT

# Findwayland-protocols.cmake
#
# Finds wayland-protocols, a collection of XML files that describe various protocols 
#
#
# This will define the following variables:
#
#   wayland-protocols_FOUND
#   True if wayland-protocols is available
#
#   wayland_protocols_DATADIR
#   Location of XML Files that describe various wayland-protocols
#
# If wayland-scanner_FOUND is TRUE, it will also define the following imported
# target:
#
#   Wayland::scanner
#   the wayland-scanner executable
#

# TODO: https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
# TODO: Move to cmake_pkg_config
# https://cmake.org/cmake/help/latest/command/cmake_pkg_config.html
find_package(PkgConfig QUIET REQUIRED)

pkg_check_modules(wayland-protocols wayland-protocols)
pkg_get_variable(wayland-protocols_DATADIR wayland-protocols pkgdatadir)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(wayland-protocols REQUIRED_VARS wayland-protocols_DATADIR)

if (wayland-protocols_FOUND)
    mark_as_advanced(wayland-protocols_DATADIR)
endif()
