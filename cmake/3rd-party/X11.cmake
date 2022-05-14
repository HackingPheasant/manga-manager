if(TARGET X11::X11)
    return()
endif()

# Full list of targets can be found at
# https://cmake.org/cmake/help/latest/module/FindX11.html
message(STATUS "Third-party (external) targets available: 'X11::X11', 'X11::xcb' and many more targets. Check CMake documentation for a full list of targets")

find_package(X11 REQUIRED)
