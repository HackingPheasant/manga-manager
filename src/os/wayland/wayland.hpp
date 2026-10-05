// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#ifndef WAYLAND_WAYLAND_H
#define WAYLAND_WAYLAND_H

#include <cstdint>
#include <map>
#include <memory>
#include <print>
#include <string>
#include <utility>

// libwayland client api docs
// https://wayland.freedesktop.org/docs/html/apb.html
#include <wayland-client.h>

#include "xdg-decoration-client-protocol-v1.h"
#include "xdg-shell-client-protocol.h"

#include "../../window-interface.hpp"

// Used with std::unique_ptr
struct wayland_deleter {
    void operator()(wl_surface *wl_surface) const noexcept { wl_surface_destroy(wl_surface); }
    void operator()(xdg_surface *xdg_surface) const noexcept { xdg_surface_destroy(xdg_surface); }
    void operator()(xdg_toplevel *xdg_toplevel) const noexcept { xdg_toplevel_destroy(xdg_toplevel); }
    void operator()(zxdg_toplevel_decoration_v1 *xdg_toplevel_decoration) const noexcept {
        zxdg_toplevel_decoration_v1_destroy(xdg_toplevel_decoration);
    }
};

// So anything that is created from the registry is spun-off into its own class
// and encapsulated in an overall display class. Anything derieved from those
// are just in the toplevel Wayland class with custom unique_ptr deleters
namespace wl {

// Base class because wayland objects should not/cannot be copied or trivally constructed
class WaylandObject {
  public:
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c21-if-you-define-or-delete-any-copy-move-or-destructor-function-define-or-delete-them-all
    WaylandObject() = default;
    WaylandObject(const WaylandObject &) = delete;                // Copy constructor
    WaylandObject(WaylandObject &&) noexcept = delete;            // Move constructor
    WaylandObject &operator=(const WaylandObject &) = delete;     // Copy assignment
    WaylandObject &operator=(WaylandObject &&) noexcept = delete; // Move assignment
    virtual ~WaylandObject() = default;                           // Destructor (Virtual because it's a base class)
};

// Wraps wl_display, which represents a connection to the compositor
class Connection : public WaylandObject {
  private:
    struct wl_display *m_display = nullptr;

  public:
    Connection();
    ~Connection();
    auto get() -> wl_display * { return m_display; }
    void roundtrip();
    auto dispatch() -> int;
    void flush();
    auto get_fd() -> int;
};

class Registry : public WaylandObject {
  protected:
    struct wl_registry *m_registry = nullptr;
    std::map<std::string, std::uint32_t> m_interfaces;

  public:
    Registry(Connection &);
    ~Registry();
    auto operator*() -> wl_registry * { return m_registry; }
    template <class T> T *bind(const wl_interface *interface, std::uint32_t version) {
        if (auto search = m_interfaces.find(interface->name); search != m_interfaces.end()) {
            std::print("Bound interface: {} \n", interface->name);
            // TODO Bind to minimum api version or higher e.g. version > API_VERSION
            return static_cast<T *>(wl_registry_bind(m_registry, search->second, interface, version));
        } else {
            std::print("Failed to bind interface: {} \n", interface->name);
            return nullptr;
        }
    }

    bool has_interface(std::string interface_name) { return m_interfaces.contains(interface_name); }
};

class Compositor : public WaylandObject {
  protected:
    struct wl_compositor *m_compositor = nullptr;

  public:
    const std::uint32_t API_VERSION = 5;
    Compositor(Registry &);
    ~Compositor();
    auto get() -> wl_compositor * { return m_compositor; }
};

class Shm : public WaylandObject {
  protected:
    struct wl_shm *m_shm = nullptr;

  public:
    const std::uint32_t API_VERSION = 1;
    Shm(Registry &);
    ~Shm();
    auto get() -> wl_shm * { return m_shm; }
};

class Seat : public WaylandObject {
  protected:
    wl_seat *m_seat = nullptr;
    std::string m_name = "";
    bool m_pointer_supported = false;
    bool m_keyboard_supported = false;
    bool m_touch_supported = false;

  public:
    const std::uint32_t API_VERSION = 7;
    Seat(Registry &);
    ~Seat();
    auto get() -> wl_seat * { return m_seat; }
    auto get_name() const -> std::string { return m_name; }
    bool is_pointer_supported() const { return m_pointer_supported; }
    bool is_keyboard_supported() const { return m_keyboard_supported; }
    bool is_touch_supported() const { return m_touch_supported; }
};
} // namespace wl

