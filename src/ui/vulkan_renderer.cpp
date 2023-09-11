#include <chrono>
#include <iostream>
#include <iterator>
#include <thread>
#include <vector>

// Reference, use as needed:
// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html
// https://vulkan.lunarg.com/doc/view/latest/windows/profiles_definitions.html
#include "vulkan_renderer.h"

#if defined(VULKAN_DEBUG)
PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger) {
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator) {
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
// Wondering what VKAPI_ATTR and VKAPI_CALL are?
// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#boilerplate-platform-specific-calling-conventions
// TL;DR: Macros to tweak calling convetions depending on compiler and
// language version used
VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
    void * /*pUserData*/) {
    // NOTE:
    // Function body taken from
    // https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/RAII_Samples/EnableValidationWithCallback/EnableValidationWithCallback.cpp
    // TODO:
    // - Make my own one instead of copy+paste. But for getting a basic thing
    // up and running, this is good enough for me.
    std::string message;

    message += vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + ": " +
               vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n";
    message += std::string("\t") + "messageIDName   = <" + pCallbackData->pMessageIdName + ">\n";
    message += std::string("\t") + "messageIdNumber = " + std::to_string(pCallbackData->messageIdNumber) + "\n";
    message += std::string("\t") + "message = <" + pCallbackData->pMessage + ">\n";

    if (0 < pCallbackData->queueLabelCount) {
        message += std::string("\t") + "Queue Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
            message += std::string("\t\t") + "labelName = <" + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount) {
        message += std::string("\t") + "CommandBuffer Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
            message += std::string("\t\t") + "labelName = <" + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
        }
    }
    if (0 < pCallbackData->objectCount) {
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            message += std::string("\t") + "Object " + std::to_string(i) + "\n";
            message += std::string("\t\t") + "objectType   = " + vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) + "\n";
            message += std::string("\t\t") + "objectHandle = " + std::to_string(pCallbackData->pObjects[i].objectHandle) + "\n";
            if (pCallbackData->pObjects[i].pObjectName) {
                message += std::string("\t\t") + "objectName   = <" + pCallbackData->pObjects[i].pObjectName + ">\n";
            }
        }
    }

    std::cout << message << std::endl;

    return VK_FALSE;
}
#endif

auto VulkanRender::findMemoryType(vk::PhysicalDeviceMemoryProperties const &memoryProperties,
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

    assert(typeIndex != std::uint32_t(~0));
    return typeIndex;
}

auto VulkanRender::enumerateExtensions(std::vector<vk::ExtensionProperties> const &extensionProperties, std::set<std::string> const &desiredExtensions) -> std::vector<char const *> {
    std::vector<char const *> extensions;

    // Check and enable (if found) required extensions
    extensions.reserve(desiredExtensions.size());

    for (auto const &extension : extensionProperties) {
        if (desiredExtensions.contains(extension.extensionName)) {
            extensions.push_back(extension.extensionName);
        }
    }

    return extensions;
}

auto VulkanRender::enumerateLayers(std::vector<vk::LayerProperties> const &layerProperties, std::set<std::string> const &desiredLayers) -> std::vector<char const *> {
    std::vector<char const *> layers;

    // Check and enable (if found) required layers
    layers.reserve(desiredLayers.size());

    for (auto const &layer : layerProperties) {
        if (desiredLayers.contains(layer.layerName)) {
            layers.push_back(layer.layerName);
        }
    }
    return layers;
}

