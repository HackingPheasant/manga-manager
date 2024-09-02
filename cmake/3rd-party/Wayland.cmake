if(TARGET Wayland::client)
    return()
endif()

include(FeatureSummary)

message(VERBOSE "Third-party targets available: 'Wayland::client', 'Wayland::server' and others")

find_package(Wayland REQUIRED)

set_package_properties(Wayland PROPERTIES
    URL "https://wayland.freedesktop.org/"
    DESCRIPTION "Library implementation of the display protocol"
    TYPE REQUIRED)
add_feature_info("Wayland Protocols" wayland_protocols_DATADIR "Specifications of extended Wayland protocols")
add_feature_info("Wayland Scanner" wayland_scanner_EXECUTABLE "Executable that converts XML protocol files to C code")
