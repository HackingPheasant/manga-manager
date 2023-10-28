add_library("os_linux_wayland")
add_library("linux::wayland" ALIAS "os_linux_wayland")

include(Wayland)

target_sources("os_linux_wayland"
    PRIVATE
    window.cpp
    PUBLIC
    window.h
    )

target_link_libraries("os_linux_wayland" PUBLIC
    project::options
    wayland::wayland-client
    )
