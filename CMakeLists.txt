# SPDX-FileCopyrightText: © 2021 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.31...4.0)

project(manga-manager
    VERSION 0.1.0
    DESCRIPTION "Manga manager, built with C++"
    HOMEPAGE_URL https://github.com/HackingPheasant/manga-manager
    LANGUAGES CXX C) # C is listed becuase of wayland-scanner auto generated code

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/cmake/3rd-party")

# Fail fast and early if someone build in the source directory
include(PreventInSourceBuilds)

# Enable testing with CTest (Required to be in top level CMakeLists.txt)
include(CTest)

# Project supported features
option(X11_SUPPORT "Enable support for X11 when building" OFF)
option(WAYLAND_SUPPORT "Enable support for Wayland when building" OFF)

# Project options
option(BUILD_DOCUMENTATION "Enable building documentation. Default: ${PROJECT_IS_TOP_LEVEL}." ${PROJECT_IS_TOP_LEVEL})
option(BUILD_EXAMPLES "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}." ${PROJECT_IS_TOP_LEVEL})
option(BUILD_TESTING "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}." ${PROJECT_IS_TOP_LEVEL})

# Build system options
# Compiler warnings are off by default, but get turned on automatically when
# using `cmake --preset dev ...` profile or when manually switched on.
option(ENABLE_COMPILE_WARNINGS "Specify whether to enable compiler warnings" OFF)
option(CMAKE_COMPILE_WARNINGS_AS_ERRORS "Specify whether to treat warnings on compile as errors." OFF)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    option(CMAKE_LINK_WARNING_AS_ERROR "Specify whether to treat warnings on link as errors." OFF)
endif()

include(Cache)
include(CompilerWarnings)
include(InterproceduralOptimization) # TODO: Attempt using this
include(CoverageReporting)
include(Fuzzing) # TODO
include(Sanitizers)
include(StaticAnalyzers)

# Interface target to pass to our project targets
add_library(project_options INTERFACE)
add_library(project::options ALIAS project_options)

target_link_libraries(project_options INTERFACE platform_wsi_defines)
enable_compiler_warnings(project_options)
enable_sanitizers(project_options) # NOTE: project_options comes from CompilerOptions.cmake

# TODO MOVE ELSEWHERE
# Intergrate Vulkan Defines with compiler options
target_compile_definitions(project_options INTERFACE
    # Enable C++ Designated Initializers in Vulkan.{hpp/cppm}
    VULKAN_HPP_NO_CONSTRUCTORS
    # Disable setter member functions
    VULKAN_HPP_NO_SETTERS
    # We currently do no make use of UniqueHandle
    VULKAN_HPP_NO_SMART_HANDLE
    # Use Dynamic Dispatcher
    # TODO: It's currently disabled, debug our issue so we can use it
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=0
)

# Dependencies
# License identifier list (for reference):
# https://spdx.org/licenses/
# https://spdx.org/licenses/exceptions-index.html

include(FeatureSummary)

# glm
# License: MIT
find_package(glm 1.0.0 REQUIRED CONFIG)
set_package_properties(glm PROPERTIES
    URL "https://glm.g-truc.net/"
    DESCRIPTION "C++ mathematics library for graphics software."
    TYPE REQUIRED
    PURPOSE "Required for graphics output")

# glslang
# License (not including preprocessing): BSD-3-Clause AND BSD-2-Clause AND MIT AND Apache-2.0 AND GPL-3.0-or-later WITH Bison-exception-2.2
# License (preprocessing included): BSD-3-Clause AND BSD-2-Clause AND MIT AND Apache-2.0 AND GPL-3.0-or-later WITH Bison-exception-2.2 AND AML-glslang AND MIT-Khronos-old
# Reference:
# https://github.com/KhronosGroup/glslang/blob/main/LICENSE.txt
# https://github.com/KhronosGroup/glslang/blob/main/license-checker.cfg
find_package(glslang REQUIRED)
set_package_properties(glslang PROPERTIES
    URL "https://github.com/KhronosGroup/glslang"
    DESCRIPTION "Offical reference compiler for OpenGL shading languages."
    TYPE REQUIRED
    PURPOSE "Required for graphics output")