namespace xdg {
namespace wm {

class Base : public wl::WaylandObject {
  protected:
    struct xdg_wm_base *m_base = nullptr;
    const std::uint32_t API_VERSION = 5;

  public:
    Base(wl::Registry &);
    ~Base();
    auto get() -> xdg_wm_base * { return m_base; }
    void pong(std::uint32_t);
};
} // namespace wm

class DecorationManager : public wl::WaylandObject {
  protected:
    struct zxdg_decoration_manager_v1 *m_decoration_manager = nullptr;
    const std::uint32_t API_VERSION = 1;

  public:
    DecorationManager(wl::Registry &registry);
    ~DecorationManager();
    auto get() -> zxdg_decoration_manager_v1 * { return m_decoration_manager; }
    static bool is_supported(wl::Registry &registry);
};

} // namespace xdg

namespace WaylandInternal {

class Display : public wl::WaylandObject {
  protected:
    std::unique_ptr<wl::Connection> m_connection;
    std::unique_ptr<wl::Registry> m_registry;
    std::unique_ptr<wl::Compositor> m_compositor;
    std::unique_ptr<wl::Shm> m_shm;
    std::unique_ptr<wl::Seat> m_seat;
    std::unique_ptr<xdg::wm::Base> m_wm_base;
    std::unique_ptr<xdg::DecorationManager> m_decoration_manager;

  public:
    Display();
    ~Display() = default;
    auto get_connection() -> wl::Connection & { return *m_connection; }
    auto get_registry() -> wl::Registry & { return *m_registry; }
    auto get_compositor() -> wl::Compositor & { return *m_compositor; }
    auto get_shm() -> wl::Shm & { return *m_shm; }
    auto get_seat() -> wl::Seat & { return *m_seat; }
    auto get_wm_base() -> xdg::wm::Base & { return *m_wm_base; }
    auto get_decoration_manager() -> xdg::DecorationManager & { return *m_decoration_manager; }
    auto has_decoration_manager() -> bool { return !!m_decoration_manager; }
};
} // namespace WaylandInternal

class Wayland : public WindowInterface {
  protected:
    std::unique_ptr<WaylandInternal::Display> m_display;
    std::unique_ptr<wl_surface, wayland_deleter> m_surface;
    std::unique_ptr<xdg_surface, wayland_deleter> m_xdg_surface;
    std::unique_ptr<xdg_toplevel, wayland_deleter> m_toplevel;
    std::unique_ptr<zxdg_toplevel_decoration_v1, wayland_deleter> m_decoration;

    std::uint32_t m_previous_width = 0;
    std::uint32_t m_previous_height = 0;
    std::uint32_t m_last_configure_event_serial = 0;
    bool m_configure_event_pending = false;
    bool m_redraw_needed = false;

    // Wayland exclusive functions
    bool is_configure_event_pending() const { return m_configure_event_pending; }
    void ack_configure();
    // void set_server_side_mode();

  public:
    Wayland();
    ~Wayland() = default;

    // Vulkan Specific
    auto create_surface(vk::raii::Instance &) -> vk::raii::SurfaceKHR final;

    // Check current state or capabilities
    // Note: A lot of these are handled by the wayland callback/listeners
    // created during the inital wayland object creation. Meaning we can just
    // return the values from the struct containing said data, without doing
    // any extra work.
    auto is_close_requested() const -> bool final { return this->state.close_requested; }
    // TODO Create functions for all that don't have final added onto it yet.
    auto is_minimized() const -> bool;
    auto is_maximized() const -> bool;
    auto is_fullscreen() const -> bool;
    auto is_minimized_supported() const -> bool final { return this->state.capabilities.minimize_supported; }
    auto is_maximized_supported() const -> bool final { return this->state.capabilities.maximize_supported; }
    auto is_fullscreen_supported() const -> bool final { return this->state.capabilities.fullscreen_suported; }
    auto is_window_menu_supported() const -> bool final { return this->state.capabilities.window_menu_supported; }
    // TODO the following 3 require
    // m_display->get_seat().is_pointer_supported() and then put that value into our top level capabilities struct
    auto is_pointer_supported() const -> bool;
    auto is_keyboard_supported() const -> bool;
    auto is_touch_supported() const -> bool;

    // Toggle stat
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
