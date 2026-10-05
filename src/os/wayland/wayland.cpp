// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>

#include <wayland-client.h>

#include "wayland.hpp"

#include "xdg-decoration-client-protocol-v1.h"
#include "xdg-shell-client-protocol.h"

wl::Connection::Connection() : m_display(wl_display_connect(nullptr)) {
    std::print("Connected to Wayland Display\n");

    if (m_display == nullptr) {
        throw std::runtime_error("Failed to connect to Wayland display\n");
    }
}

wl::Connection::~Connection() {
    if (m_display != nullptr) {
        wl_display_disconnect(m_display);
    }
}

void wl::Connection::roundtrip() {
    assert(m_display);
    wl_display_roundtrip(m_display);
}

auto wl::Connection::dispatch() -> int {
    assert(m_display);
    return wl_display_dispatch(m_display);
}

void wl::Connection::flush() {
    assert(m_display);
    wl_display_flush(m_display);
}

auto wl::Connection::get_fd() -> int {
    assert(m_display);
    return wl_display_get_fd(m_display);
}

wl::Registry::Registry(wl::Connection &connection) : m_registry(wl_display_get_registry(connection.get())) {
    if (m_registry == nullptr) {
        throw std::runtime_error("Failed to get registry object from the compositor\n");
    }

    static const struct wl_registry_listener registry_listener {
        .global =
            [](void *data, struct wl_registry * /*registry*/, std::uint32_t name, const char *interface,
                std::uint32_t /*version*/) {
                // We cast to access the class data memeber in the C callback function
                static_cast<wl::Registry *>(data)->m_interfaces[interface] = name;
            },
        .global_remove = [](void * /*data*/, struct wl_registry * /*registry*/, std::uint32_t name) {
            std::print("Got a registry removal event for {} \n", name);
        }
    };
    wl_registry_add_listener(m_registry, &registry_listener, this);
}

wl::Registry::~Registry() {
    if (m_registry != nullptr) {
        wl_registry_destroy(m_registry);
    }
}

wl::Compositor::Compositor(Registry &registry) {
    m_compositor = registry.bind<wl_compositor>(&wl_compositor_interface, API_VERSION);
    if (m_compositor == nullptr) {
        throw std::runtime_error("Failed to bind to wl_compositor");
    }
}

wl::Compositor::~Compositor() {
    if (m_compositor != nullptr) {
        wl_compositor_destroy(m_compositor);
    }
}

wl::Shm::Shm(Registry &registry) {
    m_shm = registry.bind<wl_shm>(&wl_shm_interface, API_VERSION);
    if (m_shm == nullptr) {
        throw std::runtime_error("Failed to bind to wl_shm");
    }
}

wl::Shm::~Shm() {
    if (m_shm != nullptr) {
        wl_shm_destroy(m_shm);
    }
}

wl::Seat::Seat(Registry &registry) {
    m_seat = registry.bind<wl_seat>(&wl_seat_interface, API_VERSION);
    if (m_seat == nullptr) {
        throw std::runtime_error("Failed to bind to wl_seat");
    }

    static constexpr struct wl_seat_listener m_listener {
        .capabilities =
            [](void *data, struct wl_seat * /*seat*/, std::uint32_t caps) {
                auto *self = static_cast<wl::Seat *>(data);

                // Capabilities can both appear and disappear
                self->m_pointer_supported = false;
                self->m_keyboard_supported = false;
                self->m_touch_supported = false;

                // Check for what is supported right now
                // TODO Actually setup or release said devices.
                if ((caps & WL_SEAT_CAPABILITY_POINTER) != 0U) {
                    self->m_pointer_supported = true;
                }
                if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) != 0U) {
                    self->m_keyboard_supported = true;
                }
                if ((caps & WL_SEAT_CAPABILITY_TOUCH) != 0U) {
                    self->m_touch_supported = true;
                }
            },
        .name = [](void *data, struct wl_seat * /*seat*/, const char *name) {
            static_cast<wl::Seat *>(data)->m_name = name;
        }
    };

    wl_seat_add_listener(m_seat, &m_listener, this);
}

wl::Seat::~Seat() {
    if (m_seat != nullptr) {
        wl_seat_destroy(m_seat);
    }
}

