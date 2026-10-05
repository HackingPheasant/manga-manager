// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#ifndef OS_WINDOW_INTERFACE_H
#define OS_WINDOW_INTERFACE_H

#include <cstdint>
#include <generator>
#include <string>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

// Attempt One
enum class Event {
    NOOP,  // No-operation necessary for the window
    Close, // Window is requesting to be closed, can be used to prompt user to confirm to exit or potenially save work
           // or something similar
    Destroyed,        // Window has been destroyed, and can now no longer be used
    Unmapped,         // Window exists but is not currently mapped/requested to be unmapped from the desktop
    Resizing,         // Window is actively being resized
    Resized,          // Window has finished resizing
    FocusChanged,     // Focus changed events: FocusEntered & FocusLeft
    VisbilityChanged, // Potential Visibility states: Unobscured, Partially Obscured, Fully Obscured
    PropertyChanged, // Potential Property changed events: Orientation {0 or 360 (x,y -> x,y)/90 (x,y -> y, -x)/180 (x,y
                     // -> -x,-y)/270 (x,y -> -y,x) }, Tiled {left/right/top/bottom}, State {No State/Minimized(i.e.
                     // Iconified)/Maximized/FullScreen/Active (i.e. has keyboard focus)}, Window Capabilites,
                     // Decoration Mode {Server Side Decoration/Client Side Decoration}
    Keyboard,        // Keyboard events include: Key Down & Up, Key Repeat, Keyboard Mapping Changed
    Mouse,  // Mouse events include: Moving (Motion Data & Raw Motion Data), Clicking (Button Click/Double Click/Triple
            // Click (Left/Right/Middle/Extra Buttons), ButtonPress & Button Release), Dragging (Combo of click+motion),
            // Hovering (MouseEntered/MouseLeft) and Scrolling (Scrollwheel data)
    Pointer // Poiner events include: Pointer Down, Pointer Up. Can be any amount of X pointers at any one time.
    // TODO: Text Events?? &  Hardware configuration change??
};

/* Potential example of
namespace Input {
enum class DeviceType { Keyboard, Mouse, Touch, Gamepad, Joystick, Tablet, Other };
enum class State { Pressed, Released };

namespace Keyboard {} // namespace Keyboard

namespace Mouse {
enum class Button { Left, Right, Middle, Extra1, Extra2 };
enum class Wheel { Vertical, Horizontal };
} // namespace Mouse
} // namespace Input
*/

class WindowInterface {
  protected:
    struct WindowState {
        bool close_requested = false; // User intends to close the window
        bool window_decoration_mode =
            false; // True = Server Side Decorations, False = Client Side Decorations. TODO Move to enum class??
        bool maximized = false;
        bool fullscreen = false;
        bool minimized = false;
        bool resizing = false;
        bool activated = false; // Window decorations should be painted as if window is active. Doesn't
        bool tiled_left = false;
        bool tiled_right = false;
        bool tiled_top = false;
        bool tiled_bottom = false;
        // Do we convert the above into an enum class like so?
        // enum class Tiled { left, right, top, bottom };
        bool suspended = false; // Surface repaint not need because content is not visable
        bool redraw_needed = false;
        std::uint32_t width = 1280;
        std::uint32_t height = 720;
        // Recommended size to constrain to.
        // If set to zero, then min/max size is unknown
        std::uint32_t recommended_min_width = 0;
        std::uint32_t recommended_min_height = 0;
        std::uint32_t recommended_max_width = 0;
        std::uint32_t recommended_max_height = 0;
        // TODO Window Icon here?
        std::string title{"Untitled Window"};
        // std::string app_id;

        struct Capabilities {
            bool window_menu_supported = false; // Rightclick context menu when clicking on decorations
            bool maximize_supported = false;
            bool fullscreen_suported = false;
            bool minimize_supported = false;
            bool pointer_supported = false;
            bool keyboard_supported = false;
            bool touch_supported = false;
        } capabilities;
    } state;

  public:
    // Make it a move only class
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c21-if-you-define-or-delete-any-copy-move-or-destructor-function-define-or-delete-them-all
    WindowInterface() = default;
    WindowInterface(const WindowInterface &) = delete;                // Copy constructor
    WindowInterface(WindowInterface &&) noexcept = delete;            // Move constructor
    WindowInterface &operator=(const WindowInterface &) = delete;     // Copy assignment
    WindowInterface &operator=(WindowInterface &&) noexcept = delete; // Move assignment
    virtual ~WindowInterface() = default;                             // Destructor (Virtual because it's a base class)

    // Required functions to be implemented by derieved classes

    // Vulkan Specific
    virtual auto create_surface(vk::raii::Instance &) -> vk::raii::SurfaceKHR = 0;

    // Check current state or capabilities
    // More often then not majority of these values will be modified in the event loop
    virtual auto is_close_requested() const -> bool = 0;
    virtual auto is_minimized() const -> bool = 0;
    virtual auto is_maximized() const -> bool = 0;
    virtual auto is_fullscreen() const -> bool = 0;
    virtual auto is_minimized_supported() const -> bool = 0;
    virtual auto is_maximized_supported() const -> bool = 0;
    virtual auto is_fullscreen_supported() const -> bool = 0;
    virtual auto is_window_menu_supported() const -> bool = 0;
    virtual auto is_pointer_supported() const -> bool = 0;
    virtual auto is_keyboard_supported() const -> bool = 0;
    virtual auto is_touch_supported() const -> bool = 0;

    // Toggle stat
    virtual void close() = 0;
    virtual void minimize() = 0;
    virtual void maximize() = 0;
    virtual void fullscreen() = 0;

    // Set specific things
    virtual void set_title(const std::string &) = 0;
    // virtual void set_rotation(rotation rotation) = 0;
    // virtual void set_icon(const Icon &) = 0;
    // TODO Use value types or something so its not as easy to messup parameter order
    virtual void set_window_size(std::uint32_t, std::uint32_t) = 0;
    virtual void set_window_size_recommended_min(std::uint32_t, std::uint32_t) = 0;
    virtual void set_window_size_recommended_max(std::uint32_t, std::uint32_t) = 0;

    // get specific things
    virtual auto get_window_size() -> std::pair<std::uint32_t, std::uint32_t> = 0;
    // NOTE: We should get the most recent window size before we go using it in our vulkan code
    // This is because even though we may have created the window at a certain size does not mean
    // the underlying windowing stack has to honour it. Prime example is tiling window managers!
    //
    // This function will update the values stored in window.state.{width/height}
    // then return said values.
    virtual auto get_window_size_recommended_min() -> std::pair<std::uint32_t, std::uint32_t> = 0;
    virtual auto get_window_size_recommended_max() -> std::pair<std::uint32_t, std::uint32_t> = 0;
    // virtual auto get_window_scale_dpi() -> DPI = 0;

    // TODO Do we Poll for events continous or Wait for events and have the thread asleep till an event happens
    // Or do we do Wait with a timeout?
    // virtual void poll_events() = 0;
    // TODO/NOTE: co_yield to yield a value co_return when done.
    // virtual auto poll_events() -> std::generator<std::expected<Event, std::string>> = 0;
    virtual auto poll_events() -> std::generator<Event> = 0;
};

#endif // UI_WINDOW_INTERFACE_H
