# SPDX-FileCopyrightText: © 2022 HackingPheasant <HackingPheasant@protonmail.com>
#
# SPDX-License-Identifier: MIT

# Findxcb.cmake
#
# Finds the XCB (X-protocol C Binding) library and related extensions/utils
#
# Imported Targets
# ================
#
# This module provides the following imported targets, if found:
#
#    xcb::xcb
#
# And possible XCB extensions as imported targets, when requested via the
# COMPONENTS arg to find_package():
#
#    xcb::xcb
#    xcb::xcb-composite
#    xcb::xcb-damage
#    xcb::xcb-dbe
#    xcb::xcb-dpms
#    xcb::xcb-dri2
#    xcb::xcb-dri3
#    xcb::xcb-glx
#    xcb::xcb-present
#    xcb::xcb-randr
#    xcb::xcb-record
#    xcb::xcb-render
#    xcb::xcb-res
#    xcb::xcb-screensaver
#    xcb::xcb-shape
#    xcb::xcb-shm
#    xcb::xcb-sync
#    xcb::xcb-xf86dri
#    xcb::xcb-xfixes
#    xcb::xcb-xinerama
#    xcb::xcb-xinput
#    xcb::xcb-xkb
#    xcb::xcb-xtest
#    xcb::xcb-xv
#    xcb::xcb-xvmc
#
# As well as the following related libraries/utils/programs used inconjunction with XCB
#
#    xcb::xcb-util
#    xcb::xcb-image
#    xcb::xcb-keysyms
#    xcb::xcb-render-util
#    xcb::xcb-ewmh
#    xcb::xcb-icccm
#    xcb::xcb-cursor
#    xcb::xcb-errors
#
# Result Variables
# ================
# This will define the following variables:
#
#    xcb_FOUND
#    True if the system has the xcb library.
#
#    xcb_VERSION
#    The detected version of the core xcb library which was found
#
#    xcb_<COMPONENT>_VERSION
#    The detected version of the found xcb component



#TODO: https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html

# If no componenets are passed to find_package then default to just finding XCB. 
if (NOT xcb_FIND_COMPONENTS)
    set(xcb_FIND_COMPONENTS xcb)
endif()

# List of known components
set(_XCB_COMPONENTS)
list(APPEND
    _XCB_COMPONENTS
    # libxcb
    # TODO/NOTE: bigreq.h (BIG_REQUESTS) and ge.h (Generic Event) extensions don't have thier own .pc files, handle that
    xcb 
    composite
    damage
    dbe
    dpms
    dri2
    dri3
    glx
    present
    randr
    record
    render
    res
    screensaver
    shape
    shm
    sync
    xf86dri
    xfixes
    xinerama
    xinput
    xkb
    xtest
    xv
    xvmc
    # xcb-util and related
    atom #xcb-util
    aux # xcb-util
    event # xcb-util
    util # xcb-util
    cursor # xcb-util-cursor
    errors # xcb-util-errors
    image # xcb-util-image
    keysyms # xcb-util-keysyms
    renderutil # xcb-util-renderutil
    ewmh # xcb-util-wm
    icccm # xcb-util-wm
)

#list(APPEND
#    _XCB_HEADERS
#    # libxcb
#    xcb.h
#    bigreq.h # BIG-REQUESTS extension
#    composite.h
#    damage.h
#    dbe.h
#    dpms.h
#    dri2.h
#    dri3.h
#    ge.h # Generic Event Extension
#    glx.h
#    present.h
#    randr.h
#    record.h
#    render.h
#    res.h
#    screensaver.h
#    shape.h
#    shm.h
#    sync.h
#    xcbext.h
#    xc_misc.h
#    xevie.h
#    xf86dri.h
#    xfixes.h
#    xinerama.h
#    xinput.h
#    xkb.h
#    xprint.h
#    xproto.h
#    xselinux.h
#    xtest.h
#    xv.h
#    xvmc.h
#    # xcb-util and related
#    xcb_atom.h
#    xcb_aux.h
#    xcb_event.h
#    xcb_util.h
#    xcb_cursor.h
#    xcb_errors.h
#    xcb_image.h
#    xcb_keysyms.h
#    xcb_renderutil.h
#    xcb_ewmh.h
#    xcb_icccm.h
#)

foreach(component IN LISTS ${xcb_FIND_COMPONENTS})
    # Bailout early if supplied component isn't in the list of known components
    list(FIND _XCB_COMPONENTS ${component} valid_comonent)

    if(NOT valid_comonent GREATER_EQUAL 0)
        message(FATAL_ERROR "Unknown XCB component required: ${component}.")
    endif()


    set(COMPONENT_NAME xcb-${component})
    set(COMPONENT_HEADER ${component}.h)

    if(component STREQUAL "xcb")
        set(COMPONENT_NAME "xcb")
    elseif((componet STREQUAL "atom")
        OR (componet STREQUAL "aux")
        OR (componet STREQUAL "event")
        OR (componet STREQUAL "util")
        OR (componet STREQUAL "cursor")
        OR (componet STREQUAL "errors")
        OR (componet STREQUAL "image")
        OR (componet STREQUAL "keysyms")
        OR (componet STREQUAL "renderutil")
        OR (componet STREQUAL "eqmh")
        OR (componet STREQUAL "icccm"))
        set(COMPONENT_HEADER xcb_${component}.h)
    endif()

    if(TARGET xcb::${COMPONENT_NAME})
        # If we've already created a target we can skip the rest of this current iteration
        continue()
    endif()
    
    # Some system introspection to help with include/library search paths
    cmake_pkg_config(EXTRACT ${COMPONENT_NAME} QUIET)

    find_path(XCB_${COMPONENT_NAME}_INCLUDE_DIR
        NAMES "xcb/${COMPONENT_HEADER}"
        HINTS "${CMAKE_PKG_CONFIG_INCLUDES}"
        PATHS /usr/include /usr/local/include
        DOC "XCB component ${COMPONENT_NAME} include directory")

    find_library(XCB_${COMPONENT_NAME}_LIBRARY
        NAMES "${COMPONENT_NAME}"
        HINTS "${CMAKE_PKG_CONFIG_LIBNAMES}"
        PATHS /usr/lib /usr/local/lib
        DOC "XCB component ${COMPONENT_NAME} location")

    # Handle version information
    if(${CMAKE_PKG_CONFIG_VERSION} VERSION_GREATER 0)
        set(XCB_${COMPONENT_NAME}_VERSION ${CMAKE_PKG_CONFIG_VERSION})
    else()
        if (NOT xcb_FIND_QUIETLY)
            message(AUTHOR_WARNING "Failed to find ${COMPONENT_NAME} version.")
        endif()
        set(XCB_${COMPONENT_NAME}_VERSION 0.0.0)
    endif()

    # TODO CREATE TARGETS HERE
    # WE HAVE NOW FOUND header, library and version info, and what ever else cmake_pkg_config has found for use

    #basically
    # if (mypackage_FOUND)
    #   if (NOT TARGET mypackage::mypackage)




    #else()
    #endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(xcb
    VERSION_VAR xcb_xcb_VERSION
    HANDLE_VERSION_RANGE
    HANDLE_COMPONENTS)

#mark_as_advanced()

unset(_xcb_COMPONENTS)
unset(_xcb_HEADERS)
