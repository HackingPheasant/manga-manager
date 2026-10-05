// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#ifndef X_1_X11_H
#define X_1_X11_H

#include <memory>
#include <string>
#include <utility>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>

#include "../window-interface.hpp"

// Used with std::unique_ptr
struct x11_deleter {
    void operator()(xcb_connection_t *connection) const noexcept { xcb_disconnect(connection); }
    // Maybe this one isnt needed but the one below.
    // Covers the xcb*_reply_t related structs that the xcb lib returns
    void operator()(void *p) const noexcept { std::free(p); }
};

class X11 : public WindowInterface {
  protected:
    std::unique_ptr<xcb_connection_t, x11_deleter> connection;
    xcb_screen_t *screen;
    xcb_window_t window{};

    // X11 exclusive functions
    void get_x11_window_size();

  public:
    X11();
    ~X11() = default;

    // Vulkan Specific
    auto create_surface(vk::raii::Instance &) -> vk::raii::SurfaceKHR final;
    
    // Check current state or capabilities
    // TODO See how to check if x11 window  requests closed. /shrug
    // https://www.x.org/releases/X11R7.7/doc/libxcb/tutorial/index.html#expose
    auto is_close_requested() const  -> bool final { return this->state.close_requested; }
    // TODO Create functions for all that don't have final added onto it yet.
    auto is_minimized() const -> bool final { return this->state.minimized; }
    auto is_maximized() const -> bool final { return this->state.maximized; }
    auto is_fullscreen() const -> bool;
    auto is_minimized_supported() const -> bool;
    auto is_maximized_supported() const -> bool;
    auto is_fullscreen_supported() const -> bool;
    auto is_window_menu_supported() const -> bool;
    auto is_pointer_supported() const -> bool;
    auto is_keyboard_supported() const -> bool;
    auto is_touch_supported() const -> bool;

    // Toggle states
    void close();
    void minimize();
    void maximize();
    void fullscreen();

    // Set specific things
    void set_title(const std::string &) final;
    // virtual void set_rotation(rotation rotation) = 0;
    // virtual void set_icon(const Icon &) = 0;
    // TODO Use value types or something so its not as easy to messup parameter order
    void set_window_size(std::uint32_t, std::uint32_t);
    void set_window_size_recommended_min(std::uint32_t, std::uint32_t);
    void set_window_size_recommended_max(std::uint32_t, std::uint32_t);

    // get specific things
    auto get_window_size() -> std::pair<std::uint32_t, std::uint32_t> final;
    auto get_window_size_recommended_min() -> std::pair<std::uint32_t, std::uint32_t>;
    auto get_window_size_recommended_max() -> std::pair<std::uint32_t, std::uint32_t>;
    // virtual auto get_window_scale_dpi() -> DPI = 0;

    //auto poll_events() -> std::generator<std::expected<Event, std::string>> final;
    auto poll_events() -> std::generator<Event> final;
};

#endif
