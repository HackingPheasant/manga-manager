#include <iostream>

#include "vulkan_renderer.h"
#include "window.h"

auto main() -> int try {
    const std::string AppName = "Manga Manager";

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

    // Initialize Vulkan
    // This setups all the necessary boilerplate code needed to render
    VulkanRender renderer(AppName);
    renderer.createSurface(window.window);
    renderer.selectPhysicalDevice();
    renderer.initDevice();
    // Get most recent window size before we go using it in our vulkan code
    // Updates the values in window.extent.{width/height}
    window.getCurrentWindowSize();
    renderer.initSwapchain(window.extent.width, window.extent.height);
    renderer.createUniformBuffer();
    renderer.initRenderPass();
    renderer.initFramebuffers();
    renderer.createVertexBuffer();
    renderer.initPipeline();
 
    renderer.render();
    renderer.present();

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
