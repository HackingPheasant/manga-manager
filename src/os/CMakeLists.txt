# SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_library(manga-manager.os)
add_library(manga-manager::os ALIAS manga-manager.os)

target_sources(manga-manager.os
  PUBLIC
    FILE_SET HEADERS
    FILES
      window-interface.hpp
      wsi.hpp
)

target_link_libraries(manga-manager.os PUBLIC
    project::options
    Vulkan::Headers
)

if(X11_SUPPORT)
    add_subdirectory(x11)
    #target_link_libraries(manga-manager.os PUBLIC manga-manager::display-protocol::x11)
endif()

if(WAYLAND_SUPPORT)
    add_subdirectory(wayland)
    #target_link_libraries(manga-manager.os PUBLIC manga-manager::display-protocol::wayland)
endif()

# TODO: For OS Specific API's/Functions, implement wrappers for more systems, 
# such as *BSD or other UNIX based systems, etc.
# https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_NAME.html#system-names-known-to-cmake
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(windows) # TODO
    target_link_libraries(manga-manager.os PUBLIC manga-manager::os::windows)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_subdirectory(android) # TODO
    target_link_libraries(manga-manager.os PUBLIC manga-manager::os::android)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(apple) # TODO
    target_link_libraries(manga-manager.os PUBLIC manga-manager::os::apple)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(linux)
    target_link_libraries(manga-manager.os PUBLIC manga-manager::os::linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Fuchsia")
    add_subdirectory(fuchsia) # TODO
    target_link_libraries(manga-manager.os PUBLIC manga-manager::os::fuchsia)
else()
    message(NOTICE "Unsupported Platform! Programs may not compile or function correctly. Please make sure you're building on a supported platform or have configured cmake correctly")
endif()
