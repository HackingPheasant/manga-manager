# Window System Intergration (WSI) options
# Based on
# https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#boilerplate-wsi-header-table

# Building on linux isn't as straight foward as other operating systems
# This is becase we have to support more then one Display Server/Protocol
# Thus we provide some toggle options for linux users so they can choose
# what display server support they want compiled in

if(UNIX AND NOT APPLE) # i.e.: Linux
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
endif()

add_library(platform_wsi_defines INTERFACE)

if(WIN32)
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX AND NOT APPLE) # i.e.: Linux
    if(BUILD_WSI_XCB_SUPPORT)
        target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_XCB_KHR)
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
elseif(FUCHSIA)
    target_compile_definitions(platform_wsi_defines INTERFACE VK_USE_PLATFORM_FUCHSIA)
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()
