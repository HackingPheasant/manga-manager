// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#include <chrono>
#include <iostream>
#include <memory>
#include <print>
#include <string>
#include <thread>

#include "vulkan_renderer.hpp"
#include "wsi.hpp"

auto main(int /*argc*/, const char ** /*argv[]*/) -> int try {
    // Parse command line input and intial program setup

    // Testing this for a moment
    // for (auto arg: std::span{argv+1, argv+argc}) {
    //    std::println("arg = {}", arg);
    //}

    const std::string app_name = "Manga Manager";

    auto window = ui::get_wsi_interface();
    window->set_title(app_name);

    // Initialize our renderer (Vulkan) and create a window to render to!
    // TODO Move surface creation  back into vulkan renderer.
    VulkanRender renderer(app_name);
    auto [width, height] = window->get_window_size();

    renderer.setSurface(window->create_surface(renderer.getInstance()))
        .selectPhysicalDevice()
        .initDevice()
        .initSwapchain(width, height)
        .createUniformBuffer()
        .initRenderPass()
        .initFramebuffers()
        .createVertexBuffer()
        .initPipeline()
        .render()
        .present();

    // Main program loop
    for (auto const &event : window->poll_events()) {
        switch (event) {
        case Event::NOOP:
            // TODO
            break;
        case Event::Close:
            // TODO
            std::println("Test: Close Requested! Test");
            break;
        case Event::Destroyed:
            // TODO
            break;
            case Event::Unmapped:
            // TODO
            break;
        case Event::Resizing:
            // TODO
            break;
        case Event::Resized:
            // TODO
            renderer.render().present();
            break;
        case Event::FocusChanged:
            // TODO
            break;
        case Event::VisbilityChanged:
            // TODO
            break;
        case Event::PropertyChanged:
            // TODO
            break;
        case Event::Keyboard:
            // TODO
            break;
        case Event::Mouse:
            // TODO
            break;
        case Event::Pointer:
            // TODO
            break;
        }
    }

    // Get most recent window size before we go using it in our vulkan code
    // This is because even tho we may have created the window at a cer``tain
    // size does not mean the underlying window stack has to honour it.
    // Prime example is tiling window managers!
    // Updates the values in window->extent.{width/height}
    //    window->getCurrentWindowSize();

    // TODO Build a proper event loop, one that handles libinput, Display System (Wayland/X11 etc) etc
    // Refernces
    // https://wayland-book.com/wayland-display/event-loop.html
    // https://github.com/sony/flutter-embedded-linux/blob/master/src/flutter/shell/platform/linux_embedded/window/elinux_window_drm.h
    //
    // Do we have co_routines for input as pointer (mouse), touch and keyboard? and co resume(?) on them in a polling
    // loop??
    /*while (!window->should_close) {
        switch(window->poll_events()) {
            case Event::resize:
                break;

        }

        renderer.render().present()
    }*/

    // Program shutdown routine.

    // This is purely here so we can actually have a moment to see our glorius rendered cube!
    // ALL HAIL THE CUBE
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 0;
} catch (const vk::SystemError &err) {
    std::println(std::cerr, "vk::SystemError: {} ", err.what());
    return -1;
} catch (const std::system_error &err) {
    std::println(std::cerr, "Error: {}:{} - {}\n {}", err.code().category().name(), err.code().value(),
        err.code().message(), err.what());
    return -1;
} catch (const std::exception &err) {
    std::println(std::cerr, "std::exception: {}", err.what());
    return -1;
} catch (...) {
    std::println(std::cerr, "Unknown error");
    return -1;
}
