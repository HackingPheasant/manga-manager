#ifndef UI_VULKAN_DEFINES_H
#define UI_VULKAN_DEFINES_H
// Whats the point of this?
// Well I needed to set a few preprocessor defines so they can enable some
// more functionality at compile time if the necessary headers are around
// And since I needed to have these availble in more then one source file
// it was just easier to throw it in a header

// Enable C++ Designated Initializers in Vulkan.hpp
// https://github.com/KhronosGroup/Vulkan-Hpp#designated-initializers
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
// With this pull request:
// https://github.com/KhronosGroup/Vulkan-Hpp/pull/1010
// On merge commit (2021-07-06):
// https://github.com/KhronosGroup/Vulkan-Hpp/commit/793f70dbad6b0bf16d071d016e39a988420756d1
// The configuration option has changed. For now we will keep both defines
// since this hasn't propagated out to distro releases yet.
#define VULKAN_HPP_NO_CONSTRUCTORS
// Disable setter member functions (Also introduced in above merge, 793f70db)
#define VULKAN_HPP_NO_SETTERS

// Vulkan System Defines
// It took me too long to figure out where everyone got the correct
// VK_USE_PLATFORM_* define guards. Until I stumbled across
// https://github.com/KhronosGroup/Vulkan-Loader/blob/99c0b1433a0965ba7cd0dbef0e19fce649ddc12e/loader/CMakeLists.txt#L31
// Which made me realise it was defined by the build system and not in one of
// the many vulkan header files I looked through.
// For now I'll try defining the defines here, but if it proves to be
// to brittle I will move it into the build system (move it to
// CMakeLists.txt)
// This may not cover every system it can be used on, but it covers every
// system I run on/want to run on.
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap51.html#boilerplate-wsi-header
// Above link is also used as refernce on creating the below defines

// Android
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

// Fuchsia
#if defined(__Fuchsia__)
#define VK_USE_PLATFORM_FUCHSIA
#endif

// Apple
#if defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define VK_USE_PLATFORM_IOS_MVK
#elif TARGET_OS_MAC
#define VK_USE_PLATFORM_MACOS_MVK
#endif
#endif

// Windows
#if defined(_WIN32) || defined(_WIN64) || defined(__NT__)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Linux and *nix
#if defined(__linux__) || defined(__unix__)
#if __has_include(<wayland-client.h>)
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#if __has_include(<X11/Xlib.h>)
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
#if __has_include(<X11/extensions/Xrandr.h>)
#define VK_USE_PLATFORM_XLIB_XRANDR_EXT
#endif
#endif
#if __has_include(<xcb/xcb.h>)
#define VK_USE_PLATFORM_XCB_KHR
#endif
#if __has_include(<directfb/directfb.h>)
#define VK_USE_PLATFORM_DIRECTFB_EXT
#endif
#endif

// Enable Vulkan debug utilities if we compile as Debug
#ifndef NDEBUG
#define VULKAN_DEBUG
#endif

// Now include offical vulkan headers :)
#include <vulkan/vulkan.hpp>

#endif // UI_VULKAN_DEFINES_H
