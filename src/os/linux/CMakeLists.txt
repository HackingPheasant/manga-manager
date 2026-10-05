# SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT
add_library(manga-manager.os.linux)
add_library(manga-manager::os::linux ALIAS manga-manager.os.linux)

target_link_libraries(manga-manager.os.linux PUBLIC
  project::options
)

if(X11_SUPPORT)
  target_link_libraries(manga-manager.os.linux PUBLIC manga-manager::display-protocol::x11)
endif()

if(WAYLAND_SUPPORT)
  target_link_libraries(manga-manager.os.linux PUBLIC manga-manager::display-protocol::wayland)
endif()

if(NOT X11_SUPPORT AND NOT WAYLAND_SUPPORT)
    message(FATAL_ERROR "Linux: Unsupported Platform! Please enable atleast X11 or Wayland support in the build system")
endif()