xdg::wm::Base::Base(wl::Registry &registry) {
    m_base = registry.bind<xdg_wm_base>(&xdg_wm_base_interface, API_VERSION);
    if (m_base == nullptr) {
        throw std::runtime_error("Failed to bind to xdg_wm_base");
    }

    static constexpr struct xdg_wm_base_listener xdg_wm_base_listener {
        .ping = [](void *data, struct xdg_wm_base * /*base*/, std::uint32_t serial) {
            static_cast<xdg::wm::Base *>(data)->pong(serial);
        }
    };

    xdg_wm_base_add_listener(m_base, &xdg_wm_base_listener, this);
}

xdg::wm::Base::~Base() {
    if (m_base != nullptr) {
        xdg_wm_base_destroy(m_base);
    }
}

void xdg::wm::Base::pong(std::uint32_t serial_number) { xdg_wm_base_pong(m_base, serial_number); }

xdg::DecorationManager::DecorationManager(wl::Registry &registry) {
    m_decoration_manager =
        registry.bind<zxdg_decoration_manager_v1>(&zxdg_decoration_manager_v1_interface, API_VERSION);
    if (m_decoration_manager == nullptr) {
        throw std::runtime_error("xdg::DecorationManager: could not bind to zxdg_decoration_manager_v1");
    }
}

xdg::DecorationManager::~DecorationManager() {
    if (m_decoration_manager != nullptr) {
        zxdg_decoration_manager_v1_destroy(m_decoration_manager);
    }
}

auto xdg::DecorationManager::is_supported(wl::Registry &registry) -> bool {
    return (registry.has_interface("zxdg_decoration_manager_v1"));
}

WaylandInternal::Display::Display()
    : m_connection(std::make_unique<wl::Connection>()), m_registry(std::make_unique<wl::Registry>(*m_connection)) {
    // Wait for the "Initial" set of globals to appear
    m_connection->roundtrip();

    m_compositor = std::make_unique<wl::Compositor>(*m_registry);
    m_shm = std::make_unique<wl::Shm>(*m_registry);
    m_seat = std::make_unique<wl::Seat>(*m_registry);
    m_wm_base = std::make_unique<xdg::wm::Base>(*m_registry);
    if (xdg::DecorationManager::is_supported(*m_registry)) {
        m_decoration_manager = std::make_unique<xdg::DecorationManager>(*m_registry);
    }
}

