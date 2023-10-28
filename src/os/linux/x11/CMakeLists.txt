add_library("os_linux_x11")
add_library("linux::x11" ALIAS "os_linux_x11")

include(X11)

target_sources("os_linux_x11"
    PRIVATE
    window.cpp
    PUBLIC
    window.h
    )

target_link_libraries("os_linux_x11" PUBLIC
    project::options
    X11::xcb
    )
