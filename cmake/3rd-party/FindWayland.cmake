# Find wayland-client Headers/Library
# Based off how cmake FindXXX.cmake are built

## Imported Targets
# wayland::wayland-client
# The wayland-client library, if found

## Result Variables
# This will define the following variables in your project:
# wayland-client_FOUND
# true if (the requested version of) wayland-client is available.
# wayland-client_VERSION
# the version of wayland-client.
# wayland-client_LIBRARIES
# the libraries to link against to use wayland-client.
# wayland-client_INCLUDE_DIRS
# where to find the wayland-client headers.
# wayland-client_COMPILE_OPTIONS
# this should be passed to target_compile_options(), if the
# target is not used for linking

# TODO Implement the rest for wayland-cursor, wayland-egl,
# wayland-egl-backend, wayland-scanner, wayland-server
# Have a look at FindVulkan, FindX11 or FindLibXslt for
# examples on how to find and add multiple librarys

# Use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig QUIET)
pkg_check_modules(PKG_wayland-client QUIET wayland-client)

set(wayland-client_COMPILE_OPTIONS ${PKG_wayland-client_CFLAGS_OTHER})
set(wayland-client_VERSION ${PKG_wayland-client_VERSION})

find_path(wayland-client_INCLUDE_DIR
    NAMES
    wayland-client.h
    HINTS
    ${PKG_wayland-client_INCLUDE_DIRS}
    )
find_library(wayland-client_LIBRARY
    NAMES
    wayland-client
    libwayland-client
    HINTS
    ${PKG_wayland-client_LIBRARY_DIRS}
    )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(wayland-client
    REQUIRED_VARS
    wayland-client_LIBRARY
    wayland-client_INCLUDE_DIR
    VERSION_VAR
    wayland-client_VERSION
    HANDLE_COMPONENTS
    )

if(wayland-client_FOUND AND NOT TARGET wayland::wayland-client)
    add_library(wayland::wayland-client UNKNOWN IMPORTED)
    set_target_properties(wayland::wayland-client PROPERTIES
        IMPORTED_LOCATION "${wayland-client_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${wayland-client_COMPILE_OPTIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${wayland-client_INCLUDE_DIR}"
        )
endif()

mark_as_advanced(wayland-client_LIBRARY wayland-client_INCLUDE_DIR)

if(wayland-client_FOUND)
    set(wayland-client_LIBRARIES ${wayland-client_LIBRARY})
    set(wayland-client_INCLUDE_DIRS ${wayland-client_INCLUDE_DIR})
endif()
