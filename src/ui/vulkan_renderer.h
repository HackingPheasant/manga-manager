#ifndef UI_VULKAN_RENDERER_H
#define UI_VULKAN_RENDERER_H

#include <array>
#include <cinttypes>
#include <utility> // vulkan_raii fails to compile without this
                   // It complains about std::exchange related errors
#include <set>
#include <string>

// Whats the point of this?
// Well I needed to set a few preprocessor defines so they can enable some
// more functionality at compile time if the necessary headers are around
// And since I needed to have these availble in more then one source file
// it was just easier to throw it in a header

// Enable C++ Designated Initializers in Vulkan.hpp
// https://github.com/KhronosGroup/Vulkan-Hpp#designated-initializers
#define VULKAN_HPP_NO_CONSTRUCTORS
// Disable setter member functions
#define VULKAN_HPP_NO_SETTERS

// Enable Vulkan debug utilities if we compile as Debug
#ifndef NDEBUG
#define VULKAN_DEBUG
#endif

// Now include offical vulkan headers :)
#include <vulkan/vulkan_raii.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "window.h"

class VulkanRender {
  public:
    // Inital Vulkan setup
    // - General Vulkan Initaliseation (VulkanRender (class instantiation)):
    //     - Application info
    //     - Vulkan instantiation
    //     - Debug callback setup
    // - Select Physical Device
    // - Create Surface
    // - Initalise Device
    // - Initalise Swapchain
    // - Initalise Render Pass
    // - Initalise Pipeline
    // - Initalise Framebuffers
    //
    // General Render Loop
    // - Acquire next image
    //     - Resize if outdated
    // - Render
    // - Present
    //
    // Initalization Functions
    VulkanRender(const std::string &);
    ~VulkanRender();
    void selectPhysicalDevice();
    void createSurface(SDL_Window *);
    void initDevice();
    void initSwapchain(int, int);
    void createUniformBuffer();
    void initRenderPass();
    void initFramebuffers();
    void createVertexBuffer();
    void initPipeline();
    // Repeat use functions
    auto aquireNextImage() -> std::pair<vk::Result, std::uint32_t>;
    void render();
    void present();
    template <typename T>
    void copyToDevice(vk::raii::DeviceMemory const &deviceMemory, VkDeviceSize const &size, T const &data) {
        // devicememory.mapMemory(offset, size)
        std::uint8_t *pData = static_cast<std::uint8_t *>(deviceMemory.mapMemory(0, size));
        std::memcpy(pData, &data, sizeof(data));
        deviceMemory.unmapMemory();
    }
    template <typename T>
    auto createBuffer(vk::BufferUsageFlagBits usageFlags, T const &data) -> std::pair<vk::raii::DeviceMemory, vk::raii::Buffer> {
        auto bufferCreateInfo = vk::BufferCreateInfo{
            .flags = vk::BufferCreateFlags(),
            .size = sizeof(data),
            .usage = usageFlags,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
        };

        if (graphicsAndPresentQueueFamilyIndex.at(0) != graphicsAndPresentQueueFamilyIndex.at(1)) {
            bufferCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
            bufferCreateInfo.queueFamilyIndexCount = 2;
            bufferCreateInfo.pQueueFamilyIndices = graphicsAndPresentQueueFamilyIndex.data();
        }

        auto buffer = vk::raii::Buffer(device, bufferCreateInfo);

        vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

        std::uint32_t typeIndex = findMemoryType(
            physicalDevice.getMemoryProperties(),
            memoryRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::raii::DeviceMemory deviceMemory(device,
            vk::MemoryAllocateInfo{
                .allocationSize = memoryRequirements.size,
                .memoryTypeIndex = typeIndex});

        copyToDevice(deviceMemory, memoryRequirements.size, data);

        buffer.bindMemory(*deviceMemory, 0);

        return {std::move(deviceMemory), std::move(buffer)};
    }

  private:
    // Order of these Vulkan Variables are IMPORTANT
    // Movement of these variables can potentially lead to an incorrect destruction
    // order, which then can cause various Vulkan validation issues.

    // vk::raii::Context
    // This class does not exist in the C API or in the C++ vk namespace
    // It's here to handle the few functions that are not bound to
    // a VkInstance or a VkDevice. For example:
    // context.enumerateInstanceVersion()
    // is a function that isn't bound to an instance or device.
    const vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
#if defined(VULKAN_DEBUG)
    vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger = nullptr;
#endif
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    // For now we will leave everything to the defaults (off), as we don't
    // we rely on any optional features at the moment, so no need to enable any.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#features
    const vk::PhysicalDeviceFeatures physicalDeviceFeatures;
    vk::raii::Device device = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    // std::pair<std::uint32_t, std::uint32_t> graphicsAndPresentQueueFamilyIndex;
    std::array<std::uint32_t, 2> graphicsAndPresentQueueFamilyIndex{};
    vk::raii::CommandPool commandPool = nullptr;
    vk::raii::CommandBuffers commandBuffers = nullptr;
    vk::raii::CommandBuffer commandBuffer = nullptr;
    vk::Format colorFormat;
    vk::Format depthFormat;
    vk::raii::SwapchainKHR swapChain = nullptr;
    vk::Extent2D extent;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::raii::ImageView> imageViews;
    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthMemory = nullptr;
    vk::raii::ImageView depthView = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    vk::raii::DescriptorSets descriptorSets = nullptr;
    vk::raii::DescriptorSet descriptorSet = nullptr;
    vk::raii::DeviceMemory uniformBufferMemory = nullptr;
    vk::raii::Buffer uniformDataBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::RenderPass renderPass = nullptr;
    vk::raii::ShaderModule vertexShaderModule = nullptr;
    vk::raii::ShaderModule fragmentShaderModule = nullptr;
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::Pipeline graphicsPipeline = nullptr;
    vk::raii::PipelineCache graphicsPipelineCache = nullptr;
    vk::raii::Queue graphicsQueue = nullptr;
    vk::raii::Queue presentQueue = nullptr;
    // std::vector<vk::raii::Semaphore> recycledSemaphores;
    //  TODO This is Temp VVV
    vk::raii::Semaphore imageAcquiredSemaphore = nullptr;
    vk::raii::Fence drawFence = nullptr;
    vk::Result result;
    std::uint32_t imageIndex;

    // FenceTimeout specifies how long the function waits, in nanoseconds, if no image is available
    // https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkAcquireNextImageKHR.html
    const std::uint64_t fenceTimeout = 100000000; // 100000000 nanoseconds = 0.1 seconds

    // Shader and Frag code for the renderer
    // Look away, I am commiting c++ crimes
    static auto constexpr vertShader = std::to_array<std::uint32_t>({
#include "vulkantut.vert.inc"
    });

    static auto constexpr fragShader = std::to_array<std::uint32_t>({
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
    static auto constexpr coloredCubeData = std::to_array<VertexPC>({
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

    const std::set<std::string> desiredInstanceLayers {
#if defined(VULKAN_DEBUG)
        // https://vulkan.lunarg.com/doc/view/latest/linux/khronos_validation_layer.html
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    // NOTE: Not all platform includes below will be made use of yet or at all in this code
    //  but listing them here for future experiments/use
    const std::set<std::string> desiredInstanceExtensions {
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
#if defined(VK_USE_PLATFORM_MACOS_MVK)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_MVK_macos_surface.html
        "VK_MVK_macos_surface",
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_win32_surface.html
        "VK_KHR_win32_surface",
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_wayland_surface.html
        "VK_KHR_wayland_surface",
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_xlib_surface.html
        "VK_KHR_xlib_surface",
#endif
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_acquire_xlib_display.html
        "VK_EXT_acquire_xlib_display",
        "VK_EXT_direct_mode_display",
        "VK_KHR_display",
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_xcb_surface.html
        "VK_KHR_xcb_surface",
#endif
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_surface.html
        "VK_KHR_surface"
        // clang-format on
    };

    const std::set<std::string> desiredDeviceExtensions{
        "VK_KHR_swapchain"};

    auto findMemoryType(vk::PhysicalDeviceMemoryProperties const &, std::uint32_t, vk::MemoryPropertyFlags)
        -> std::uint32_t;
    auto enumerateExtensions(std::vector<vk::ExtensionProperties> const &, std::set<std::string> const &) -> std::vector<char const *>;
    auto enumerateLayers(std::vector<vk::LayerProperties> const &, std::set<std::string> const &) -> std::vector<char const *>;
    auto getQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &, vk::QueueFlagBits) -> std::uint32_t;
    auto getGraphicsAndPresentQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const &, std::uint32_t) -> std::array<std::uint32_t, 2>;
    // TODO For our current example the cube size/shape is stored in a uniform
    // buffer (?)
    // So we should create two functions.
    // 1. Function to create buffers (e.g. I pass vk::BufferUsageFlagBits::eUniformBuffer
    // to the function and it'll create a uniformbuffer for me
    // 2. Upload data to device (pass buffer and data, e.g. Uniform buffer and our cube matrix)
};
#endif
