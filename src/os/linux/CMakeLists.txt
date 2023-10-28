add_library("os_linux")
add_library("os::linux" ALIAS "os_linux")

target_link_libraries("os_linux" PUBLIC
    project::options
)

if(BUILD_LINUX_X11_SUPPORT)
    add_subdirectory(x11)
    target_link_libraries("os_linux" PUBLIC
        linux::x11
    )
endif()

if(BUILD_LINUX_WAYLAND_SUPPORT)
    add_subdirectory(wayland)
    target_link_libraries("os_linux" PUBLIC
        linux::wayland
    )
endif()

if(NOT BUILD_LINUX_X11_SUPPORT AND NOT BUILD_LINUX_WAYLAND_SUPPORT)
    message(FATAL_ERROR "Unsupported Platform! Please enable atleast X11 or Wayland in the build system")
endif()
