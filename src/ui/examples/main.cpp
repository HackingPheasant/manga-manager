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

#include "vulkan_defines.h"
#include "window.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

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
//
// Note: You'll see alot of const on vulkan.hpp objects, but since it's a C++
// wrapper around C code the vulkan.hpp objects are const but the stuff they
// wrap are still mutuble, this is known as shallow const (Deep const is where
// an object and any children/objects it contains are immutable).
// For some developers they may wonder why even mark it as const, in this case
// its more like a contract between me (developer) and the code where I
// "promise" that I wont be modifying the vulkan.hpp object *myself* after
// creation. But I am fully aware that the library I am interacting with itself
// may modify the content contained inside.
auto main() -> int try {
    // TODO: Comment this code better 👀
    // Till then, this site gives a decent rundown of
    // what each section of code does
    // https://alaingalvan.medium.com/raw-vulkan-b60d3d946b9c
    const std::string AppName = "Manga Manager";
    const std::string EngineName = "Vulkan.hpp";

    // Shader and Frag code for the renderer
    // Look away, I am commiting c++ crimes
    auto constexpr vertShader = std::to_array<std::uint32_t>({
#include "vulkantut.vert.inc"
    });

    auto constexpr fragShader = std::to_array<std::uint32_t>({
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

    // Create the window
    // This is where we will later render our content to
    //
    // This is kept at the top of the source file to allow x11 (and possibly
    // applicable to wayland) enough time to create and present the window as
    // well as allowing SDL to grab the current and correct values for the
    // window extent (width/height) instead of the just defaulting to the
    // initial window creation values we provided below
    // This allows us to have a correctly sized swapchain and avoid visual
    // issues like below
    //
    // https://twitter.com/HackingPheasant/status/1475730781750247425
    // https://web.archive.org/web/20211229060434/https://twitter.com/HackingPheasant/status/1475730781750247425
    //
    // The other potential option other then doing this was sleeping for a 1
    // second or less which seemed to also allow x11 to do its thing in time
    // But this seemed the lesser of two evils with less potential for going
    // wrong.
    AppWindow window(AppName, 1280, 720);

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
    const std::vector<vk::LayerProperties> instanceLayerProperties = vk::enumerateInstanceLayerProperties();

    // Check and enable (if found) required Layers
    std::vector<char const *> instanceLayers;

    const std::set<std::string> desiredInstanceLayers {
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
    const std::vector<vk::ExtensionProperties> instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();

    // Check and enable (if found) required Extensions
    std::vector<char const *> instanceExtensions;

    // NOTE: Not all platform includes below will be made use of yet or at all in this code
    //  but listing them here for future experiments/use
    const std::set<std::string> desiredInstanceExtensions {
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
    const vk::Instance instance = vk::createInstance(
        vk::InstanceCreateInfo{
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()});

    // Create abstract surface to render to.
    const vk::SurfaceKHR surface = window.createSurface(instance);

    // Enumerate the physicalDevices
    // TODO: Handle more then just the first device we decided to grab,
    // this will also require slight modification on how we handle extensions
    // possibly. Basically we would need to go through  the list of physical
    // devices and select only those capable of runnning the API version we want.
    // Example of how to do it:
    // https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/samples/16_Vulkan_1_1/16_Vulkan_1_1.cpp
    // But for now we will just grab the first device
    const vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();

    // Queue Family
    const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    // No Lint comment is used till the below patch is merged into clang tidy
    // https://reviews.llvm.org/D88833
    assert(queueFamilyProperties.size() < std::numeric_limits<std::uint32_t>::max()); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    // Get the first index into queueFamilyProperties which supports graphics
    auto const graphicsQueueFamilyProperty = std::find_if(
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
    const std::vector<vk::ExtensionProperties> physicalDeviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

    // Check and enable (if found) required Extensions
    std::vector<char const *> physicalDeviceExtensions;

    const std::set<std::string> desiredPhysicalDeviceExtensions{
        "VK_KHR_swapchain"};

    physicalDeviceExtensions.reserve(desiredPhysicalDeviceExtensions.size());

    for (auto const &extension : physicalDeviceExtensionProperties) {
        if (desiredPhysicalDeviceExtensions.contains(extension.extensionName)) {
            physicalDeviceExtensions.push_back(extension.extensionName);
        }
    }

    // Create a Device
    const float queuePriority = 0.0F;

    auto const deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{
        .flags = vk::DeviceQueueCreateFlags(),
        .queueFamilyIndex = static_cast<std::uint32_t>(graphicsQueueFamilyIndex),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    const vk::PhysicalDeviceFeatures physicalDeviceFeatures;

    // Note for vk:DeviceCreateInfo:
    // Even though Device Layers have been deprecated, it is still
    // recommended applications pass an empty list of layers or a
    // list that exactly matches the sequence enabled at
    // *Instance* creation time.
    // I'll be following the latter suggestion and just pass the
    // instanceLayers vector.
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
    const vk::Device device = physicalDevice.createDevice(
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
    const vk::CommandPool commandPool = device.createCommandPool(
        vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlags(),
            .queueFamilyIndex = static_cast<std::uint32_t>(graphicsQueueFamilyIndex)});

    // Allocate a CommandBuffer from the CommandPool
    auto const commandBufferAllocateInfo = vk::CommandBufferAllocateInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};

    const vk::CommandBuffer commandBuffer =
        device.allocateCommandBuffers(commandBufferAllocateInfo).front();

    // Create the queues for later use
    const vk::Queue graphicsQueue = device.getQueue(static_cast<std::uint32_t>(graphicsQueueFamilyIndex), 0);
    const vk::Queue presentQueue = device.getQueue(static_cast<std::uint32_t>(presentQueueFamilyIndex), 0);

    // Get the supported VkFormats
    const std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
    assert(!formats.empty()); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    const vk::Format colorFormat =
        (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    const vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    vk::Extent2D extent;

    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max()) {
        // Get most recent window size before we go using it in our vulkan code
        // Updates the values in window.extent.{width/height}
        window.getWindowSize();

        // If the surface size is undefined, the size is set to the size of the images requested.
        extent.width =
            std::clamp(static_cast<std::uint32_t>(window.extent.width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height =
            std::clamp(static_cast<std::uint32_t>(window.extent.height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    } else {
        // If the surface size is defined, the swap chain size must match
        extent = surfaceCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    const vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

    const vk::SurfaceTransformFlagBitsKHR preTransform =
        (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
            : surfaceCapabilities.currentTransform;

    const vk::CompositeAlphaFlagBitsKHR compositeAlpha =
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
            ? vk::CompositeAlphaFlagBitsKHR::eInherit
            : vk::CompositeAlphaFlagBitsKHR::eOpaque;

    const std::array<std::uint32_t, 2> queueFamilyIndices = {
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

    const vk::SwapchainKHR swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    const std::vector<vk::Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);

    std::vector<vk::ImageView> imageViews;
    imageViews.reserve(swapChainImages.size());

    auto const componentMapping = vk::ComponentMapping{
        .r = vk::ComponentSwizzle::eR,
        .g = vk::ComponentSwizzle::eG,
        .b = vk::ComponentSwizzle::eB,
        .a = vk::ComponentSwizzle::eA};

    auto const subResourceRangeColor = vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    for (auto image : swapChainImages) {
        auto const imageViewCreateInfo = vk::ImageViewCreateInfo{
            .flags = vk::ImageViewCreateFlags(),
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = colorFormat,
            .components = componentMapping,
            .subresourceRange = subResourceRangeColor};

        imageViews.push_back(device.createImageView(imageViewCreateInfo));
    }

    const vk::Format depthFormat = vk::Format::eD16Unorm;
    const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(depthFormat);

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

    const vk::Image depthImage = device.createImage(imageCreateInfo);

    const vk::MemoryRequirements depthImageMemoryRequirements = device.getImageMemoryRequirements(depthImage);

    const std::uint32_t depthMemoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        depthImageMemoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    const vk::DeviceMemory depthMemory = device.allocateMemory(
        vk::MemoryAllocateInfo{
            .allocationSize = depthImageMemoryRequirements.size,
            .memoryTypeIndex = depthMemoryTypeIndex});

    device.bindImageMemory(depthImage, depthMemory, 0);

    auto const subResourceRangeDepth = vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eDepth,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    const vk::ImageView depthImageView = device.createImageView(
        vk::ImageViewCreateInfo{
            .flags = vk::ImageViewCreateFlags(),
            .image = depthImage,
            .viewType = vk::ImageViewType::e2D,
            .format = depthFormat,
            .components = componentMapping,
            .subresourceRange = subResourceRangeDepth});

    // TODO: replace this later with what "Dear ImGUI" outputs.
    const glm::mat4x4 model = glm::mat4x4(1.0F);
    const glm::mat4x4 view =
        glm::lookAt(glm::vec3(-5.0F, 3.0F, -10.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, -1.0F, 0.0F));
    const glm::mat4x4 projection = glm::perspective(glm::radians(45.0F), 1.0F, 0.1F, 100.0F);
    // clang-format off
    const glm::mat4x4 clip = glm::mat4x4( 1.0F,  0.0F, 0.0F, 0.0F,
                                    0.0F, -1.0F, 0.0F, 0.0F,
                                    0.0F,  0.0F, 0.5F, 0.0F,
                                    0.0F,  0.0F, 0.5F, 1.0F );  // vulkan clip space has inverted y and half z !
    // clang-format on
    const glm::mat4x4 mvpc = clip * projection * view * model;

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

    const vk::Buffer uniformBuffer = device.createBuffer(uniformBufferCreateInfo);

    const vk::MemoryRequirements uniformDataMemoryRequirements = device.getBufferMemoryRequirements(uniformBuffer);

    const std::uint32_t uniformMemoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        uniformDataMemoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    const vk::DeviceMemory uniformDataMemory = device.allocateMemory(
        vk::MemoryAllocateInfo{
            .allocationSize = uniformDataMemoryRequirements.size,
            .memoryTypeIndex = uniformMemoryTypeIndex});

    auto const pUniformData = static_cast<std::uint8_t *>(device.mapMemory(uniformDataMemory, 0, uniformDataMemoryRequirements.size));
    std::memcpy(pUniformData, &mvpc, sizeof(mvpc));
    device.unmapMemory(uniformDataMemory);

    device.bindBufferMemory(uniformBuffer, uniformDataMemory, 0);

    // create a DescriptorSetLayout
    auto const descriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr};

    const vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(
        vk::DescriptorSetLayoutCreateInfo{
            .flags = vk::DescriptorSetLayoutCreateFlags(),
            .bindingCount = 1,
            .pBindings = &descriptorSetLayoutBinding});

    // create a PipelineLayout using that DescriptorSetLayout
    const vk::PipelineLayout pipelineLayout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo{
            .flags = vk::PipelineLayoutCreateFlags(),
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr});

    // create a descriptor pool
    const vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = static_cast<std::uint32_t>(swapChainImages.size())};

    const vk::DescriptorPool descriptorPool = device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo{
            .flags = vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
            .maxSets = static_cast<std::uint32_t>(swapChainImages.size()),
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize});

    // allocate a descriptor set
    const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout};

    const vk::DescriptorSet descriptorSet = device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

    const vk::DescriptorBufferInfo descriptorBufferInfo{
        .buffer = uniformBuffer,
        .offset = 0,
        .range = sizeof(glm::mat4x4)};

    const vk::WriteDescriptorSet writeDescriptorSet{
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pImageInfo = nullptr,
        .pBufferInfo = &descriptorBufferInfo,
        .pTexelBufferView = nullptr};

    device.updateDescriptorSets(writeDescriptorSet, nullptr);

    const std::array<vk::AttachmentDescription, 2> attachmentDescriptions = {
        vk::AttachmentDescription{
            .flags = vk::AttachmentDescriptionFlags(),
            .format = colorFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR},
        vk::AttachmentDescription{
            .flags = vk::AttachmentDescriptionFlags(),
            .format = depthFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eDontCare,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal}};

    auto const colorReference = vk::AttachmentReference{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal};

    auto const depthReference = vk::AttachmentReference{
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

    auto const subpass = vk::SubpassDescription{
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

    const vk::RenderPass renderPass = device.createRenderPass(
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
    // https://web.archive.org/web/20211012062754/https://twitter.com/HackingPheasant/status/1447811202839547904
    const vk::ShaderModule vertexShaderModule = device.createShaderModule(
        vk::ShaderModuleCreateInfo{
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = vertShader.size() * sizeof(std::uint32_t),
            .pCode = vertShader.data()});

    const vk::ShaderModule fragmentShaderModule = device.createShaderModule(
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

    const vk::Buffer vertexBuffer = device.createBuffer(vertexBufferCreateInfo);

    // allocate device memory for that buffer
    const vk::MemoryRequirements vertexDataMemoryRequirements = device.getBufferMemoryRequirements(vertexBuffer);

    const std::uint32_t vertexMemoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        vertexDataMemoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    const vk::DeviceMemory vertexDataMemory = device.allocateMemory(
        vk::MemoryAllocateInfo{
            .allocationSize = vertexDataMemoryRequirements.size,
            .memoryTypeIndex = vertexMemoryTypeIndex});

    // copy the vertex and color data into that device memory
    auto const pVertexData = static_cast<std::uint8_t *>(device.mapMemory(vertexDataMemory, 0, vertexDataMemoryRequirements.size));
    std::memcpy(pVertexData, &coloredCubeData, sizeof(coloredCubeData));
    device.unmapMemory(vertexDataMemory);

    // and bind the device memory to the vertex buffer
    device.bindBufferMemory(vertexBuffer, vertexDataMemory, 0);

    const std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
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

    auto const vertexInputBindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(coloredCubeData[0]),
        .inputRate = vk::VertexInputRate::eVertex};

    constexpr std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributeDescriptions = {
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

    auto const pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
        .flags = vk::PipelineVertexInputStateCreateFlags(),
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()};

    auto const pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{
        .flags = vk::PipelineInputAssemblyStateCreateFlags(),
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE};

    auto const pipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{
        .flags = vk::PipelineViewportStateCreateFlags(),
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr};

    auto const pipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{
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

    auto const pipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo{
        .flags = vk::PipelineMultisampleStateCreateFlags(),
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0F,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    auto const stencilOpState = vk::StencilOpState{
        .failOp = vk::StencilOp::eKeep,
        .passOp = vk::StencilOp::eKeep,
        .depthFailOp = vk::StencilOp::eKeep,
        .compareOp = vk::CompareOp::eAlways,
        .compareMask = 0,
        .writeMask = 0,
        .reference = 0};

    auto const pipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo{
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

    const vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    auto const pipelineColorBlendAttachmentState = vk::PipelineColorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eZero,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eZero,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = colorComponentFlags};

    auto const pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo{
        .flags = vk::PipelineColorBlendStateCreateFlags(),
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eNoOp,
        .attachmentCount = 1,
        .pAttachments = &pipelineColorBlendAttachmentState,
        .blendConstants = {{1.0F, 1.0F, 1.0F, 1.0F}}};

    constexpr std::array<vk::DynamicState, 2> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    auto const pipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{
        .flags = vk::PipelineDynamicStateCreateFlags(),
        .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};

    auto const graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo{
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

    const vk::PipelineCache pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

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
    const vk::Semaphore imageAcquiredSemaphore = device.createSemaphore(
        vk::SemaphoreCreateInfo{
            .flags = vk::SemaphoreCreateFlags()});

    const vk::ResultValue<std::uint32_t> currentBuffer = device.acquireNextImageKHR(
        swapChain, FenceTimeout, imageAcquiredSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(currentBuffer.value < framebuffers.size());    // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

    commandBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlags(),
        .pInheritanceInfo = nullptr});

    const std::array<vk::ClearValue, 2> clearValues = {
        vk::ClearValue{
            .color = vk::ClearColorValue{
                .float32 = std::array<float, 4>({{0.2F, 0.2F, 0.2F, 0.2F}})}},
        vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.0F, .stencil = 0}}};

    auto const renderPassBeginInfo = vk::RenderPassBeginInfo{
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

    const vk::Fence drawFence = device.createFence(vk::FenceCreateInfo{
        .flags = vk::FenceCreateFlags()});

    const vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    auto const submitInfo = vk::SubmitInfo{
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
    const vk::Result resultPresentKHR = presentQueue.presentKHR(
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
    // This is purely here so we can actually have a moment to see our glorius rendered cube!
    // ALL HAIL THE CUBE
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
    // instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
    instance.destroy();

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
