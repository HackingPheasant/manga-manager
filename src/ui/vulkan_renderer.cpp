// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#include <cassert>
#include <cstdint>
#include <iterator>
#include <print>
#include <set>
#include <tuple>
#include <vector>

#include "vulkan_renderer.hpp"

// TODO Still haven't gotten this working correctly yet...
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1 && !defined(VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE)
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

#if defined(VULKAN_DEBUG)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables, readability-identifier-naming,
// modernize-use-trailing-return-type, cppcoreguidelines-pro-bounds-pointer-arithmetic)
PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pMessenger) {
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator) {
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

// Wondering what VKAPI_ATTR and VKAPI_CALL are?
// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#boilerplate-platform-specific-calling-conventions
// TL;DR: Macros to tweak calling convetions depending on compiler and
// language version used
VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types, VkDebugUtilsMessengerCallbackDataEXT const *callback_data,
    void * /*pUserData*/) {
    // NOTE:
    // Function body taken from
    // https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/RAII_Samples/EnableValidationWithCallback/EnableValidationWithCallback.cpp
    // TODO:
    // - Make my own one instead of copy+paste. But for getting a basic thing
    // up and running, this is good enough for me.
    std::string message;

    message += vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)) + ": " +
               vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)) + ":\n";
    message += std::string("\t") + "messageIDName   = <" + callback_data->pMessageIdName + ">\n";
    message += std::string("\t") + "messageIdNumber = " + std::to_string(callback_data->messageIdNumber) + "\n";
    message += std::string("\t") + "message = <" + callback_data->pMessage + ">\n";

    if (0 < callback_data->queueLabelCount) {
        message += std::string("\t") + "Queue Labels:\n";
        for (uint32_t i = 0; i < callback_data->queueLabelCount; i++) {
            message += std::string("\t\t") + "labelName = <" + callback_data->pQueueLabels[i].pLabelName + ">\n";
        }
    }
    if (0 < callback_data->cmdBufLabelCount) {
        message += std::string("\t") + "Command Buffer Labels:\n";
        for (uint32_t i = 0; i < callback_data->cmdBufLabelCount; i++) {
            message += std::string("\t\t") + "labelName = <" + callback_data->pCmdBufLabels[i].pLabelName + ">\n";
        }
    }
    if (0 < callback_data->objectCount) {
        for (uint32_t i = 0; i < callback_data->objectCount; i++) {
            message += std::string("\t") + "Object " + std::to_string(i) + "\n";
            message += std::string("\t\t") + "objectType   = " +
                       vk::to_string(static_cast<vk::ObjectType>(callback_data->pObjects[i].objectType)) + "\n";
            message += std::string("\t\t") +
                       "objectHandle = " + std::to_string(callback_data->pObjects[i].objectHandle) + "\n";
            if (callback_data->pObjects[i].pObjectName != nullptr) {
                message += std::string("\t\t") + "objectName   = <" + callback_data->pObjects[i].pObjectName + ">\n";
            }
        }
    }

    std::println("{}", message);

    return VK_FALSE;
}
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables, readability-identifier-naming,
// modernize-use-trailing-return-type, cppcoreguidelines-pro-bounds-pointer-arithmetic)
#endif

auto VulkanRender::findMemoryType(vk::PhysicalDeviceMemoryProperties const &memory_properties, std::uint32_t type_bits,
    vk::MemoryPropertyFlags requirements_mask) -> std::uint32_t {
    auto type_index = static_cast<std::uint32_t>(~0);
    const auto memory_count = memory_properties.memoryTypeCount;

    for (std::uint32_t memory_index = 0; memory_index < memory_count; memory_index++) {
        if ((type_bits & 1) != 0 &&
            ((memory_properties.memoryTypes[memory_index].propertyFlags & requirements_mask) == requirements_mask)) {
            type_index = memory_index;
            break;
        }
        type_bits >>= 1;
    }

    assert(type_index != std::uint32_t(~0));
    return type_index;
}

auto VulkanRender::enumerateExtensions(std::vector<vk::ExtensionProperties> const &extension_properties,
    std::set<std::string> const &desired_extensions) -> std::vector<char const *> {
    std::vector<char const *> extensions;

    // Check and enable (if found) required extensions
    extensions.reserve(desired_extensions.size());

    for (auto const &extension : extension_properties) {
        if (desired_extensions.contains(extension.extensionName)) {
            extensions.push_back(extension.extensionName);
        }
    }

    return extensions;
}

