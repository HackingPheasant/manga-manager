// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#ifndef OS_WSI_H
#define OS_WSI_H

#include <memory>

#include "window-interface.hpp"

#if defined(WSI_ANDROID)
#include "android/window.hpp"
#endif

#if defined(WSI_FUCHSIA)
#include "fuchsia/window.hpp"
#endif

#if defined(WSI_APPLE)
#include "apple/window.hpp"
#endif

#if defined(WSI_WIN32)
#include "windows/window.hpp"
#endif

#if defined(WSI_WAYLAND)
#include "wayland/wayland.hpp"
#endif

#if defined(WSI_X11)
#include "x11/x11.hpp"
#endif

namespace ui {

auto get_wsi_interface() -> std::unique_ptr<WindowInterface> {
#if defined(WSI_ANDROID)
    throw std::runtime_error("Unimplemented");
    return std::make_unique<Android>();
#endif

#if defined(WSI_FUCHSIA)
    throw std::runtime_error("Unimplemented");
    return std::make_unique<Fuchsia>();
#endif

#if defined(WSI_APPLE)
    throw std::runtime_error("Unimplemented");
    return std::make_unique<Apple>();
#endif

#if defined(WSI_WIN32)
    throw std::runtime_error("Unimplemented");
    return std::make_unique<Win32>();
#endif

#if defined(WSI_WAYLAND) && defined(WSI_X11)
    // Decided on how to create a window
    // We should prioritie Wayland over X11
    // Wayland > Xwayland (X11 server on wayland) > X11
    //
    // We do this by cheacking the following enviroment variables
    // - WAYLAND_DISPLAY
    // - DISPLAY
    // If WAYLAND_DISPLAY is set we default to creating a Wayland window, otherwise
    // we will create a X11 (XCB) window which can run on either X11 or Wayland via xwayland
    // Some users can purposly be running wayland but remove the WAYLAND_DISPLAY
    // variable to force apps to run via xwayland for whatever reason. We won't
    // go out of our way to do anything in such instances, other then creating an
    // X11 window as usual.
    if (std::getenv("WAYLAND_DISPLAY")) {
        std::unique_ptr<Wayland> wayland(new Wayland());
        return wayland;
    } else {
        std::unique_ptr<X11> x11(new X11());
        return x11;
    }
#endif

#if defined(WSI_WAYLAND) && !defined(WSI_X11)
    return std::make_unique<Wayland>();
#endif

#if defined(WSI_X11) && !defined(WSI_WAYLAND)
    return std::make_unique<X11>();
#endif
}

} // namespace ui
#endif
