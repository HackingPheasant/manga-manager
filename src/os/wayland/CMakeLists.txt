# SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_library(manga-manager.display-protocol.wayland)
add_library(manga-manager::display-protocol::wayland ALIAS manga-manager.display-protocol.wayland)

target_compile_definitions(manga-manager.display-protocol.wayland INTERFACE VK_USE_PLATFORM_WAYLAND_KHR WSI_WAYLAND)

target_sources(manga-manager.display-protocol.wayland
  PRIVATE
    wayland.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      wayland.hpp
)

wayland_generate_client(xdg-shell)
wayland_generate_client(xdg-decoration 1)


target_link_libraries(manga-manager.display-protocol.wayland PUBLIC
    project::options
    Wayland::client
    Wayland::client::xdg-shell
    Wayland::client::xdg-decoration::v1
)
