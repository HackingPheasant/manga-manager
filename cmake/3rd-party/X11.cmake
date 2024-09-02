if(TARGET X11::xcb)
    return()
endif()

include(FeatureSummary)

# Full list of targets can be found at
# https://cmake.org/cmake/help/latest/module/FindX11.html
message(VERBOSE "Third-party targets available: 'X11::X11', 'X11::xcb' and many more targets. Check CMake documentation for a full list of targets")

find_package(X11 QUIET REQUIRED
    COMPONENTS xcb
)

set_package_properties(X11 PROPERTIES
    URL "https://xcb.freedesktop.org/"
    DESCRIPTION "Library imlpementation of the X protocol"
    TYPE REQUIRED)