auto VulkanRender::enumerateLayers(std::vector<vk::LayerProperties> const &layer_properties,
    std::set<std::string> const &desired_layers) -> std::vector<char const *> {
    std::vector<char const *> layers;

    // Check and enable (if found) required layers
    layers.reserve(desired_layers.size());

    for (auto const &layer : layer_properties) {
        if (desired_layers.contains(layer.layerName)) {
            layers.push_back(layer.layerName);
        }
    }
    return layers;
}

auto VulkanRender::getQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &queue_family_properties,
    vk::QueueFlagBits queue_flag) -> std::uint32_t {
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queue_flag & vk::QueueFlagBits::eCompute) {
        // TODO: Test the iterator ("it") part of the range based for loop
        // actually works. Either via looking at the variable in GDB or
        // testing on a device that has more then one queuefamily available
        for (auto it = queue_family_properties.begin(); auto const &qfp : queue_family_properties) {
            if ((qfp.queueFlags & queue_flag) && !(qfp.queueFlags & vk::QueueFlagBits::eGraphics)) {
                return static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(), it));
            }

            it++;
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics
    // and compute
    if (queue_flag & vk::QueueFlagBits::eTransfer) {
        for (auto it = queue_family_properties.begin(); auto const &qfp : queue_family_properties) {
            if ((qfp.queueFlags & queue_flag) && !(qfp.queueFlags & vk::QueueFlagBits::eGraphics) &&
                !(qfp.queueFlags & vk::QueueFlagBits::eCompute)) {
                return static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(), it));
            }

            it++;
        }
    }

    // For other queue types or if no separate compute queue is present, return
    // the first one to support the requested flags
    for (auto it = queue_family_properties.begin(); auto const &qfp : queue_family_properties) {
        if (qfp.queueFlags & queue_flag) {
            return static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(), it));
        }

        it++;
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

auto VulkanRender::getGraphicsAndPresentQueueFamilyIndex(
    std::vector<vk::QueueFamilyProperties> const &queue_family_properties,
    std::uint32_t graphics_queue_family_index) -> std::array<std::uint32_t, 2> {
    std::array<std::uint32_t, 2> queue_family_indices{};

    if (physical_device.getSurfaceSupportKHR(graphics_queue_family_index, *surface) != 0U) {
        queue_family_indices.at(0) = graphics_queue_family_index;
        queue_family_indices.at(1) = graphics_queue_family_index;
        return queue_family_indices;
        // The first graphics_queue_family_index does also support present
    }

    // The graphics_queue_family_index doesn't support present -> look for another
    // family index that supports both graphics and present
    for (auto it = queue_family_properties.begin(); auto const &qfp : queue_family_properties) {
        auto i = static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(), it));
        if ((qfp.queueFlags & vk::QueueFlagBits::eGraphics) &&
            (physical_device.getSurfaceSupportKHR(i, *surface) != 0U)) {
            queue_family_indices.at(0) = i;
            queue_family_indices.at(1) = i;
            return queue_family_indices;
        }

        it++;
    }

    // There's nothing like a single family index that supports both grahics
    // and present -> look for another family index that supports present
    for (auto it = queue_family_properties.begin(); [[maybe_unused]] auto const &qfp : queue_family_properties) {
        auto i = static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(), it));
        if (physical_device.getSurfaceSupportKHR(i, *surface) != 0U) {
            queue_family_indices.at(0) = graphics_queue_family_index;
            queue_family_indices.at(1) = i;
            return queue_family_indices;
        }

        it++;
    }

    throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
}

