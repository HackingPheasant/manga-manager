// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#ifndef UI_VULKAN_RENDERER_H
#define UI_VULKAN_RENDERER_H

#include <array>
#include <cstdint>
#include <utility> // vulkan_raii fails to compile without this
                   // It complains about std::exchange related errors
#include <set>
#include <string>

// Enable Vulkan debug utilities if we compile as Debug
#ifndef NDEBUG
#define VULKAN_DEBUG
#endif

// Now include offical vulkan headers :)
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

class VulkanRender {
  public:
    VulkanRender(const std::string &);
    ~VulkanRender() = default;
    vk::raii::Instance &getInstance() { return instance; }
    VulkanRender &selectPhysicalDevice();
    VulkanRender &setSurface(vk::raii::SurfaceKHR _surface) {
        this->surface = std::move(_surface);
        return *this;
    }
    VulkanRender &initDevice();
    VulkanRender &initSwapchain(std::uint32_t, std::uint32_t);
    VulkanRender &createUniformBuffer();
    VulkanRender &initRenderPass();
    VulkanRender &initFramebuffers();
    VulkanRender &createVertexBuffer();
    VulkanRender &initPipeline();
    VulkanRender &render();
    VulkanRender &present();
    template <typename T>
    void copyToDevice(vk::raii::DeviceMemory const &device_memory, VkDeviceSize const &size, T const &data) {
        // devicememory.mapMemory(offset, size)
        std::uint8_t *pData = static_cast<std::uint8_t *>(device_memory.mapMemory(0, size));
        std::memcpy(pData, &data, sizeof(data));
        device_memory.unmapMemory();
    }
    template <typename T>
    auto createBuffer(vk::BufferUsageFlagBits usage_flags, T const &data)
        -> std::pair<vk::raii::DeviceMemory, vk::raii::Buffer> {
        auto buffer_create_info = vk::BufferCreateInfo{
            .flags = vk::BufferCreateFlags(),
            .size = sizeof(data),
            .usage = usage_flags,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
        };

        if (graphics_and_present_queue_family_index.at(0) != graphics_and_present_queue_family_index.at(1)) {
            buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;
            buffer_create_info.queueFamilyIndexCount = 2;
            buffer_create_info.pQueueFamilyIndices = graphics_and_present_queue_family_index.data();
        }

        auto buffer = vk::raii::Buffer(device, buffer_create_info);

        vk::MemoryRequirements memory_requirements = buffer.getMemoryRequirements();

        std::uint32_t type_index =
            findMemoryType(physical_device.getMemoryProperties(), memory_requirements.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::raii::DeviceMemory device_memory(
            device, vk::MemoryAllocateInfo{.allocationSize = memory_requirements.size, .memoryTypeIndex = type_index});

        copyToDevice(device_memory, memory_requirements.size, data);

        buffer.bindMemory(*device_memory, 0);

        return {std::move(device_memory), std::move(buffer)};
    }

  private:
    // TODO Remove const of data memebers
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c12-dont-make-data-members-const-or-references-in-a-copyable-or-movable-type

    // Order of these Vulkan Variables are IMPORTANT
    // Movement of these variables can potentially lead to an incorrect destruction
    // order, which then can cause various Vulkan validation issues.

    // vk::raii::Context
    // This class does not exist in the C API or in the C++ vk namespace
    // It's here to handle the few functions that are not bound to
    // a VkInstance or a VkDevice. For example:
    // context.enumerateInstanceVersion()
    // is a function that isn't bound to an instance or device.
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
#if defined(VULKAN_DEBUG)
    vk::raii::DebugUtilsMessengerEXT debug_utils_messenger = nullptr;
#endif
    vk::raii::PhysicalDevice physical_device = nullptr;
    // For now we will leave everything to the defaults (off), as we don't
    // we rely on any optional features at the moment, so no need to enable any.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#features
    vk::PhysicalDeviceFeatures enable_device_features;
    vk::raii::Device device = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    std::array<std::uint32_t, 2> graphics_and_present_queue_family_index{};
    vk::raii::CommandPool command_pool = nullptr;
    vk::raii::CommandBuffers command_buffers = nullptr;
    vk::raii::CommandBuffer command_buffer = nullptr;
    vk::Format color_format;
    vk::Format depth_format;
    vk::raii::SwapchainKHR swap_chain = nullptr;
    vk::Extent2D extent;
    std::vector<vk::Image> swap_chain_images;
    std::vector<vk::raii::ImageView> image_views;
    vk::raii::Image depth_image = nullptr;
    vk::raii::DeviceMemory depth_memory = nullptr;
    vk::raii::ImageView depth_view = nullptr;
    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
    vk::raii::DescriptorPool descriptor_pool = nullptr;
    vk::raii::DescriptorSets descriptor_sets = nullptr;
    vk::raii::DescriptorSet descriptor_set = nullptr;
    vk::raii::DeviceMemory uniform_buffer_memory = nullptr;
    vk::raii::Buffer uniform_data_buffer = nullptr;
    vk::raii::DeviceMemory vertex_buffer_memory = nullptr;
    vk::raii::Buffer vertex_buffer = nullptr;
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::RenderPass render_pass = nullptr;
    vk::raii::ShaderModule vertex_shader_module = nullptr;
    vk::raii::ShaderModule fragment_shader_module = nullptr;
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::Pipeline graphics_pipeline = nullptr;
    vk::raii::PipelineCache graphics_pipeline_cache = nullptr;
    vk::raii::Queue graphics_queue = nullptr;
    vk::raii::Queue present_queue = nullptr;
    //  TODO This is Temp VVV
    vk::raii::Semaphore image_acquired_semaphore = nullptr;
    vk::raii::Fence draw_fence = nullptr;
    // vk::Result result;
    std::uint32_t image_index = 0;

    // FenceTimeout specifies how long the function waits, in nanoseconds, if no image is available
    // https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkAcquireNextImageKHR.html
    std::uint64_t fence_timeout = 100000000; // 100000000 nanoseconds = 0.1 seconds

    // Shader and Frag code for the renderer
    // Look away, I am commiting c++ crimes
    static auto constexpr vert_shader = std::to_array<std::uint32_t>({
#include "vulkantut.vert.inc"
    });

    static auto constexpr frag_shader = std::to_array<std::uint32_t>({
#include "vulkantut.frag.inc"
    });
    // It's safe to look again.

    // BEGIN Vulkan-HPP Samples stuff
    // It's the colored cube you see on screen.
    struct VertexPC {
        float x, y, z, w; // Position
        float r, g, b, a; // Color
    };

    // I could create std::array myself but I can't be arsed counting
    static auto constexpr colored_cube_data = std::to_array<VertexPC>({
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
    });
    // END Vulkan-HPP Samples stuff

    const std::set<std::string> desired_instance_layers {
#if defined(VULKAN_DEBUG)
        // https://vulkan.lunarg.com/doc/view/latest/linux/khronos_validation_layer.html
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    // NOTE: Not all platform includes below will be made use of yet or at all in this code
    //  but listing them here for future experiments/use
    const std::set<std::string> desired_instance_extensions {
        // clang-format off
#if defined(VULKAN_DEBUG)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_debug_utils.html
        "VK_EXT_debug_utils",
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_android_surface.html
        "VK_KHR_android_surface",
#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_FUCHSIA_imagepipe_surface.html
        "VK_FUCHSIA_imagepipe_surface",
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_metal_surface.html
        "VK_EXT_metal_surface",
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_win32_surface.html
        "VK_KHR_win32_surface",
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_wayland_surface.html
        "VK_KHR_wayland_surface",
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_xcb_surface.html
        "VK_KHR_xcb_surface",
#endif
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_surface.html
        "VK_KHR_surface"
        // clang-format on
    };

    const std::set<std::string> desired_device_extensions{"VK_KHR_swapchain"};

    static auto findMemoryType(vk::PhysicalDeviceMemoryProperties const &, std::uint32_t, vk::MemoryPropertyFlags)
        -> std::uint32_t;
    static auto enumerateExtensions(std::vector<vk::ExtensionProperties> const &, std::set<std::string> const &)
        -> std::vector<char const *>;
    static auto enumerateLayers(std::vector<vk::LayerProperties> const &, std::set<std::string> const &)
        -> std::vector<char const *>;
    static auto getQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &, vk::QueueFlagBits) -> std::uint32_t;
    auto getGraphicsAndPresentQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &, std::uint32_t)
        -> std::array<std::uint32_t, 2>;
};
#endif