auto VulkanRender::getQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &queueFamilyProperties, vk::QueueFlagBits queueFlag) -> std::uint32_t {
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlag & vk::QueueFlagBits::eCompute) {
        // TODO: Test the iterator ("it") part of the range based for loop
        // actually works. Either via looking at the variable in GDB or
        // testing on a device that has more then one queuefamily available
        for (auto it = queueFamilyProperties.begin(); auto const &qfp : queueFamilyProperties) {
            if ((qfp.queueFlags & queueFlag) && !(qfp.queueFlags & vk::QueueFlagBits::eGraphics)) {
                return static_cast<std::uint32_t>(std::distance(queueFamilyProperties.begin(), it));
            }

            it++;
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlag & vk::QueueFlagBits::eTransfer) {
        for (auto it = queueFamilyProperties.begin(); auto const &qfp : queueFamilyProperties) {
            if ((qfp.queueFlags & queueFlag) && !(qfp.queueFlags & vk::QueueFlagBits::eGraphics) && !(qfp.queueFlags & vk::QueueFlagBits::eCompute)) {
                return static_cast<std::uint32_t>(std::distance(queueFamilyProperties.begin(), it));
            }

            it++;
        }
    }

    // For other queue types or if no separate compute queue is present, return
    // the first one to support the requested flags
    for (auto it = queueFamilyProperties.begin(); auto const &qfp : queueFamilyProperties) {
        if (qfp.queueFlags & queueFlag) {
            return static_cast<std::uint32_t>(std::distance(queueFamilyProperties.begin(), it));
        }

        it++;
    }

    throw std::runtime_error("Could not find a matching queue family index");
}
auto VulkanRender::getGraphicsAndPresentQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &queueFamilyProperties, std::uint32_t graphicsQueueFamilyIndex) -> std::array<std::uint32_t, 2> {
    std::array<std::uint32_t, 2> queueFamilyIndices;

    if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, *surface)) {
        queueFamilyIndices.at(0) = graphicsQueueFamilyIndex;
        queueFamilyIndices.at(1) = graphicsQueueFamilyIndex;
        return queueFamilyIndices;
        // The first graphicsQueueFamilyIndex does also support present
    }

    // The graphicsQueueFamilyIndex doesn't support present -> look for another
    // family index that supports both graphics and present
    for (auto it = queueFamilyProperties.begin(); auto const &qfp : queueFamilyProperties) {
        auto i = static_cast<std::uint32_t>(std::distance(queueFamilyProperties.begin(), it));
        if ((qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(i, *surface)) {
            queueFamilyIndices.at(0) = i;
            queueFamilyIndices.at(1) = i;
            return queueFamilyIndices;
        }

        it++;
    }

    // There's nothing like a single family index that supports both grahics
    // and present -> look for another family index that supports present
    for (auto it = queueFamilyProperties.begin(); [[maybe_unused]] auto const &qfp : queueFamilyProperties) {
        auto i = static_cast<std::uint32_t>(std::distance(queueFamilyProperties.begin(), it));
        if (physicalDevice.getSurfaceSupportKHR(i, *surface)) {
            queueFamilyIndices.at(0) = graphicsQueueFamilyIndex;
            queueFamilyIndices.at(1) = i;
            return queueFamilyIndices;
        }

        it++;
    }

    throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
}


