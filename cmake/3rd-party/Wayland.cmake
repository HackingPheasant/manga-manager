if(TARGET wayland::wayland-client)
    return()
endif()

message(VERBOSE "Third-party targets available: 'wayland::wayland-client'")

find_package(Wayland REQUIRED)
