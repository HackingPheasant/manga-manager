// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

// #include <chrono>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <utility>

#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xproto.h>

#include "x11.hpp"

// libxcb Docs
// https://www.x.org/releases/current/doc/libxcb/tutorial/index.html
X11::X11()
    : connection(xcb_connect(nullptr, nullptr)),
      screen(xcb_setup_roots_iterator(xcb_get_setup(connection.get())).data) {
    // Open the connection to the X server. ^^
    // Provided NULL so xcb_connect can use the DISPLAY enviroment variable
    // and default to the first screen
    if (xcb_connection_has_error(connection.get()) != 0) {
        throw std::runtime_error("Failed to connect to an X server\n");
    }

    if (screen == nullptr) {
        throw std::runtime_error("Failed to get the current screen\n");
    }

    // Ask for our window's ID
    window = xcb_generate_id(connection.get());

    constexpr std::uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    // XCB_NONE and XCB_COPY_FROM_PARENT are both defined as 0L in xcb.h and
    // since we aren't using anything else aside from the event masks we will
    // just default these values to nothing of importance to silence the
    // compiler warnings.
    const xcb_create_window_value_list_t value_list{.background_pixmap = XCB_COPY_FROM_PARENT,
        .background_pixel = screen->black_pixel,
        .border_pixmap = XCB_COPY_FROM_PARENT,
        .border_pixel = XCB_COPY_FROM_PARENT,
        .bit_gravity = XCB_COPY_FROM_PARENT,
        .win_gravity = XCB_COPY_FROM_PARENT,
        .backing_store = XCB_COPY_FROM_PARENT,
        .backing_planes = XCB_COPY_FROM_PARENT,
        .backing_pixel = XCB_COPY_FROM_PARENT,
        .override_redirect = XCB_COPY_FROM_PARENT,
        .save_under = XCB_COPY_FROM_PARENT,
        .event_mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS |
                      XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
                      XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE |
                      XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE,
        .do_not_propogate_mask = XCB_COPY_FROM_PARENT,
        .colormap = XCB_COPY_FROM_PARENT,
        .cursor = XCB_COPY_FROM_PARENT};

    // Create the window
    xcb_create_window_aux(connection.get(),       // Connection to the X server
        XCB_COPY_FROM_PARENT,                     // Screen depth (Same as root)
        window,                                   // ID of the window
        screen->root,                             // ID of the parent window
        0,                                        // Top-left x-coordinate
        0,                                        // Top-left y-coordinate
        static_cast<std::uint16_t>(state.width),  // Window width in pixels
        static_cast<std::uint16_t>(state.height), // Window height in pixels
        10,                                       // Window border width in pixels
        XCB_WINDOW_CLASS_INPUT_OUTPUT,            // Class
        screen->root_visual,                      // Visual
        value_mask,                               // Value mask
        &value_list);                             // Value list

    // Map the window on the screen
    xcb_map_window(connection.get(), window);

    // Make sure commands are sent before we do anything else, so window is shown
    xcb_flush(connection.get());

    // XCB is asynchronous, but because of this we can (and have accidentaly
    // done so in the past) pull the window dimensions to quickly after
    // window creation leading to us incorrectly creating the swapchain
    // with the wrong size. So we will instead just sleep 0.05 seconds prior
    // to finishing this function. This helps mitigate the issue.
    //
    // Past examples of running into said data race issue
    // https://twitter.com/HackingPheasant/status/1475730781750247425
    // https://web.archive.org/web/20211229060434/https://twitter.com/HackingPheasant/status/1475730781750247425
    // std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

auto X11::create_surface(vk::raii::Instance &instance) -> vk::raii::SurfaceKHR {
    return instance.createXcbSurfaceKHR(vk::XcbSurfaceCreateInfoKHR{
        .flags = vk::XcbSurfaceCreateFlagsKHR(), .connection = connection.get(), .window = window});
}

auto X11::is_fullscreen() const -> bool {}
auto X11::is_minimized_supported() const -> bool {}
auto X11::is_maximized_supported() const -> bool {}
auto X11::is_fullscreen_supported() const -> bool {}
auto X11::is_window_menu_supported() const -> bool {}
auto X11::is_pointer_supported() const -> bool {}
auto X11::is_keyboard_supported() const -> bool {}
auto X11::is_touch_supported() const -> bool {}
void X11::close() {}
void X11::minimize() {}
void X11::maximize() {}
void X11::fullscreen() {}

void X11::set_title(const std::string &title) {
    // TODO Sanitize input
    //  Set the title of the window
    xcb_change_property(connection.get(),         // Connection
        XCB_PROP_MODE_REPLACE,                    // Property mode
        window,                                   // Window
        XCB_ATOM_WM_NAME,                         // Property to change
        XCB_ATOM_STRING,                          // Type of the property
        8,                                        // Format of the property (8, 16, 32)
        static_cast<std::uint32_t>(title.size()), // Length of the data parameter
        title.c_str());                           // Data

    //  Set the title of the window iconnifed
    xcb_change_property(connection.get(), XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
        static_cast<std::uint32_t>(title.size()), title.c_str());
}

void X11::set_window_size(std::uint32_t, std::uint32_t) {}
void X11::set_window_size_recommended_min(std::uint32_t, std::uint32_t) {}
void X11::set_window_size_recommended_max(std::uint32_t, std::uint32_t) {}

auto X11::get_window_size() -> std::pair<std::uint32_t, std::uint32_t> {
    // Get the size of the windows underlying drawable dimension
    const std::unique_ptr<xcb_get_geometry_reply_t, x11_deleter> geometry{
        xcb_get_geometry_reply(connection.get(), xcb_get_geometry(connection.get(), window), nullptr)};

    // TODO See if X11 will tell us our max bounds.
    // But for the moment making sure we don't exced the max for
    // uint32_t is good enough.
    state.width =
        std::clamp(static_cast<std::uint32_t>(geometry->width), 0U, std::numeric_limits<std::uint32_t>::max());
    state.height =
        std::clamp(static_cast<std::uint32_t>(geometry->height), 0U, std::numeric_limits<std::uint32_t>::max());

    return {state.width, state.height};
}

auto X11::get_window_size_recommended_min() -> std::pair<std::uint32_t, std::uint32_t> {}
auto X11::get_window_size_recommended_max() -> std::pair<std::uint32_t, std::uint32_t> {}

//auto X11::poll_events() -> std::generator<std::expected<Event, std::string>> {
auto X11::poll_events() -> std::generator<Event> {
    bool need_redraw = false;

    std::unique_ptr<xcb_generic_event_t, x11_deleter> event{xcb_poll_for_event(connection.get())};

    // while ((event = xcb_wait_for_event(connection.get())) != nullptr) {
    // }

    // while (xcb_connection_has_error(connection.get()) != 0) {

    //  while ((event = xcb_poll_for_event(connection.get())) != nullptr) {
    // }
    //}
}