VulkanRender::VulkanRender(const std::string &AppName) {
    // This isn't actually used inside any vulkan code, it's info that is
    // possibly used by GPU vendors to give your application special treatment
    auto applicationInfo = vk::ApplicationInfo{
        .pApplicationName = AppName.c_str(),
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_3};

    // I'd prefer to pass "context.enumerate...Properties()" directly to the
    // function(s), but this causes a heap-use-after-free according to
    // AddressSanitizer. So...
    // TODO Check what the underlying cause for the heap-use-after-free
    // if doing it my preferred way
    auto instanceLayerProperties = context.enumerateInstanceLayerProperties();
    auto instanceLayers = enumerateLayers(instanceLayerProperties, desiredInstanceLayers);

    auto instanceExtensionProperties = context.enumerateInstanceExtensionProperties();
    auto instanceExtensions = enumerateExtensions(instanceExtensionProperties, desiredInstanceExtensions);

    instance = vk::raii::Instance(
        context,
        vk::InstanceCreateInfo{
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()});

#if defined(VULKAN_DEBUG)
    // Enable validation with debug callback (if program is compiled as debug)
    pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    if (!pfnVkCreateDebugUtilsMessengerEXT || !pfnVkDestroyDebugUtilsMessengerEXT) {
        throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT and pfnVkDestroyDebugUtilsMessengerEXT function.");
    }

    debugUtilsMessenger = vk::raii::DebugUtilsMessengerEXT(
        instance,
        vk::DebugUtilsMessengerCreateInfoEXT{
            .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
            // Enabled all severitys and all types mainly for inital setup and debug puposes
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
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

// TODO Currently have a memory leak on cleanup
VulkanRender::~VulkanRender() {
}

void VulkanRender::createSurface(SDL_Window *window) {
    // We need to pass the C version of the object to the 3rd-party library
    VkSurfaceKHR _surface = nullptr;
    if (SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(*instance), &_surface) == 0) {
        throw std::runtime_error("Could not create a Vulkan surface: " + std::string(SDL_GetError()));
    }

    // We can then transfer the C version to  the RAII C++ version of the object :)
    surface = vk::raii::SurfaceKHR(instance, _surface);
}

void VulkanRender::selectPhysicalDevice() {
    // Enumerate the physicalDevices
    vk::raii::PhysicalDevices physicalDevices(instance);

    if (physicalDevices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan Support.");
    }

    // Then go through and select the preferred device
    // e.g. Prefering a dedicated GPU over an intergrated GPU
    // NOTE: There could be a lot more here, but for now this is good enough
    for (auto const &pds : physicalDevices) {
        if (pds.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            physicalDevice = pds;
        }
    }

    if (!*physicalDevice) {
        // Otherwise just default to the first available device
        physicalDevice = physicalDevices.front();
    }

    vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
    std::cout << "Device Name    : " << deviceProperties.deviceName << "\n";
    std::cout << "Device Type    : " << vk::to_string(deviceProperties.deviceType) << "\n";
    std::cout << "Vulkan Version : " << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_PATCH(deviceProperties.apiVersion) << "\n";
    // Example of accessing the limits struct
    // std::cout << "Max Compute Shared Memory Size: " << deviceProperties.limits.maxComputeSharedMemorySize / 1024 << " KB" << std::endl;

    // Notes for vulkan terminology (to better understand the next section of code):
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

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    assert(queueFamilyProperties.size() < std::numeric_limits<std::uint32_t>::max());

    if (queueFamilyProperties.empty()) {
        throw std::runtime_error("No queue family found.");
    }

    auto graphicsQueueFamilyIndex = getQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eGraphics);
    assert(graphicsQueueFamilyIndex < queueFamilyProperties.size());

    graphicsAndPresentQueueFamilyIndex = getGraphicsAndPresentQueueFamilyIndex(queueFamilyProperties, graphicsQueueFamilyIndex);
}

void VulkanRender::initDevice() {
    auto deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
    auto deviceExtensions = enumerateExtensions(deviceExtensionProperties, desiredDeviceExtensions);

    // Create a Device
    float queuePriority = 0.0F;

    auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{
        .flags = vk::DeviceQueueCreateFlags(),
        .queueFamilyIndex = graphicsAndPresentQueueFamilyIndex.at(0),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    // Note for vk:DeviceCreateInfo:
    // Even though Device Layers have been deprecated, it is still
    // recommended applications pass an empty list of layers or a
    // list that exactly matches the sequence enabled at
    // *Instance* creation time.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
    device = vk::raii::Device(physicalDevice,
        vk::DeviceCreateInfo{
            .flags = vk::DeviceCreateFlags(),
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &physicalDeviceFeatures});

    // Create CommandBuffers and CommandPools
    // CommandPools allocate CommandBuffers, which is where commands for
    // GPU's get recoreded to. The GPU does not excute anything until the
    // command buffer is submitted
    //
    // Special Vulkan RAII Note:
    // Due to the raii-approach, the usual implicit actions done in the pure
    // C fuctions way does not mesh well with the raii-approach. So, to handle
    // it correctly you should explicitly destroy vk::raii::CommandBuffers
    // before destroying the related vk::raii::CommandPool
    commandPool = vk::raii::CommandPool(device,
        vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlags(),
            .queueFamilyIndex = graphicsAndPresentQueueFamilyIndex.at(0)});

    // The commandBufferCount of the vk::CommandBufferAllocateInfo struct
    // controls how many elements are in the
    // std::vector(vk::raii::CommandBuffers).
    // At the moment, we only intended to make use of a singular command buffer
    commandBuffers = vk::raii::CommandBuffers(device,
        vk::CommandBufferAllocateInfo{
            .commandPool = *commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1});

    commandBuffer = std::move(commandBuffers.front());

    // Create the queues for later use
    graphicsQueue = vk::raii::Queue(device, graphicsAndPresentQueueFamilyIndex.at(0), 0);
    presentQueue = vk::raii::Queue(device, graphicsAndPresentQueueFamilyIndex.at(1), 0);
}

void VulkanRender::initSwapchain(int windowWidth, int windowHeight) {
    // Create Swapchain
    // So we have something to render into
    //
    // What the following section entails:
    // - Figure out what formats and capabilities we can use and enable
    // - Create Swapchain
    // - Create VkImages and ImageViews
    // - Create depth buffer, to allow for 3d

    // Get the supported VkFormats
    std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
    assert(!surfaceFormats.empty());

    colorFormat =
        (surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : surfaceFormats[0].format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);

    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max()) {
        // If the surface size is undefined, the size is set to the size of the images requested.
        extent.width =
            std::clamp(static_cast<std::uint32_t>(windowWidth), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height =
            std::clamp(static_cast<std::uint32_t>(windowHeight), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    } else {
        // If the surface size is defined, the swap chain size must match
        extent = surfaceCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPresentModeKHR.html
    // We can deal with other present modes down the line if the need ever arises
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

    auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR{
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = *surface,
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

    if (graphicsAndPresentQueueFamilyIndex.at(0) != graphicsAndPresentQueueFamilyIndex.at(1)) {
        // If the graphics and present queues are from different queue
        // families, we either have to explicitly transfer ownership of
        // images between the queues, or we have to create the swapchain
        // with imageSharingMode as vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = graphicsAndPresentQueueFamilyIndex.data();
    }

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    // Get presentable images associated with vk::raii::SwapchainKHR swapchain
    // This will give us plain VkImages (these are controlled by the swapchain,
    // we should not destroy them ourselves). VkImages basically represents
    // multidimensional arrays of data, wheres a ImageView is more a wrapper
    // around the VkImage (with associated metadata on how to access the image,
    // and which part of the image to acess).
    // Similar in concept as C++'s string and string_view
    swapChainImages = swapChain.getImages();

    imageViews.reserve(swapChainImages.size());

    // clang-format off
    // clang format breaks on multiple nested direct initialization structs.
    auto imageViewCreateInfo = vk::ImageViewCreateInfo{
        .flags = vk::ImageViewCreateFlags(),
        .image = nullptr,
        .viewType = vk::ImageViewType::e2D,
        .format = colorFormat,
        .components = vk::ComponentMapping{
            .r = vk::ComponentSwizzle::eR,
            .g = vk::ComponentSwizzle::eG,
            .b = vk::ComponentSwizzle::eB,
            .a = vk::ComponentSwizzle::eA},
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};
    // clang-format on

    for (auto image : swapChainImages) {
        imageViewCreateInfo.image = image;

        imageViews.emplace_back(device, imageViewCreateInfo);
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

    depthFormat = vk::Format::eD16Unorm;
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

    if (graphicsAndPresentQueueFamilyIndex.at(0) != graphicsAndPresentQueueFamilyIndex.at(1)) {
        // Done as the same reason described in swapChainCreateInfo comment
        imageCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = graphicsAndPresentQueueFamilyIndex.data();
    }

    depthImage = vk::raii::Image(device, imageCreateInfo);

    vk::MemoryRequirements memoryRequirements = depthImage.getMemoryRequirements();

    std::uint32_t memoryTypeIndex = findMemoryType(
        physicalDevice.getMemoryProperties(),
        memoryRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    depthMemory = vk::raii::DeviceMemory(device,
        vk::MemoryAllocateInfo{
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex});

    depthImage.bindMemory(*depthMemory, 0);

    // clang-format off
    // clang format breaks on multiple nested direct initialization structs.
    depthView = vk::raii::ImageView(device,
        vk::ImageViewCreateInfo{
            .flags = vk::ImageViewCreateFlags(),
            .image = *depthImage,
            .viewType = vk::ImageViewType::e2D,
            .format = depthFormat,
            .components = vk::ComponentMapping{
                .r = vk::ComponentSwizzle::eR,
                .g = vk::ComponentSwizzle::eG,
                .b = vk::ComponentSwizzle::eB,
                .a = vk::ComponentSwizzle::eA},
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eDepth,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}});
    // clang-format on
}

void VulkanRender::createUniformBuffer() {
    // My specific data that I want to use in the uniform buffer
    // At the moment I'll have it inside the function till I get the inital
    // prototype finished, but I'd prefer to pass that sort of data into
    // the function.

    // Uniform buffers are great for small, read only data
    // E.g. The below matric (our cube) will get uploaded into the uniform buffer).
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

    std::tie(uniformBufferMemory, uniformDataBuffer) = createBuffer(vk::BufferUsageFlagBits::eUniformBuffer, mvpc);
}

void VulkanRender::initRenderPass() {
    std::array<vk::AttachmentDescription, 2> attachmentDescriptions = {
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

    renderPass = vk::raii::RenderPass(device,
        vk::RenderPassCreateInfo{
            .flags = vk::RenderPassCreateFlags(),
            .attachmentCount = static_cast<std::uint32_t>(attachmentDescriptions.size()),
            .pAttachments = attachmentDescriptions.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 0,
            .pDependencies = nullptr});
}

void VulkanRender::initFramebuffers() {
    std::array<vk::ImageView, 2> attachments;
    attachments[1] = *depthView;

    framebuffers.reserve(imageViews.size());

    for (auto const &imageView : imageViews) {
        attachments[0] = *imageView;

        auto framebufferCreateInfo = vk::FramebufferCreateInfo{
            .flags = vk::FramebufferCreateFlags(),
            .renderPass = *renderPass,
            .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1};

        framebuffers.emplace_back(device, framebufferCreateInfo);
    }
}

void VulkanRender::createVertexBuffer() {
    std::tie(vertexBufferMemory, vertexBuffer) = createBuffer(vk::BufferUsageFlagBits::eVertexBuffer, coloredCubeData);
}

void VulkanRender::initPipeline() {
    auto descriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr};

    descriptorSetLayout = vk::raii::DescriptorSetLayout(device,
        vk::DescriptorSetLayoutCreateInfo{
            .flags = vk::DescriptorSetLayoutCreateFlags(),
            .bindingCount = 1,
            .pBindings = &descriptorSetLayoutBinding});

    pipelineLayout = vk::raii::PipelineLayout(device,
        vk::PipelineLayoutCreateInfo{
            .flags = vk::PipelineLayoutCreateFlags(),
            .setLayoutCount = 1,
            .pSetLayouts = &(*descriptorSetLayout),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr});

    // E.g. UNTESTED
    // std::vector<vk::DescriptorPoolSize> poolSizes =
    // {
    //  { vk::DescriptorType::eUniformBuffer, 10 }
    // };
    auto poolSize = vk::DescriptorPoolSize{
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1};

    descriptorPool = vk::raii::DescriptorPool(device,
        vk::DescriptorPoolCreateInfo{
            // eFreeDescriptorSet flag needed when using Vulkan RAII Library
            .flags = vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize});

    descriptorSets = vk::raii::DescriptorSets(device,
        vk::DescriptorSetAllocateInfo{
            .descriptorPool = *descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &(*descriptorSetLayout)});

    descriptorSet = std::move(descriptorSets.front());

    vk::DescriptorBufferInfo descriptorBufferInfo{
        .buffer = *uniformDataBuffer,
        .offset = 0,
        .range = sizeof(glm::mat4x4)};

    vk::WriteDescriptorSet writeDescriptorSet{
        .dstSet = *descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pImageInfo = nullptr,
        .pBufferInfo = &descriptorBufferInfo,
        .pTexelBufferView = nullptr};

    device.updateDescriptorSets(writeDescriptorSet, nullptr);

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
    vertexShaderModule = vk::raii::ShaderModule(device,
        vk::ShaderModuleCreateInfo{
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = vertShader.size() * sizeof(std::uint32_t),
            .pCode = vertShader.data()});

    fragmentShaderModule = vk::raii::ShaderModule(device,
        vk::ShaderModuleCreateInfo{
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = fragShader.size() * sizeof(std::uint32_t),
            .pCode = fragShader.data()});

    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
        vk::PipelineShaderStageCreateInfo{
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vertexShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr},
        vk::PipelineShaderStageCreateInfo{
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *fragmentShaderModule,
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
        .layout = *pipelineLayout,
        .renderPass = *renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = 0};

    // TODO Setup and use pipeline cache properly
    // https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/RAII_Samples/PipelineCache/PipelineCache.cpp
    graphicsPipelineCache = vk::raii::PipelineCache(device, vk::PipelineCacheCreateInfo());
    graphicsPipeline = vk::raii::Pipeline(device, graphicsPipelineCache, graphicsPipelineCreateInfo);

    switch (graphicsPipeline.getConstructorSuccessCode()) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::ePipelineCompileRequiredEXT:
        // Do something meaningfull here
        std::cout << "vk::Pipeline returned vk::Result::ePipelineCompileRequiredEXT !\n";
        break;
    default:
        assert(false); // should never happen
    }
}

auto VulkanRender::aquireNextImage() -> std::pair<vk::Result, std::uint32_t> {
    imageAcquiredSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());

    vk::Result result;
    std::uint32_t imageIndex;

    std::tie(result, imageIndex) = swapChain.acquireNextImage(fenceTimeout, *imageAcquiredSemaphore);

    if (result != vk::Result::eSuccess) {
        return {result, imageIndex};
    }
    assert(imageIndex < swapChainImages.size());

    return {vk::Result::eSuccess, imageIndex};
}

void VulkanRender::render() {
    std::tie(result, imageIndex) = aquireNextImage();

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
        // an unexpected result is returned !
        assert(false); 
    }

    commandBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlags(),
        .pInheritanceInfo = nullptr});

    std::array<vk::ClearValue, 2> clearValues = {
        vk::ClearValue{
            .color = vk::ClearColorValue{
                .float32 = std::array<float, 4>({{0.2F, 0.2F, 0.2F, 0.2F}})}},
        vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.0F, .stencil = 0}}};

    auto renderPassBeginInfo = vk::RenderPassBeginInfo{
        .renderPass = *renderPass,
        .framebuffer = *framebuffers[imageIndex],
        .renderArea = vk::Rect2D{
            .offset = vk::Offset2D{
                .x = 0,
                .y = 0},
            .extent = extent},
        .clearValueCount = clearValues.size(),
        .pClearValues = clearValues.data()};

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, {*descriptorSet}, nullptr);

    commandBuffer.bindVertexBuffers(0, {*vertexBuffer}, {0});
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

    drawFence = vk::raii::Fence(device, vk::FenceCreateInfo{
                                            .flags = vk::FenceCreateFlags()});

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    auto submitInfo = vk::SubmitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(*imageAcquiredSemaphore),
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &(*commandBuffer),
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr};

    graphicsQueue.submit(submitInfo, *drawFence);

    /* Make sure command buffer is finished before presenting */
    while (vk::Result::eTimeout == device.waitForFences({*drawFence}, VK_TRUE, fenceTimeout)) {
        /* do nothing */
    }
}

void VulkanRender::present() {
    /* Now present the image in the window */
    auto presentResult = presentQueue.presentKHR(
        vk::PresentInfoKHR{
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &(*swapChain),
            .pImageIndices = &imageIndex,
            .pResults = nullptr});

    switch (presentResult) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR:
        // Do something meaningfull here
        std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
        break;
    default:
        // an unexpected result is returned !
        assert(false); 
    }
    // This is purely here so we can actually have a moment to see our glorius rendered cube!
    // ALL HAIL THE CUBE
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    device.waitIdle();
}