Wayland::Wayland()
    : m_display(std::make_unique<WaylandInternal::Display>()),
      m_surface(wl_compositor_create_surface(m_display->get_compositor().get())),
      m_xdg_surface(xdg_wm_base_get_xdg_surface(m_display->get_wm_base().get(), m_surface.get())),
      m_toplevel(xdg_surface_get_toplevel(m_xdg_surface.get())), m_decoration(nullptr) {
    if (m_surface == nullptr) {
        throw std::runtime_error("Failed to create surface ");
    }

    if (m_xdg_surface == nullptr) {
        throw std::runtime_error("Failed to create xdg_surface");
    }

    if (m_toplevel == nullptr) {
        throw std::runtime_error("Failed to create xdg_toplevel");
    }

    if (m_display->has_decoration_manager()) {
        m_decoration.reset(zxdg_decoration_manager_v1_get_toplevel_decoration(
            m_display->get_decoration_manager().get(), m_toplevel.get()));

        if (m_decoration == nullptr) {
            throw std::runtime_error("Failed to create xdg_toplevel_decoration");
        }
    }

    // xdg_surface_listener is where we handle resizes, but xdg_toplevel_listener
    // is where we handle the *hints* from the compositor to resize.
    static constexpr struct xdg_surface_listener xdg_listener {
        .configure = [](void *data, struct xdg_surface * /*surface*/, std::uint32_t serial) {
            auto *self = static_cast<Wayland *>(data);
            self->m_last_configure_event_serial = serial;
            self->m_configure_event_pending = true;

            self->ack_configure();
            // TODO Basically what this should do is ack configure first
            // Then if state has changed (size changed, maximized, etc), then redraw
            //
            // ack -> resize -> draw
            //
            // TODO/Note: Vulkan docs state
            // If the application wishes to synchronize any window changes with a particular frame, such requests must
            // be sent to the Wayland display server prior to calling vkQueuePresentKHR.
        }
    };

    // xdg_toplevel_listener is where the compositor *hints* to the client what
    // size it should be. xdg_surface_listener is where it actually gets changed.
    static constexpr struct xdg_toplevel_listener toplevel_listener {
        .configure =
            [](void *data, struct xdg_toplevel * /*toplevel*/, std::int32_t width, std::int32_t height,
                struct wl_array *states) {
                auto *self = static_cast<Wayland *>(data);
                auto *arr_ptr = static_cast<std::uint32_t *>(states->data);

                // wl_array_for_each handling in C++ based on the below KDE lines
                // https://github.com/KDE/kwayland/blob/bd1988ce6f568edd6b29bd6b1220919fb91d293a/src/client/xdgshell_stable.cpp#L310
                for (std::size_t i = 0; i < states->size / sizeof(std::uint32_t); i++) {
                    // Find out our current window state.
                    switch (arr_ptr[i]) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    case XDG_TOPLEVEL_STATE_MAXIMIZED:
                        self->state.maximized = true;
                        break;
                    case XDG_TOPLEVEL_STATE_FULLSCREEN:
                        self->state.fullscreen = true;
                        break;
                    case XDG_TOPLEVEL_STATE_RESIZING:
                        self->state.resizing = true;
                        break;
                    case XDG_TOPLEVEL_STATE_ACTIVATED:
                        self->state.activated = true;
                        break;
                    case XDG_TOPLEVEL_STATE_TILED_LEFT:
                        self->state.tiled_left = true;
                        break;
                    case XDG_TOPLEVEL_STATE_TILED_RIGHT:
                        self->state.tiled_right = true;
                        break;
                    case XDG_TOPLEVEL_STATE_TILED_TOP:
                        self->state.tiled_top = true;
                        break;
                    case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
                        self->state.tiled_bottom = true;
                        break;
                    case XDG_TOPLEVEL_STATE_SUSPENDED:
                        self->state.suspended = true;
                        break;
                    default:
                        break;
                    }
                }

                // Lets first store our previous size, incase we need it
                self->m_previous_width = self->state.width;
                self->m_previous_height = self->state.height;

                // Handle client dimensions
                // Note: .configure_bounds may be sent prior to .configure,
                // setting our max size. Max bounds size may change at any
                // point still.
                if (width == 0 || height == 0) {
                    // Compositor is deferring to us
                    // We will just fall back to the values inside the
                    // Window.WindowState struct. Which will either be the
                    // default width/height we have set in the struct or what
                    // ever it may have been changed to after creation.
                    return;
                }

                // Check if we have been told to restrain the window geometry,
                // if we don't then default to max numeric limit.
                auto max_width = self->state.recommended_max_width > 0 ? self->state.recommended_max_width
                                                                       : std::numeric_limits<std::uint32_t>::max();
                auto max_height = self->state.recommended_max_height > 0 ? self->state.recommended_max_height
                                                                         : std::numeric_limits<std::uint32_t>::max();

                // Just to silence the implicit conversion warnings
                auto req_width = static_cast<std::uint32_t>(width);
                auto req_height = static_cast<std::uint32_t>(height);
                // Set our window size.
                // Note: The window doesn't get resized till xdg_surface.configure gets called.
                self->state.width = std::clamp(req_width, 0U, max_width);
                self->state.height = std::clamp(req_height, 0U, max_height);

                self->m_configure_event_pending = true;
                std::print("Received: Configure request: {} x {} \n", width, height);
            },
        .close =
            [](void *data, struct xdg_toplevel * /*toplevel*/) {
                static_cast<Wayland *>(data)->state.close_requested = true;
                std::print("Received: Close request\n");
            },
        .configure_bounds =
            [](void *data, struct xdg_toplevel * /*toplevel*/, std::int32_t width, std::int32_t height) {
                auto *self = static_cast<Wayland *>(data);
                self->state.recommended_max_width = static_cast<std::uint32_t>(width);
                self->state.recommended_max_height = static_cast<std::uint32_t>(height);
                std::print("Received: Recommended max dimensions: {} x {} \n", width, height);
            },
        .wm_capabilities = [](void *data, struct xdg_toplevel * /*toplevel*/, struct wl_array *capabilities) {
            auto *self = static_cast<Wayland *>(data);
            auto *arr_ptr = static_cast<std::uint32_t *>(capabilities->data);

            for (std::size_t i = 0; i < capabilities->size / sizeof(std::uint32_t); i++) {
                switch (arr_ptr[i]) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                case XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU:
                    self->state.capabilities.window_menu_supported = true;
                    break;
                case XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE:
                    self->state.capabilities.maximize_supported = true;
                    break;
                case XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN:
                    self->state.capabilities.fullscreen_suported = true;
                    break;
                case XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE:
                    self->state.capabilities.minimize_supported = true;
                    break;
                default:
                    break;
                }
            }
        }
    };

    static constexpr zxdg_toplevel_decoration_v1_listener toplevel_decoration_listener{
        .configure = [](void *data, zxdg_toplevel_decoration_v1 * /*toplevel_decoration*/, uint32_t mode) {
            auto *self = static_cast<Wayland *>(data);

            switch (mode) {
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
                self->state.window_decoration_mode = false;
                break;
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
                self->state.window_decoration_mode = true;
                break;
            default:
                break;
            }
        }};

    xdg_surface_add_listener(m_xdg_surface.get(), &xdg_listener, this);
    xdg_toplevel_add_listener(m_toplevel.get(), &toplevel_listener, this);
    if (m_decoration != nullptr) {
        zxdg_toplevel_decoration_v1_add_listener(m_decoration.get(), &toplevel_decoration_listener, this);
    }

    // Must commit once before a window can appear (so then Vulkan can start rendering).
    wl_surface_commit(m_surface.get());
    m_display->get_connection().roundtrip();
    // wl_surface_commit(m_surface.get());
    //  TODO Put this second commit here commented so I can test weather it
}