VulkanRender::VulkanRender(const std::string &app_name) {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // Initialize minimal set of function pointers
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

    // This isn't actually used inside any vulkan code, it's info that is
    // possibly used by GPU vendors to give your application special treatment
    auto application_info = vk::ApplicationInfo{.pApplicationName = app_name.c_str(),
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_3};

    // I'd prefer to pass "context.enumerate...Properties()" directly to the
    // function(s), but this causes a heap-use-after-free according to
    // AddressSanitizer. So...
    // TODO Check what the underlying cause for the heap-use-after-free
    // if doing it my preferred way
    auto instance_layer_properties = context.enumerateInstanceLayerProperties();
    auto instance_layers = enumerateLayers(instance_layer_properties, desired_instance_layers);

    auto instance_extension_properties = context.enumerateInstanceExtensionProperties();
    auto instance_extensions = enumerateExtensions(instance_extension_properties, desired_instance_extensions);

    instance =
        vk::raii::Instance(context, vk::InstanceCreateInfo{.flags = vk::InstanceCreateFlags(),
                                        .pApplicationInfo = &application_info,
                                        .enabledLayerCount = static_cast<std::uint32_t>(instance_layers.size()),
                                        .ppEnabledLayerNames = instance_layers.data(),
                                        .enabledExtensionCount = static_cast<std::uint32_t>(instance_extensions.size()),
                                        .ppEnabledExtensionNames = instance_extensions.data()});

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // Initialize function pointers for instance
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
#endif

#if defined(VULKAN_DEBUG)
    // Enable validation with debug callback (if program is compiled as debug)
    pfnVkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    pfnVkDestroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    if ((pfnVkCreateDebugUtilsMessengerEXT == nullptr) || (pfnVkDestroyDebugUtilsMessengerEXT == nullptr)) {
        throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT "
                                 "and pfnVkDestroyDebugUtilsMessengerEXT function.");
    }

    debug_utils_messenger = vk::raii::DebugUtilsMessengerEXT(instance,
        vk::DebugUtilsMessengerCreateInfoEXT{
            .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
            // Enabled all severitys and all types mainly for inital setup and
            // debug puposes
            .messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
            // vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
            //  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = &debugCallback,
            .pUserData = nullptr // Optional
        });
#endif
}

// TODO Learn to call stuff like this prior to surface creation
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceWaylandPresentationSupportKHR.html
//

auto VulkanRender::selectPhysicalDevice() -> VulkanRender & {
    // Enumerate the physicalDevices
    vk::raii::PhysicalDevices physical_devices(instance);

    if (physical_devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan Support.");
    }

    // Then go through and select the preferred device
    // e.g. Prefering a dedicated GPU over an intergrated GPU
    // NOTE: There could be a lot more here, but for now this is good enough
    for (auto const &pds : physical_devices) {
        if (pds.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            physical_device = pds;
        }
    }

    if (!*physical_device) {
        // Otherwise just default to the first available device
        physical_device = physical_devices.front();
    }

    vk::PhysicalDeviceProperties device_properties = physical_device.getProperties();
    std::println("Device Name    : {}", std::string(device_properties.deviceName));
    std::println("Device Type    : {}", vk::to_string(device_properties.deviceType));
    std::println("Vulkan Version : {}.{}.{}", VK_VERSION_MAJOR(device_properties.apiVersion),
        VK_VERSION_MINOR(device_properties.apiVersion), VK_VERSION_PATCH(device_properties.apiVersion));
    // Example of accessing the limits struct
    // std::cout << "Max Compute Shared Memory Size: " <<
    // device_properties.limits.maxComputeSharedMemorySize / 1024 << " KB" <<
    // std::endl;

    // Get Physical Device Features
    // We can see whats available and then choose to enable them in our
    // enable_device_features virable, which is used in device creation
    // At the moment, I don't need anything specific so this is more
    // done to silence a vulkan best practice warning
    // TODO See if we actual use this first.......
    [[maybe_unused]] auto features2 = physical_device.getFeatures2();

    // Notes for vulkan terminology (to better understand the next section of
    // code):
    // - A queuefamily descibes what "type" a queue is
    // - A queue is what you submit a command buffer too
    // - A Command buffer (allocated from a command pool)
    //   is where you record all your commands (such as drawing and memory
    //   transfers, etc) you want to be excuted. So essentially command
    //   buffers are a unit of work that created by the CPU to then get excuted
    //   by the GPU
    //
    // Brief Queue family related info overview
    // - Gaphics: Renders an image (creating graphics pipelines and drawing)
    //     - Present: Present images to the surface we created
    // - Compute: Offers computation capabilities
    // display what we have rendered)
    // - Transfer: Used for very fast memory-copying operations
    // - Sparse: Relaxs several restrions around memory and such, allowing for
    // spare resources, such as mega-textures
    // Other queue family related bits and info (such as video encode bits) can
    // be found at:
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#VkQueueFamilyProperties

    const std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();
    assert(queue_family_properties.size() < std::numeric_limits<std::uint32_t>::max());

    if (queue_family_properties.empty()) {
        throw std::runtime_error("No queue family found.");
    }

    auto graphics_queue_family_index = getQueueFamilyIndex(queue_family_properties, vk::QueueFlagBits::eGraphics);
    assert(graphics_queue_family_index < queue_family_properties.size());

    graphics_and_present_queue_family_index =
        getGraphicsAndPresentQueueFamilyIndex(queue_family_properties, graphics_queue_family_index);

    return *this;
}

