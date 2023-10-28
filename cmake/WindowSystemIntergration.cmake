# Window System Intergration (WSI) options
# Based on
# https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#boilerplate-wsi-header-table

# Building on linux isn't as straight foward as other operating systems
# This is becase we have to support more then one Display Server/Protocol
# Thus we provide some toggle options for linux users so they can choose
# what display server support they want compiled in

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(BUILD_LINUX_X11_SUPPORT "Build with X11 support" ON)
    option(BUILD_LINUX_WAYLAND_SUPPORT "Build with Wayland support" ON)
endif()

add_library(platform_wsi_defines INTERFACE)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_WIN32_KHR WSI_WIN32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_ANDROID_KHR WSI_ANDROID)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_METAL_EXT WSI_APPLE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(BUILD_LINUX_X11_SUPPORT)
        # We will only support XCB and not Xlib, because under the hood, libX11
        # is already making use of XCB
        # https://www.x.org/wiki/guide/xlib-and-xcb/#index2h2
        target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_XCB_KHR WSI_X11)
    endif()
    if(BUILD_LINUX_WAYLAND_SUPPORT)
        target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_WAYLAND_KHR WSI_WAYLAND)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Fuchsia")
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_FUCHSIA WSI_FUCHSIA)
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()
