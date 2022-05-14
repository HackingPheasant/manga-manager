if(TARGET wayland::wayland-client)
    return()
endif()

message(STATUS "Third-party (external) targets available: 'wayland::wayland-client'")

find_package(Wayland REQUIRED)