auto VulkanRender::initDevice() -> VulkanRender & {
    auto device_extension_properties = physical_device.enumerateDeviceExtensionProperties();
    auto device_extensions = enumerateExtensions(device_extension_properties, desired_device_extensions);

    // Create a Device
    const float queue_priority = 0.0F;

    auto device_queue_create_info = vk::DeviceQueueCreateInfo{.flags = vk::DeviceQueueCreateFlags(),
        .queueFamilyIndex = graphics_and_present_queue_family_index.at(0),
        .queueCount = 1,
        .pQueuePriorities = &queue_priority};

    // Note for vk:DeviceCreateInfo:
    // Even though Device Layers have been deprecated, it is still
    // recommended applications pass an empty list of layers or a
    // list that exactly matches the sequence enabled at
    // *Instance* creation time.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
    device = vk::raii::Device(
        physical_device, vk::DeviceCreateInfo{.flags = vk::DeviceCreateFlags(),
                             .queueCreateInfoCount = 1,
                             .pQueueCreateInfos = &device_queue_create_info,
                             .enabledLayerCount = 0,
                             .ppEnabledLayerNames = nullptr,
                             .enabledExtensionCount = static_cast<std::uint32_t>(device_extensions.size()),
                             .ppEnabledExtensionNames = device_extensions.data(),
                             .pEnabledFeatures = &enable_device_features});

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // Optional function pointer specialization for device
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
#endif

    // Create Command Buffers and Command Pools
    // Command Pools allocate Command Buffers, which is where commands for
    // GPU's get recoreded to. The GPU does not excute anything until the
    // command buffer is submitted
    //
    // Special Vulkan RAII Note:
    // Due to the raii-approach, the usual implicit actions done in the pure
    // C fuctions way does not mesh well with the raii-approach. So, to handle
    // it correctly you should explicitly destroy vk::raii::CommandBuffers
    // before destroying the related vk::raii::CommandPool
    command_pool =
        vk::raii::CommandPool(device, vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlags(),
                                          .queueFamilyIndex = graphics_and_present_queue_family_index.at(0)});

    // The commandBufferCount of the vk::CommandBufferAllocateInfo struct
    // controls how many elements are in the
    // std::vector(vk::raii::CommandBuffers).
    // At the moment, we only intended to make use of a singular command buffer
    command_buffers = vk::raii::CommandBuffers(
        device, vk::CommandBufferAllocateInfo{
                    .commandPool = *command_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1});

    command_buffer = std::move(command_buffers.front());

    // Create the queues for later use
    graphics_queue = vk::raii::Queue(device, graphics_and_present_queue_family_index.at(0), 0);
    present_queue = vk::raii::Queue(device, graphics_and_present_queue_family_index.at(1), 0);

    return *this;
}

