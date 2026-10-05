# SPDX-FileCopyrightText: © 2022 HackingPheasant <HackingPheasant@protonmail.com>
#
# SPDX-License-Identifier: MIT

# FindWayland.cmake
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

# TODO: https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
# TODO: Move to https://cmake.org/cmake/help/latest/command/cmake_pkg_config.html
find_package(PkgConfig QUIET REQUIRED)

# REDO
pkg_check_modules(wayland_client REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(wayland_cursor IMPORTED_TARGET wayland-cursor)
pkg_check_modules(wayland_server IMPORTED_TARGET wayland-server)

add_library(Wayland::client ALIAS PkgConfig::wayland_client)
add_library(Wayland::cursor ALIAS PkgConfig::wayland_cursor)
add_library(Wayland::server ALIAS PkgConfig::wayland_server)

set(Wayland_FOUND TRUE)
