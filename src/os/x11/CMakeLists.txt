# SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_library(manga-manager.display-protocol.x11)
add_library(manga-manager::display-protocol::x11 ALIAS manga-manager.display-protocol.x11)
    
# We will only support XCB and not Xlib, because under the hood, libX11
# is already making use of XCB
# https://www.x.org/wiki/guide/xlib-and-xcb/#index2h2
target_compile_definitions(manga-manager.display-protocol.x11 INTERFACE VK_USE_PLATFORM_XCB_KHR WSI_X11)

target_sources(manga-manager.display-protocol.x11
  PRIVATE
    x11.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      x11.hpp
)

target_link_libraries(manga-manager.display-protocol.x11 PUBLIC
    project::options
    xcb::xcb
)