void Wayland::ack_configure() {
    if (!m_configure_event_pending) {
        throw std::logic_error("xdg_surface: ack_configure() but no configure event is pending");
    }
    xdg_surface_ack_configure(m_xdg_surface.get(), m_last_configure_event_serial);
    m_configure_event_pending = false;
}

auto Wayland::create_surface(vk::raii::Instance &instance) -> vk::raii::SurfaceKHR {
    return instance.createWaylandSurfaceKHR(vk::WaylandSurfaceCreateInfoKHR{.flags = vk::WaylandSurfaceCreateFlagsKHR(),
        .display = m_display->get_connection().get(),
        .surface = m_surface.get()});
}

auto Wayland::is_minimized() const -> bool {}
auto Wayland::is_maximized() const -> bool {}
auto Wayland::is_fullscreen() const -> bool {}
auto Wayland::is_pointer_supported() const -> bool {}
auto Wayland::is_keyboard_supported() const -> bool {}
auto Wayland::is_touch_supported() const -> bool {}
void Wayland::close() {}
void Wayland::minimize() {}
void Wayland::maximize() {}
void Wayland::fullscreen() {}

void Wayland::set_title(const std::string &title) {
    // TODO Sanitize input
    xdg_toplevel_set_title(m_toplevel.get(), title.c_str());

    //  xdg_toplevel_set_app_id(m_toplevel.get(), app_id.c_str());
}
void Wayland::set_window_size(std::uint32_t, std::uint32_t) {}
void Wayland::set_window_size_recommended_min(std::uint32_t, std::uint32_t) {}
void Wayland::set_window_size_recommended_max(std::uint32_t, std::uint32_t) {}

auto Wayland::get_window_size() -> std::pair<std::uint32_t, std::uint32_t> {
    // TODO Make sure we pull in the current info before sending it off
    // Also make sure the we pull the current size
    // e.g. window is 200x200 but the compositor has hinted to go 300x200,
    // but we have yet to actually resize.
    return {state.width, state.height};
}

auto Wayland::get_window_size_recommended_min() -> std::pair<std::uint32_t, std::uint32_t> {}
auto Wayland::get_window_size_recommended_max() -> std::pair<std::uint32_t, std::uint32_t> {}

//auto Wayland::poll_events() -> std::generator<std::expected<Event, std::string>> {
auto Wayland::poll_events() -> std::generator<Event> {
    assert(m_display);

    // TODO This was set to true before, but it shouldn't of been. set to false, now I need to test and see if anything
    // is
    bool need_redraw = false;
    while (m_display->get_connection().dispatch() != 1) {
        if (is_close_requested()) {
            // TODO Handle close requested here
            // e.g. Save dialog maybe?
            break;
        }

        // TODO Is this *if* needed? check and decided.
        // I have stake out some of the width/height checks here so I am not sure what else I need this if for???
        if (is_configure_event_pending()) {
            ack_configure();
            need_redraw = true;
        }

        if (need_redraw) {
            // TODO get frames here
            wl_surface_commit(m_surface.get());
        }
    }
}