auto VulkanRender::initSwapchain(std::uint32_t window_width, std::uint32_t window_height) -> VulkanRender & {
    // Create Swapchain
    // So we have something to render into
    //
    // What the following section entails:
    // - Figure out what formats and capabilities we can use and enable
    // - Create Swapchain
    // - Create VkImages and ImageViews
    // - Create depth buffer, to allow for 3d

    // Get the supported VkFormats
    std::vector<vk::SurfaceFormatKHR> surface_formats = physical_device.getSurfaceFormatsKHR(*surface);
    assert(!surface_formats.empty());

    color_format =
        (surface_formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : surface_formats[0].format;

    const vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);

    if (surface_capabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max()) {
        // If the surface size is undefined, the size is set to the size of the
        // images requested.
        extent.width = std::clamp(
            window_width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent.height = std::clamp(
            window_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
    } else {
        // If the surface size is defined, the swap chain size must match
        extent = surface_capabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPresentModeKHR.html
    // We can deal with other present modes down the line if the need ever arises
    const vk::PresentModeKHR swapchain_present_mode = vk::PresentModeKHR::eFifo;

    const vk::SurfaceTransformFlagBitsKHR pre_transform =
        (surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
            : surface_capabilities.currentTransform;

    const vk::CompositeAlphaFlagBitsKHR composite_alpha =
        (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
            ? vk::CompositeAlphaFlagBitsKHR::eInherit
            : vk::CompositeAlphaFlagBitsKHR::eOpaque;

    auto swap_chain_create_info = vk::SwapchainCreateInfoKHR{.flags = vk::SwapchainCreateFlagsKHR(),
        .surface = *surface,
        .minImageCount = surface_capabilities.minImageCount,
        .imageFormat = color_format,
        .imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = pre_transform,
        .compositeAlpha = composite_alpha,
        .presentMode = swapchain_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr};

    if (graphics_and_present_queue_family_index.at(0) != graphics_and_present_queue_family_index.at(1)) {
        // If the graphics and present queues are from different queue
        // families, we either have to explicitly transfer ownership of
        // images between the queues, or we have to create the swapchain
        // with imageSharingMode as vk::SharingMode::eConcurrent
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        swap_chain_create_info.pQueueFamilyIndices = graphics_and_present_queue_family_index.data();
    }

    // Some code below swapchain leaks some xcb memory.
    // TODO: Investigate it.
    swap_chain = vk::raii::SwapchainKHR(device, swap_chain_create_info);
    // Get presentable images associated with vk::raii::SwapchainKHR swapchain
    // This will give us plain VkImages (these are controlled by the swapchain,
    // we should not destroy them ourselves). VkImages basically represents
    // multidimensional arrays of data, wheres a ImageView is more a wrapper
    // around the VkImage (with associated metadata on how to access the image,
    // and which part of the image to acess).
    // Similar in concept as C++'s string and string_view
    swap_chain_images = swap_chain.getImages();

    image_views.reserve(swap_chain_images.size());

    auto image_view_create_info = vk::ImageViewCreateInfo{.flags = vk::ImageViewCreateFlags(),
        .image = nullptr,
        .viewType = vk::ImageViewType::e2D,
        .format = color_format,
        .components = vk::ComponentMapping{.r = vk::ComponentSwizzle::eR,
            .g = vk::ComponentSwizzle::eG,
            .b = vk::ComponentSwizzle::eB,
            .a = vk::ComponentSwizzle::eA},
        .subresourceRange = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};

    for (auto image : swap_chain_images) {
        image_view_create_info.image = image;

        image_views.emplace_back(device, image_view_create_info);
    }

    // Create a depth buffer
    // Depth buffers allow for 3d graphics
    //
    // Creating a depth buffer:
    // - Create Image
    // - Get MemoryRequirements
    // - Determine appropriate memory type index
    // - Allocate Memory
    // - Bind memory to the image
    // - Create imageview's with the depth buffer image

    depth_format = vk::Format::eD16Unorm;
    const vk::FormatProperties format_properties = physical_device.getFormatProperties(depth_format);

    vk::ImageTiling tiling;
    if (format_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        tiling = vk::ImageTiling::eLinear;
    } else if (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        tiling = vk::ImageTiling::eOptimal;
    } else {
        throw std::runtime_error("DepthStencilAttachment is not supported for D16Unorm depth format.");
    }

    auto image_create_info = vk::ImageCreateInfo{.flags = vk::ImageCreateFlags(),
        .imageType = vk::ImageType::e2D,
        .format = depth_format,
        .extent = vk::Extent3D{.width = extent.width, .height = extent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = vk::ImageLayout::eUndefined};

    if (graphics_and_present_queue_family_index.at(0) != graphics_and_present_queue_family_index.at(1)) {
        // Done as the same reason described in swap_chain_create_info comment
        image_create_info.sharingMode = vk::SharingMode::eConcurrent;
        image_create_info.queueFamilyIndexCount = 2;
        image_create_info.pQueueFamilyIndices = graphics_and_present_queue_family_index.data();
    }

    depth_image = vk::raii::Image(device, image_create_info);

    const vk::MemoryRequirements memory_requirements = depth_image.getMemoryRequirements();

    const std::uint32_t memory_type_index = VulkanRender::findMemoryType(physical_device.getMemoryProperties(),
        memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    depth_memory = vk::raii::DeviceMemory(device,
        vk::MemoryAllocateInfo{.allocationSize = memory_requirements.size, .memoryTypeIndex = memory_type_index});

    depth_image.bindMemory(*depth_memory, 0);

    depth_view = vk::raii::ImageView(
        device, vk::ImageViewCreateInfo{.flags = vk::ImageViewCreateFlags(),
                    .image = *depth_image,
                    .viewType = vk::ImageViewType::e2D,
                    .format = depth_format,
                    .components = vk::ComponentMapping{.r = vk::ComponentSwizzle::eR,
                        .g = vk::ComponentSwizzle::eG,
                        .b = vk::ComponentSwizzle::eB,
                        .a = vk::ComponentSwizzle::eA},
                    .subresourceRange = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eDepth,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1}});

    return *this;
}

auto VulkanRender::createUniformBuffer() -> VulkanRender & {
    // My specific data that I want to use in the uniform buffer
    // At the moment I'll have it inside the function till I get the inital
    // prototype finished, but I'd prefer to pass that sort of data into
    // the function.

    // Uniform buffers are great for small, read only data
    // E.g. The below matric (our cube) will get uploaded into the uniform
    // buffer).
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

    std::tie(uniform_buffer_memory, uniform_data_buffer) = createBuffer(vk::BufferUsageFlagBits::eUniformBuffer, mvpc);

    return *this;
}

auto VulkanRender::initRenderPass() -> VulkanRender & {
    std::array<vk::AttachmentDescription, 2> attachment_descriptions = {
        vk::AttachmentDescription{.flags = vk::AttachmentDescriptionFlags(),
            .format = color_format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR},
        vk::AttachmentDescription{.flags = vk::AttachmentDescriptionFlags(),
            .format = depth_format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eDontCare,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal}};

    auto color_reference = vk::AttachmentReference{.attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal};

    auto depth_reference =
        vk::AttachmentReference{.attachment = 1, .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

    auto subpass = vk::SubpassDescription{.flags = vk::SubpassDescriptionFlags(),
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depth_reference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr};

    render_pass =
        vk::raii::RenderPass(device, vk::RenderPassCreateInfo{.flags = vk::RenderPassCreateFlags(),
                                         .attachmentCount = static_cast<std::uint32_t>(attachment_descriptions.size()),
                                         .pAttachments = attachment_descriptions.data(),
                                         .subpassCount = 1,
                                         .pSubpasses = &subpass,
                                         .dependencyCount = 0,
                                         .pDependencies = nullptr});

    return *this;
}

auto VulkanRender::initFramebuffers() -> VulkanRender & {
    std::array<vk::ImageView, 2> attachments;
    attachments[1] = *depth_view;

    framebuffers.reserve(image_views.size());

    for (auto const &image_view : image_views) {
        attachments[0] = *image_view;

        auto framebuffer_create_info = vk::FramebufferCreateInfo{.flags = vk::FramebufferCreateFlags(),
            .renderPass = *render_pass,
            .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1};

        framebuffers.emplace_back(device, framebuffer_create_info);
    }

    return *this;
}

auto VulkanRender::createVertexBuffer() -> VulkanRender & {
    std::tie(vertex_buffer_memory, vertex_buffer) =
        createBuffer(vk::BufferUsageFlagBits::eVertexBuffer, colored_cube_data);

    return *this;
}

auto VulkanRender::initPipeline() -> VulkanRender & {
    auto descriptor_set_layout_binding = vk::DescriptorSetLayoutBinding{.binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr};

    descriptor_set_layout = vk::raii::DescriptorSetLayout(
        device, vk::DescriptorSetLayoutCreateInfo{.flags = vk::DescriptorSetLayoutCreateFlags(),
                    .bindingCount = 1,
                    .pBindings = &descriptor_set_layout_binding});

    pipeline_layout =
        vk::raii::PipelineLayout(device, vk::PipelineLayoutCreateInfo{.flags = vk::PipelineLayoutCreateFlags(),
                                             .setLayoutCount = 1,
                                             .pSetLayouts = &(*descriptor_set_layout),
                                             .pushConstantRangeCount = 0,
                                             .pPushConstantRanges = nullptr});

    // E.g. UNTESTED
    // std::vector<vk::DescriptorPoolSize> pool_sizes =
    // {
    //  { vk::DescriptorType::eUniformBuffer, 10 }
    // };
    auto pool_size = vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1};

    descriptor_pool = vk::raii::DescriptorPool(
        device, vk::DescriptorPoolCreateInfo{// eFreeDescriptorSet flag needed when using Vulkan RAII Library
                    .flags = vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
                    .maxSets = 1,
                    .poolSizeCount = 1,
                    .pPoolSizes = &pool_size});

    descriptor_sets = vk::raii::DescriptorSets(device,
        vk::DescriptorSetAllocateInfo{
            .descriptorPool = *descriptor_pool, .descriptorSetCount = 1, .pSetLayouts = &(*descriptor_set_layout)});

    descriptor_set = std::move(descriptor_sets.front());

    const vk::DescriptorBufferInfo descriptor_buffer_info{
        .buffer = *uniform_data_buffer, .offset = 0, .range = sizeof(glm::mat4x4)};

    const vk::WriteDescriptorSet write_descriptor_set{.dstSet = *descriptor_set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pImageInfo = nullptr,
        .pBufferInfo = &descriptor_buffer_info,
        .pTexelBufferView = nullptr};

    device.updateDescriptorSets(write_descriptor_set, nullptr);

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
    vertex_shader_module =
        vk::raii::ShaderModule(device, vk::ShaderModuleCreateInfo{.flags = vk::ShaderModuleCreateFlags(),
                                           .codeSize = vert_shader.size() * sizeof(std::uint32_t),
                                           .pCode = vert_shader.data()});

    fragment_shader_module =
        vk::raii::ShaderModule(device, vk::ShaderModuleCreateInfo{.flags = vk::ShaderModuleCreateFlags(),
                                           .codeSize = frag_shader.size() * sizeof(std::uint32_t),
                                           .pCode = frag_shader.data()});

    std::array<vk::PipelineShaderStageCreateInfo, 2> pipeline_shader_stage_create_infos = {
        vk::PipelineShaderStageCreateInfo{.flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vertex_shader_module,
            .pName = "main",
            .pSpecializationInfo = nullptr},
        vk::PipelineShaderStageCreateInfo{.flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *fragment_shader_module,
            .pName = "main",
            .pSpecializationInfo = nullptr}};

    auto vertex_input_binding_description = vk::VertexInputBindingDescription{
        .binding = 0, .stride = sizeof(colored_cube_data[0]), .inputRate = vk::VertexInputRate::eVertex};

    std::array<vk::VertexInputAttributeDescription, 2> vertex_input_attribute_descriptions = {
        vk::VertexInputAttributeDescription{
            .location = 0, .binding = 0, .format = vk::Format::eR32G32B32A32Sfloat, .offset = 0},
        vk::VertexInputAttributeDescription{
            .location = 1, .binding = 0, .format = vk::Format::eR32G32B32A32Sfloat, .offset = 16}};

    auto pipeline_vertex_input_state_create_info =
        vk::PipelineVertexInputStateCreateInfo{.flags = vk::PipelineVertexInputStateCreateFlags(),
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertex_input_binding_description,
            .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_input_attribute_descriptions.size()),
            .pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data()};

    auto pipeline_input_assembly_state_create_info =
        vk::PipelineInputAssemblyStateCreateInfo{.flags = vk::PipelineInputAssemblyStateCreateFlags(),
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = VK_FALSE};

    auto pipeline_viewport_state_create_info =
        vk::PipelineViewportStateCreateInfo{.flags = vk::PipelineViewportStateCreateFlags(),
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr};

    auto pipeline_rasterization_state_create_info =
        vk::PipelineRasterizationStateCreateInfo{.flags = vk::PipelineRasterizationStateCreateFlags(),
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

    auto pipeline_multisample_state_create_info =
        vk::PipelineMultisampleStateCreateInfo{.flags = vk::PipelineMultisampleStateCreateFlags(),
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0.0F,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE};

    auto stencil_op_state = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,
        .passOp = vk::StencilOp::eKeep,
        .depthFailOp = vk::StencilOp::eKeep,
        .compareOp = vk::CompareOp::eAlways,
        .compareMask = 0,
        .writeMask = 0,
        .reference = 0};

    auto pipeline_depth_stencil_state_create_info =
        vk::PipelineDepthStencilStateCreateInfo{.flags = vk::PipelineDepthStencilStateCreateFlags(),
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = vk::CompareOp::eLessOrEqual,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = stencil_op_state,
            .back = stencil_op_state,
            .minDepthBounds = VK_FALSE,
            .maxDepthBounds = VK_FALSE};

    const vk::ColorComponentFlags color_component_flags(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);

    auto pipeline_color_blend_attachment_state = vk::PipelineColorBlendAttachmentState{.blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eZero,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eZero,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = color_component_flags};

    auto pipeline_color_blend_state_create_info =
        vk::PipelineColorBlendStateCreateInfo{.flags = vk::PipelineColorBlendStateCreateFlags(),
            .logicOpEnable = VK_FALSE,
            .logicOp = vk::LogicOp::eNoOp,
            .attachmentCount = 1,
            .pAttachments = &pipeline_color_blend_attachment_state,
            .blendConstants = {{1.0F, 1.0F, 1.0F, 1.0F}}};

    std::array<vk::DynamicState, 2> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    auto pipeline_dynamic_state_create_info =
        vk::PipelineDynamicStateCreateInfo{.flags = vk::PipelineDynamicStateCreateFlags(),
            .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()};

    auto graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo{.flags = vk::PipelineCreateFlags(),
        .stageCount = static_cast<std::uint32_t>(pipeline_shader_stage_create_infos.size()),
        .pStages = pipeline_shader_stage_create_infos.data(),
        .pVertexInputState = &pipeline_vertex_input_state_create_info,
        .pInputAssemblyState = &pipeline_input_assembly_state_create_info,
        .pTessellationState = nullptr,
        .pViewportState = &pipeline_viewport_state_create_info,
        .pRasterizationState = &pipeline_rasterization_state_create_info,
        .pMultisampleState = &pipeline_multisample_state_create_info,
        .pDepthStencilState = &pipeline_depth_stencil_state_create_info,
        .pColorBlendState = &pipeline_color_blend_state_create_info,
        .pDynamicState = &pipeline_dynamic_state_create_info,
        .layout = *pipeline_layout,
        .renderPass = *render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = 0};

    // TODO Setup and use pipeline cache properly
    // https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/RAII_Samples/PipelineCache/PipelineCache.cpp
    graphics_pipeline_cache = vk::raii::PipelineCache(device, vk::PipelineCacheCreateInfo());
    graphics_pipeline = vk::raii::Pipeline(device, graphics_pipeline_cache, graphics_pipeline_create_info);

    switch (graphics_pipeline.getConstructorSuccessCode()) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::ePipelineCompileRequiredEXT:
        // Do something meaningfull here
        std::println("vk::Pipeline returned vk::Result::ePipelineCompileRequiredEXT !");
        break;
    default:
        assert(false); // should never happen
    }

    return *this;
}

auto VulkanRender::render() -> VulkanRender & {
    // Aquire next image
    image_acquired_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());

    vk::Result result;
    std::tie(result, image_index) = swap_chain.acquireNextImage(fence_timeout, *image_acquired_semaphore);
    assert(image_index < swap_chain_images.size());

    // TODO Properly handle anything other than an Success!
    switch (result) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eTimeout:
        break;
    case vk::Result::eNotReady:
        break;
    case vk::Result::eSuboptimalKHR:
        break;
    default:
        // an unexpected result is returned!
        assert(false);
    }

    command_buffer.begin(
        vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlags(), .pInheritanceInfo = nullptr});

    std::array<vk::ClearValue, 2> clear_values = {
        vk::ClearValue{.color = vk::ClearColorValue{.float32 = std::array<float, 4>({{0.2F, 0.2F, 0.2F, 0.2F}})}},
        vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.0F, .stencil = 0}}};

    auto render_pass_begin_info = vk::RenderPassBeginInfo{.renderPass = *render_pass,
        .framebuffer = *framebuffers[image_index],
        .renderArea = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0}, .extent = extent},
        .clearValueCount = clear_values.size(),
        .pClearValues = clear_values.data()};

    command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);
    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *pipeline_layout, 0, {*descriptor_set}, nullptr);

    command_buffer.bindVertexBuffers(0, {*vertex_buffer}, {0});
    command_buffer.setViewport(0, vk::Viewport{.x = 0.0F,
                                      .y = 0.0F,
                                      .width = static_cast<float>(extent.width),
                                      .height = static_cast<float>(extent.height),
                                      .minDepth = 0.0F,
                                      .maxDepth = 1.0F});
    command_buffer.setScissor(0, vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0}, .extent = extent});

    command_buffer.draw(12 * 3, 1, 0, 0);
    command_buffer.endRenderPass();
    command_buffer.end();

    draw_fence = vk::raii::Fence(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlags()});

    const vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    auto submit_info = vk::SubmitInfo{.waitSemaphoreCount = 1,
        .pWaitSemaphores = &(*image_acquired_semaphore),
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &(*command_buffer),
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr};

    graphics_queue.submit(submit_info, *draw_fence);

    /* Make sure command buffer is finished before presenting */
    while (vk::Result::eTimeout == device.waitForFences({*draw_fence}, VK_TRUE, fence_timeout)) {
        /* do nothing */
    }

    return *this;
}

auto VulkanRender::present() -> VulkanRender & {
    /* Now present the image in the window */
    auto present_result = present_queue.presentKHR(vk::PresentInfoKHR{.waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .swapchainCount = 1,
        .pSwapchains = &(*swap_chain),
        .pImageIndices = &image_index,
        .pResults = nullptr});

    switch (present_result) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR:
        // Do something meaningfull here
        std::println("vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !");
        break;
    default:
        // an unexpected result is returned !
        assert(false);
    }

    device.waitIdle();

    return *this;
}
