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

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

class Render {
};

// TODO Make vk::DebugReportCallbackCreateInfoEXT
// https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/samples/CreateDebugUtilsMessenger/CreateDebugUtilsMessenger.cpp

auto findMemoryType(vk::PhysicalDeviceMemoryProperties const &memoryProperties,
    std::uint32_t typeBits,
    vk::MemoryPropertyFlags requirementsMask) -> std::uint32_t {
    auto typeIndex = std::uint32_t(~0);

    for (std::uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) != 0 &&
            ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)) {
            typeIndex = i;
            break;
        }
        typeBits >>= 1;
    }

    assert(typeIndex != std::uint32_t(~0)); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    return typeIndex;
}

// Quick overview of whats to follow:
// 1. Initalize the Vulkan API
// 2. Create commands
// 3. Initialize resources
// 4. Setup Commands for each command buffer to set the GPU state to render what we want.
// 5. Render
// 6. Cleanup after ourselves
auto main() -> int try {
    //TODO: Comment this code better 👀
    // Till then, this site gives a decent rundown of
    // what each section of code does
    // https://alaingalvan.medium.com/raw-vulkan-b60d3d946b9c
    const std::string AppName = "Manga Manager";
    const std::string EngineName = "Vulkan.hpp";
    int width = 1280;
    int height = 720;

    // Shader and Frag code for the renderer
    // Look away, I am commiting c++ crimes
    constexpr auto vertShader = std::to_array<std::uint32_t>({
#include "vulkantut.vert.inc"
    });

    constexpr auto fragShader = std::to_array<std::uint32_t>({
#include "vulkantut.frag.inc"
    });
    // It's safe to look again.

    // BEGIN Vulkan-HPP Samples stuff
    // It's the colored cube you see on screen.
    struct VertexPC {
        float x, y, z, w; // Position
        float r, g, b, a; // Color
    };

    static const VertexPC coloredCubeData[] =
        {
            // clang-format off
        // red face
        { -1.0F, -1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 0.0F, 1.0F },
        { -1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 0.0F, 1.0F },
        { -1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 0.0F, 1.0F },
        {  1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 0.0F, 1.0F },
        // green face
        { -1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 0.0F, 1.0F },
        { -1.0F,  1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 0.0F, 1.0F },
        { -1.0F,  1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F,  1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 0.0F, 1.0F },
        // blue face
        { -1.0F,  1.0F,  1.0F, 1.0F,    0.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F, -1.0F,  1.0F, 1.0F,    0.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F,  1.0F, -1.0F, 1.0F,    0.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F,  1.0F, -1.0F, 1.0F,    0.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F, -1.0F,  1.0F, 1.0F,    0.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 0.0F, 1.0F, 1.0F },
        // yellow face
        {  1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F,  1.0F, -1.0F, 1.0F,    1.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F,  1.0F, 1.0F,    1.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F,  1.0F, 1.0F,    1.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F,  1.0F, -1.0F, 1.0F,    1.0F, 1.0F, 0.0F, 1.0F },
        {  1.0F, -1.0F, -1.0F, 1.0F,    1.0F, 1.0F, 0.0F, 1.0F },
        // magenta face
        {  1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 1.0F, 1.0F },
        {  1.0F,  1.0F, -1.0F, 1.0F,    1.0F, 0.0F, 1.0F, 1.0F },
        {  1.0F,  1.0F, -1.0F, 1.0F,    1.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F,  1.0F,  1.0F, 1.0F,    1.0F, 0.0F, 1.0F, 1.0F },
        { -1.0F,  1.0F, -1.0F, 1.0F,    1.0F, 0.0F, 1.0F, 1.0F },
        // cyan face
        {  1.0F, -1.0F,  1.0F, 1.0F,    0.0F, 1.0F, 1.0F, 1.0F },
        {  1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 1.0F, 1.0F },
        { -1.0F, -1.0F,  1.0F, 1.0F,    0.0F, 1.0F, 1.0F, 1.0F },
        { -1.0F, -1.0F,  1.0F, 1.0F,    0.0F, 1.0F, 1.0F, 1.0F },
        {  1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 1.0F, 1.0F },
        { -1.0F, -1.0F, -1.0F, 1.0F,    0.0F, 1.0F, 1.0F, 1.0F },
            // clang-format on
    };
    // END Vulkan-HPP Samples stuff

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error("Unable to initialize SDL: " + std::string(SDL_GetError()));
    }

    // Create an application window with the following settings:
    SDL_Window *window = SDL_CreateWindow(
        AppName.c_str(),                                                      // window title
        SDL_WINDOWPOS_CENTERED,                                               // initial x position
        SDL_WINDOWPOS_CENTERED,                                               // initial y position
        width,                                                                // width, in pixels
        height,                                                               // height, in pixels
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI); // flags

    // Check that the window was successfully created
    if (window == nullptr) {
        throw std::runtime_error("Could not create window: " + std::string(SDL_GetError()));
    }

    // Now the large majority of this file will be setting up the necessary boilerplate
    // for the Vulkan render

    // Initialize the vk::ApplicationInfo structure
    // This isn't made use off inside the vulkan code, it's info that is
    // *possibly* used by GPU vendors to give your application special
    // treatment
    auto const applicationInfo = vk::ApplicationInfo{
        .pApplicationName = AppName.c_str(),
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = EngineName.c_str(),
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0)}; // (variant, major, minor, patch)

    // Enumerate the Instance Layers
    std::vector<vk::LayerProperties> instanceLayerProperties = vk::enumerateInstanceLayerProperties();

    // Check and enable (if found) required Layers
    std::vector<char const *> instanceLayers;

    std::set<std::string> desiredInstanceLayers {
#if defined(VULKAN_DEBUG)
        // https://vulkan.lunarg.com/doc/view/latest/linux/khronos_validation_layer.html
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    instanceLayers.reserve(desiredInstanceLayers.size());

    for (auto const &layer : instanceLayerProperties) {
        if (desiredInstanceLayers.contains(layer.layerName)) {
            instanceLayers.push_back(layer.layerName);
        }
    }

    // Enumerate the Instance Extensions
    std::vector<vk::ExtensionProperties> instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();

    // Check and enable (if found) required Extensions
    std::vector<char const *> instanceExtensions;

    // NOTE: Not all platform includes below will be made use of yet or at all in this code
    //  but listing them here for future experiments/use
    std::set<std::string> desiredInstanceExtensions {
        // clang-format off
#if defined(VULKAN_DEBUG)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_debug_utils.html
        "VK_EXT_debug_utils",
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_android_surface.html
        "VK_KHR_android_surface",
#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_FUCHSIA_imagepipe_surface.html
        "VK_FUCHSIA_imagepipe_surface",
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_metal_surface.html
        "VK_EXT_metal_surface",
#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_MVK_ios_surface.html
        "VK_MVK_ios_surface",
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_MVK_macos_surface.html
        "VK_MVK_macos_surface",
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_win32_surface.html
        "VK_KHR_win32_surface",
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_wayland_surface.html
        "VK_KHR_wayland_surface",
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_xlib_surface.html
        "VK_KHR_xlib_surface",
#endif
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_acquire_xlib_display.html
        "VK_EXT_acquire_xlib_display",
        "VK_EXT_direct_mode_display",
        "VK_KHR_display",
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_xcb_surface.html
        "VK_KHR_xcb_surface",
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_directfb_surface.html
        "VK_EXT_directfb_surface",
#endif
        "VK_KHR_surface"
        // clang-format on
    };

    instanceExtensions.reserve(desiredInstanceExtensions.size());

    for (auto const &extension : instanceExtensionProperties) {
        if (desiredInstanceExtensions.contains(extension.extensionName)) {
            instanceExtensions.push_back(extension.extensionName);
        }
    }

    // Create an Instance
    vk::Instance instance = vk::createInstance(
        vk::InstanceCreateInfo{
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()});

    // Create abstract surface to render to.
    vk::SurfaceKHR surface;
    {
        VkSurfaceKHR _surface = nullptr;
        if (SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), &_surface) == 0) {
            throw std::runtime_error("Could not create a Vulkan surface: " + std::string(SDL_GetError()));
        }
        surface = vk::SurfaceKHR(_surface);
    }

    // Enumerate the physicalDevices
    // TODO: Handle more then just the first device we decided to grab,
    // this will also require slight modification on how we handle extensions
    // possibly. Basically we would need to go through  the list of physical
    // devices and select only those capable of runnning the API version we want.
    // Example of how to do it:
    // https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/samples/16_Vulkan_1_1/16_Vulkan_1_1.cpp
    // But for now we will just grab the first device
    vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();

    // Queue Family
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    // No Lint comment is used till the below patch is merged into clang tidy
    // https://reviews.llvm.org/D88833
    assert(queueFamilyProperties.size() < std::numeric_limits<std::uint32_t>::max()); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    // Get the first index into queueFamilyProperties which supports graphics
    auto graphicsQueueFamilyProperty = std::find_if(
        queueFamilyProperties.begin(),
        queueFamilyProperties.end(),
        [](vk::QueueFamilyProperties const &qfp) {
            return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
        });
    auto graphicsQueueFamilyIndex =
        static_cast<std::size_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    assert(graphicsQueueFamilyIndex < queueFamilyProperties.size()); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    // Find Graphics and Present queue family index.
    // Determine a queueFamilyIndex that supports Present
    // First check if the graphicsQueueFamilyIndex is good enough
    std::size_t presentQueueFamilyIndex =
        physicalDevice.getSurfaceSupportKHR(static_cast<std::uint32_t>(graphicsQueueFamilyIndex), surface) != 0
            ? graphicsQueueFamilyIndex
            : queueFamilyProperties.size();

    if (presentQueueFamilyIndex == queueFamilyProperties.size()) {
        // The graphicsQueueFamilyIndex doesn't support present -> look for
        // an other family index that supports both graphics and present
        for (std::size_t i = 0; i < queueFamilyProperties.size(); i++) {
            if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), surface) != 0) {
                graphicsQueueFamilyIndex = static_cast<std::uint32_t>(i);
                presentQueueFamilyIndex = i;
                break;
            }
        }

        if (presentQueueFamilyIndex == queueFamilyProperties.size()) {
            // There is no such thing as a single family index that
            // supports both graphical and present -> look for another
            // family index that supports present
            for (std::size_t i = 0; i < queueFamilyProperties.size(); i++) {
                if (physicalDevice.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), surface) != 0) {
                    presentQueueFamilyIndex = i;
                    break;
                }
            }
        }
    }

    if ((graphicsQueueFamilyIndex == queueFamilyProperties.size()) ||
        (presentQueueFamilyIndex == queueFamilyProperties.size())) {
        throw std::runtime_error("Could not find a queue for graphics or present -> terminating");
    }

    // Enumerate the Device Extensions
    std::vector<vk::ExtensionProperties> physicalDeviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

    // Check and enable (if found) required Extensions
    std::vector<char const *> physicalDeviceExtensions;

    std::set<std::string> desiredPhysicalDeviceExtensions{
        "VK_KHR_swapchain"};

    physicalDeviceExtensions.reserve(desiredPhysicalDeviceExtensions.size());

    for (auto const &extension : physicalDeviceExtensionProperties) {
        if (desiredPhysicalDeviceExtensions.contains(extension.extensionName)) {
            physicalDeviceExtensions.push_back(extension.extensionName);
        }
    }

    // Create a Device
    float queuePriority = 0.0F;

    auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{
        .flags = vk::DeviceQueueCreateFlags(),
        .queueFamilyIndex = static_cast<std::uint32_t>(graphicsQueueFamilyIndex),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    vk::PhysicalDeviceFeatures physicalDeviceFeatures;

    // Note for vk:DeviceCreateInfo:
    // Even though Device Layers have been deprecated, it is still
    // recommended applications pass an empty list of layers or a
    // list that exactly matches the sequence enabled at
    // *Instance* creation time.
    // I'll be following the latter suggestion and just pass the
    // instanceLayers vector.
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
    vk::Device device = physicalDevice.createDevice(
        vk::DeviceCreateInfo{
            .flags = vk::DeviceCreateFlags(),
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(physicalDeviceExtensions.size()),
            .ppEnabledExtensionNames = physicalDeviceExtensions.data(),
            .pEnabledFeatures = &physicalDeviceFeatures});

    // Create a CommandPool to allocate a CommandBuffer from
    vk::CommandPool commandPool = device.createCommandPool(
        vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlags(),
            .queueFamilyIndex = static_cast<std::uint32_t>(graphicsQueueFamilyIndex)});

    // Allocate a CommandBuffer from the CommandPool
    auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};

    vk::CommandBuffer commandBuffer =
        device.allocateCommandBuffers(commandBufferAllocateInfo).front();

    // Create the queues for later use
    vk::Queue graphicsQueue = device.getQueue(static_cast<std::uint32_t>(graphicsQueueFamilyIndex), 0);
    vk::Queue presentQueue = device.getQueue(static_cast<std::uint32_t>(presentQueueFamilyIndex), 0);

    // Get the supported VkFormats
    std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
    assert(!formats.empty()); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    vk::Format colorFormat =
        (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    // Get the size of the windows underlying drawable dimension
    // May differ to SDL_GetWindowSize() if rendering to a high-DPI
    // drawable. e,g, Apples "Retina" screen
    SDL_Vulkan_GetDrawableSize(window, &width, &height);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    vk::Extent2D extent;

    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max()) {
        // If the surface size is undefined, the size is set to the size of the images requested.
        extent.width =
            std::clamp(static_cast<std::uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height =
            std::clamp(static_cast<std::uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    } else {
        // If the surface size is defined, the swap chain size must match
        extent = surfaceCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

    vk::SurfaceTransformFlagBitsKHR preTransform =
        (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
            : surfaceCapabilities.currentTransform;

    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
            ? vk::CompositeAlphaFlagBitsKHR::eInherit
            : vk::CompositeAlphaFlagBitsKHR::eOpaque;

    std::array<std::uint32_t, 2> queueFamilyIndices = {
        static_cast<std::uint32_t>(graphicsQueueFamilyIndex),
        static_cast<std::uint32_t>(presentQueueFamilyIndex)};

    auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR{
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = surface,
        .minImageCount = surfaceCapabilities.minImageCount,
        .imageFormat = colorFormat,
        .imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = preTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = swapchainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr};

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        // If the graphics and present queues are from different queue
        // families, we either have to explicitly transfer ownership of
        // images between the queues, or we have to create the swapchain
        // with imageSharingMode as vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    vk::SwapchainKHR swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    std::vector<vk::Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);

    std::vector<vk::ImageView> imageViews;
    imageViews.reserve(swapChainImages.size());

    auto componentMapping = vk::ComponentMapping{
        .r = vk::ComponentSwizzle::eR,
        .g = vk::ComponentSwizzle::eG,
        .b = vk::ComponentSwizzle::eB,
        .a = vk::ComponentSwizzle::eA};

    auto subResourceRangeColor = vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    for (auto image : swapChainImages) {
        auto imageViewCreateInfo = vk::ImageViewCreateInfo{
            .flags = vk::ImageViewCreateFlags(),
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = colorFormat,
            .components = componentMapping,
            .subresourceRange = subResourceRangeColor};

        imageViews.push_back(device.createImageView(imageViewCreateInfo));
    }

    const vk::Format depthFormat = vk::Format::eD16Unorm;
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(depthFormat);

    vk::ImageTiling tiling;
    if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        tiling = vk::ImageTiling::eLinear;
    } else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        tiling = vk::ImageTiling::eOptimal;
    } else {
        throw std::runtime_error("DepthStencilAttachment is not supported for D16Unorm depth format.");
    }

    auto imageCreateInfo = vk::ImageCreateInfo{
        .flags = vk::ImageCreateFlags(),
        .imageType = vk::ImageType::e2D,
        .format = depthFormat,
        .extent = vk::Extent3D{
            .width = extent.width,
            .height = extent.height,
            .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = vk::ImageLayout::eUndefined};

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        // Done as the same reason described in swapChainCreateInfo comment
        imageCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    vk::Image depthImage = device.createImage(imageCreateInfo);

    vk::MemoryRequirements depthImageMemoryRequirements = device.getImageMemoryRequirements(depthImage);

    std::uint32_t depthMemoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        depthImageMemoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::DeviceMemory depthMemory = device.allocateMemory(
        vk::MemoryAllocateInfo{
            .allocationSize = depthImageMemoryRequirements.size,
            .memoryTypeIndex = depthMemoryTypeIndex});

    device.bindImageMemory(depthImage, depthMemory, 0);

    auto subResourceRangeDepth = vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eDepth,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    vk::ImageView depthImageView = device.createImageView(
        vk::ImageViewCreateInfo{
            .flags = vk::ImageViewCreateFlags(),
            .image = depthImage,
            .viewType = vk::ImageViewType::e2D,
            .format = depthFormat,
            .components = componentMapping,
            .subresourceRange = subResourceRangeDepth});

    // TODO: replace this later with what "Dear ImGUI" outputs.
    glm::mat4x4 model = glm::mat4x4(1.0F);
    glm::mat4x4 view =
        glm::lookAt(glm::vec3(-5.0F, 3.0F, -10.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, -1.0F, 0.0F));
    glm::mat4x4 projection = glm::perspective(glm::radians(45.0F), 1.0F, 0.1F, 100.0F);
    // clang-format off
    glm::mat4x4 clip = glm::mat4x4( 1.0F,  0.0F, 0.0F, 0.0F,
                                    0.0F, -1.0F, 0.0F, 0.0F,
                                    0.0F,  0.0F, 0.5F, 0.0F,
                                    0.0F,  0.0F, 0.5F, 1.0F );  // vulkan clip space has inverted y and half z !
    // clang-format on
    glm::mat4x4 mvpc = clip * projection * view * model;

    auto uniformBufferCreateInfo = vk::BufferCreateInfo{
        .flags = vk::BufferCreateFlags(),
        .size = sizeof(mvpc),
        .usage = vk::BufferUsageFlagBits::eUniformBuffer,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        // Done as the same reason described in swapChainCreateInfo comment
        uniformBufferCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
        uniformBufferCreateInfo.queueFamilyIndexCount = 2;
        uniformBufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    vk::Buffer uniformBuffer = device.createBuffer(uniformBufferCreateInfo);

    vk::MemoryRequirements uniformDataMemoryRequirements = device.getBufferMemoryRequirements(uniformBuffer);

    std::uint32_t uniformMemoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        uniformDataMemoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::DeviceMemory uniformDataMemory = device.allocateMemory(
        vk::MemoryAllocateInfo{
            .allocationSize = uniformDataMemoryRequirements.size,
            .memoryTypeIndex = uniformMemoryTypeIndex});

    std::uint8_t *pUniformData = static_cast<std::uint8_t *>(device.mapMemory(uniformDataMemory, 0, uniformDataMemoryRequirements.size));
    std::memcpy(pUniformData, &mvpc, sizeof(mvpc));
    device.unmapMemory(uniformDataMemory);

    device.bindBufferMemory(uniformBuffer, uniformDataMemory, 0);

    // create a DescriptorSetLayout
    auto descriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr};

    vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(
        vk::DescriptorSetLayoutCreateInfo{
            .flags = vk::DescriptorSetLayoutCreateFlags(),
            .bindingCount = 1,
            .pBindings = &descriptorSetLayoutBinding});

    // create a PipelineLayout using that DescriptorSetLayout
    vk::PipelineLayout pipelineLayout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo{
            .flags = vk::PipelineLayoutCreateFlags(),
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr});

    // create a descriptor pool
    vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = static_cast<std::uint32_t>(swapChainImages.size())};

    vk::DescriptorPool descriptorPool = device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo{
            .flags = vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
            .maxSets = static_cast<std::uint32_t>(swapChainImages.size()),
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize});

    // allocate a descriptor set
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout};

    vk::DescriptorSet descriptorSet = device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

    vk::DescriptorBufferInfo descriptorBufferInfo{
        .buffer = uniformBuffer,
        .offset = 0,
        .range = sizeof(glm::mat4x4)};

    vk::WriteDescriptorSet writeDescriptorSet{
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pImageInfo = nullptr,
        .pBufferInfo = &descriptorBufferInfo,
        .pTexelBufferView = nullptr};

    device.updateDescriptorSets(writeDescriptorSet, nullptr);

    std::array<vk::AttachmentDescription, 2> attachmentDescriptions;

    attachmentDescriptions[0] = vk::AttachmentDescription{
        .flags = vk::AttachmentDescriptionFlags(),
        .format = colorFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR};

    attachmentDescriptions[1] = vk::AttachmentDescription{
        .flags = vk::AttachmentDescriptionFlags(),
        .format = depthFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

    auto colorReference = vk::AttachmentReference{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal};

    auto depthReference = vk::AttachmentReference{
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

    auto subpass = vk::SubpassDescription{
        .flags = vk::SubpassDescriptionFlags(),
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr};

    vk::RenderPass renderPass = device.createRenderPass(
        vk::RenderPassCreateInfo{
            .flags = vk::RenderPassCreateFlags(),
            .attachmentCount = static_cast<std::uint32_t>(attachmentDescriptions.size()),
            .pAttachments = attachmentDescriptions.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 0,
            .pDependencies = nullptr});

    // Why do I times the size by 4?
    //
    // Because .size() will tell you how many elements in the array
    // Not the size of the array
    //
    // So for example .size returns 287 elements, sizeof unint32_t is 4
    //     287 * 4 = 1148
    // That is the size we need to pass to codeSize
    //
    // After I realised the problem finding a related github issue was
    // super quick, explaining the issue I took weeks to debug. I wish
    // I saw this earlier, and not after I spent weeks trying to figure
    // out the issue and a solution
    // https://github.com/KhronosGroup/Vulkan-Hpp/issues/857#issuecomment-762706418
    // My twitter rant about it can be found here
    // https://twitter.com/HackingPheasant/status/1447811202839547904
    vk::ShaderModule vertexShaderModule = device.createShaderModule(
        vk::ShaderModuleCreateInfo{
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = vertShader.size() * sizeof(std::uint32_t),
            .pCode = vertShader.data()});

    vk::ShaderModule fragmentShaderModule = device.createShaderModule(
        vk::ShaderModuleCreateInfo{
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = fragShader.size() * sizeof(std::uint32_t),
            .pCode = fragShader.data()});

    std::array<vk::ImageView, 2> attachments;
    attachments[1] = depthImageView;

    auto framebufferCreateInfo = vk::FramebufferCreateInfo{
        .flags = vk::FramebufferCreateFlags(),
        .renderPass = renderPass,
        .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1};

    std::vector<vk::Framebuffer> framebuffers;
    framebuffers.reserve(imageViews.size());

    for (auto const &imageView : imageViews) {
        attachments[0] = imageView;
        framebuffers.push_back(device.createFramebuffer(framebufferCreateInfo));
    }

    // create a vertex buffer for some vertex and color data
    auto vertexBufferCreateInfo = vk::BufferCreateInfo{
        .flags = vk::BufferCreateFlags(),
        .size = sizeof(coloredCubeData),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        // Done as the same reason described in swapChainCreateInfo comment
        vertexBufferCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
        vertexBufferCreateInfo.queueFamilyIndexCount = 2;
        vertexBufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    vk::Buffer vertexBuffer = device.createBuffer(vertexBufferCreateInfo);

    // allocate device memory for that buffer
    vk::MemoryRequirements vertexDataMemoryRequirements = device.getBufferMemoryRequirements(vertexBuffer);

    std::uint32_t vertexMemoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        vertexDataMemoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::DeviceMemory vertexDataMemory = device.allocateMemory(
        vk::MemoryAllocateInfo{
            .allocationSize = vertexDataMemoryRequirements.size,
            .memoryTypeIndex = vertexMemoryTypeIndex});

    // copy the vertex and color data into that device memory
    std::uint8_t *pVertexData = static_cast<std::uint8_t *>(device.mapMemory(vertexDataMemory, 0, vertexDataMemoryRequirements.size));
    std::memcpy(pVertexData, &coloredCubeData, sizeof(coloredCubeData));
    device.unmapMemory(vertexDataMemory);

    // and bind the device memory to the vertex buffer
    device.bindBufferMemory(vertexBuffer, vertexDataMemory, 0);

    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
        vk::PipelineShaderStageCreateInfo{
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertexShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr},
        vk::PipelineShaderStageCreateInfo{
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragmentShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr}};

    auto vertexInputBindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(coloredCubeData[0]),
        .inputRate = vk::VertexInputRate::eVertex};

    std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributeDescriptions = {
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = 0},
        vk::VertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = 16}};

    auto pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
        .flags = vk::PipelineVertexInputStateCreateFlags(),
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()};

    auto pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{
        .flags = vk::PipelineInputAssemblyStateCreateFlags(),
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE};

    auto pipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{
        .flags = vk::PipelineViewportStateCreateFlags(),
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr};

    auto pipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{
        .flags = vk::PipelineRasterizationStateCreateFlags(),
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0F,
        .depthBiasClamp = 0.0F,
        .depthBiasSlopeFactor = 0.0F,
        .lineWidth = 1.0F};

    auto pipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo{
        .flags = vk::PipelineMultisampleStateCreateFlags(),
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0F,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    auto stencilOpState = vk::StencilOpState{
        .failOp = vk::StencilOp::eKeep,
        .passOp = vk::StencilOp::eKeep,
        .depthFailOp = vk::StencilOp::eKeep,
        .compareOp = vk::CompareOp::eAlways,
        .compareMask = 0,
        .writeMask = 0,
        .reference = 0};

    auto pipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo{
        .flags = vk::PipelineDepthStencilStateCreateFlags(),
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = vk::CompareOp::eLessOrEqual,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = stencilOpState,
        .back = stencilOpState,
        .minDepthBounds = VK_FALSE,
        .maxDepthBounds = VK_FALSE};

    vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    auto pipelineColorBlendAttachmentState = vk::PipelineColorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eZero,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eZero,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = colorComponentFlags};

    auto pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo{
        .flags = vk::PipelineColorBlendStateCreateFlags(),
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eNoOp,
        .attachmentCount = 1,
        .pAttachments = &pipelineColorBlendAttachmentState,
        .blendConstants = {{1.0F, 1.0F, 1.0F, 1.0F}}};

    std::array<vk::DynamicState, 2> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    auto pipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{
        .flags = vk::PipelineDynamicStateCreateFlags(),
        .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};

    auto graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo{
        .flags = vk::PipelineCreateFlags(),
        .stageCount = static_cast<std::uint32_t>(pipelineShaderStageCreateInfos.size()),
        .pStages = pipelineShaderStageCreateInfos.data(),
        .pVertexInputState = &pipelineVertexInputStateCreateInfo,
        .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &pipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = &pipelineDepthStencilStateCreateInfo,
        .pColorBlendState = &pipelineColorBlendStateCreateInfo,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = 0};

    vk::PipelineCache pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

    vk::Result resultPipeline;
    vk::Pipeline graphicsPipeline;

    std::tie(resultPipeline, graphicsPipeline) = device.createGraphicsPipeline(pipelineCache, graphicsPipelineCreateInfo);

    switch (resultPipeline) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::ePipelineCompileRequiredEXT:
        // Do something meaningfull here
        std::cout << "vk::Pipeline returned vk::Result::ePipelineCompileRequiredEXT !\n";
        break;
    default:
        // should never happen
        assert(false); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    }

    // FenceTimeout specifies how long the function waits, in nanoseconds, if no image is available
    // https://khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkAcquireNextImageKHR.html
    const std::uint64_t FenceTimeout = 100000000; // 100000000 nanoseconds = 0.1 seconds

    // Get the index of the next available swapchain image:
    vk::Semaphore imageAcquiredSemaphore = device.createSemaphore(
        vk::SemaphoreCreateInfo{
            .flags = vk::SemaphoreCreateFlags()});

    vk::ResultValue<std::uint32_t> currentBuffer = device.acquireNextImageKHR(
        swapChain, FenceTimeout, imageAcquiredSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(currentBuffer.value < framebuffers.size());    // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    commandBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlags(),
        .pInheritanceInfo = nullptr});

    std::array<vk::ClearValue, 2> clearValues = {
        vk::ClearValue{
            .color = vk::ClearColorValue{
                .float32 = std::array<float, 4>({{0.2F, 0.2F, 0.2F, 0.2F}})}},
        vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.0F, .stencil = 0}}};

    auto renderPassBeginInfo = vk::RenderPassBeginInfo{
        .renderPass = renderPass,
        .framebuffer = framebuffers[currentBuffer.value],
        .renderArea = vk::Rect2D{
            .offset = vk::Offset2D{
                .x = 0,
                .y = 0},
            .extent = extent},
        .clearValueCount = clearValues.size(),
        .pClearValues = clearValues.data()};

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);

    commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});
    commandBuffer.setViewport(0, vk::Viewport{
                                     .x = 0.0F,
                                     .y = 0.0F,
                                     .width = static_cast<float>(extent.width),
                                     .height = static_cast<float>(extent.height),
                                     .minDepth = 0.0F,
                                     .maxDepth = 1.0F});
    commandBuffer.setScissor(0, vk::Rect2D{
                                    .offset = vk::Offset2D{
                                        .x = 0,
                                        .y = 0},
                                    .extent = extent});

    commandBuffer.draw(12 * 3, 1, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();

    vk::Fence drawFence = device.createFence(vk::FenceCreateInfo{
        .flags = vk::FenceCreateFlags()});

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    auto submitInfo = vk::SubmitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAcquiredSemaphore,
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr};

    graphicsQueue.submit(submitInfo, drawFence);

    /* Make sure command buffer is finished before presenting */
    while (vk::Result::eTimeout == device.waitForFences(drawFence, VK_TRUE, FenceTimeout)) {
        /* do nothing */
    }

    /* Now present the image in the window */
    vk::Result resultPresentKHR = presentQueue.presentKHR(
        vk::PresentInfoKHR{
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &currentBuffer.value,
            .pResults = nullptr});

    switch (resultPresentKHR) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR:
        // Do something meaningfull here
        std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
        break;
    default:
        // an unexpected result is returned !
        assert(false); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    device.waitIdle();

    // Clean up the Vulkan mess
    device.destroyFence(drawFence);
    device.destroySemaphore(imageAcquiredSemaphore);
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineCache(pipelineCache);
    device.freeMemory(vertexDataMemory);
    device.destroyBuffer(vertexBuffer);
    for (auto const &framebuffer : framebuffers) {
        device.destroyFramebuffer(framebuffer);
    }
    device.destroyShaderModule(fragmentShaderModule);
    device.destroyShaderModule(vertexShaderModule);
    device.destroyRenderPass(renderPass);
    device.freeDescriptorSets(descriptorPool, descriptorSet);
    device.destroyDescriptorPool(descriptorPool);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyDescriptorSetLayout(descriptorSetLayout);
    device.freeMemory(uniformDataMemory);
    device.destroyBuffer(uniformBuffer);
    device.destroyImageView(depthImageView);
    device.freeMemory(depthMemory);
    device.destroyImage(depthImage);
    for (const auto &imageView : imageViews) {
        device.destroyImageView(imageView);
    }
    device.destroySwapchainKHR(swapChain);
    // Freeing the commandBuffer is optional, as it will be automatically
    // freed when the corresponding CommandPool is destroyed.
    device.freeCommandBuffers(commandPool, commandBuffer);
    device.destroyCommandPool(commandPool);
    device.destroy();
    instance.destroySurfaceKHR(surface);
#if defined(VULKAN_DEBUG)
    //instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
    instance.destroy();

    // Clean up the SDL2 mess
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
} catch (vk::SystemError &err) {
    std::cerr << "vk::SystemError: " << err.what() << std::endl;
    return -1;
} catch (std::exception &err) {
    std::cerr << "std::exception: " << err.what() << std::endl;
    return -1;
} catch (...) {
    std::cerr << "Unknown error\n";
    return -1;
}