# nlohmann::json
# License: MIT
find_package(nlohmann_json REQUIRED)
set_package_properties(nlohmann_json PROPERTIES
    URL "https://json.nlohmann.me/"
    DESCRIPTION "Modern C++ JSON library"
    TYPE RECOMMENDED
    PURPOSE "Recommended for interacting with various API's")

# Vulkan
# License: Apache-2.0 OR MIT
#
# We require a Vulkan version ≥ 1.3.275, this is when VulkanHeaders started 
# shipping it's Vulkan*Config.cmake files with the SDK
#
# https://gitlab.kitware.com/cmake/cmake/-/issues/25617
# https://vulkan.lunarg.com/issue/view/6598781b5df1125b58afbb3c
find_package(VulkanHeaders 1.3.275 REQUIRED CONFIG HINTS $ENV{VULKAN_SDK})
set_package_properties(VulkanHeaders PROPERTIES
    URL "https://github.com/KhronosGroup/Vulkan-Headers"
    DESCRIPTION "Provides Vulkan function prototypes to interact with the graphics API"
    TYPE REQUIRED
    PURPOSE "Required for graphics output")


# libxcb
# License: MIT
find_package(xcb COMPONENTS xcb xcb-util)
set_package_properties(xcb PROPERTIES
    URL "https://xcb.freedesktop.org/"
    DESCRIPTION "The X protocol C-language Binding (XCB) is a replacement for Xlib featuring a small footprint, latency hiding, direct access to the protocol, improved threading support, and extensibility"
    TYPE RECOMMENDED
    PURPOSE "Required if supporting X11 in the project")

# wayland
# License: MIT 
find_package(Wayland)
set_package_properties(Wayland PROPERTIES
    URL "https://wayland.freedesktop.org/"
    DESCRIPTION "Library implementation of the display protocol"
    TYPE RECOMMENDED
    PURPOSE "Required if supporting Wayland in the project")

# wayland-scanner
# TODO: Should be part of wayland package, since the wayland-scaner code shares the same repo
# License: MIT
find_package(wayland-scanner)
set_package_properties(wayland-scanner PROPERTIES
    URL "https://wayland.freedesktop.org/"
    DESCRIPTION "Executable that converts XML protocol files to C code"
    TYPE RECOMMENDED
    PURPOSE "Generates the necessary code from Wayland Protocols")
# wayland-protocols
# License: MIT
find_package(wayland-protocols)
set_package_properties(wayland-protocols PROPERTIES
    URL "https://gitlab.freedesktop.org/wayland/wayland-protocols"
    DESCRIPTION "Contains Wayland protocols that add functionality not available in the Wayland core protocol."
    TYPE RECOMMENDED
    PURPOSE "The XML Files that are feed into wayland-scanner to generate code")

# Our code to build!
# Make sure to link to "project::options" on any targets we build.

# Libraries 
add_subdirectory(src)

# Applications built with/atop our libraries
add_subdirectory(applications)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_DOCUMENTATION)
    #add_subdirectory(docs)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


# Packaging
# Make sure we get our LICENSE/README provided with the program when packaged.
# TODO https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

install(FILES LICENSE README.md
    TYPE DATA)

include(Packaging)

feature_summary(DEFAULT_DESCRIPTION WHAT ALL)
feature_summary(FILENAME "${CMAKE_CURRENT_BINARY_DIR}/features.log" WHAT ALL)
# Note: Doesn't seem to correctly mark pkg-config and FetchContent supplied
# dependencies as found. Also the docs are shit, so linking issues that have 
# helpful info
# https://gitlab.kitware.com/cmake/cmake/-/issues/22689
# https://gitlab.kitware.com/cmake/cmake/-/issues/25322
# https://gitlab.kitware.com/cmake/cmake/-/issues/26486
# https://gitlab.kitware.com/cmake/cmake/-/issues/26487
# So for now we will filter out the PACKAGES_NOT_FOUND because of the
# false postives.
