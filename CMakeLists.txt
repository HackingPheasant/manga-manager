# Project specific settings
cmake_minimum_required(VERSION 3.21)
# Minimum 3.21 because of CMakePresets.json changes and FindVulkan.cmake changes

project(manga-manager
    VERSION 0.1.0
    DESCRIPTION "Manga manager, built with C++"
    HOMEPAGE_URL https://github.com/HackingPheasant/manga-manager
    LANGUAGES CXX)

# Add cmake helpers and 3rd party library helpers to cmake module path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/cmake/3rd-party" ${CMAKE_MODULE_PATH})

include(StandardProjectSettings)
include(PreventInSourceBuilds)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
add_library(project::options ALIAS project_options)
target_compile_features(project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if(ENABLE_BUILD_WITH_TIME_TRACE)
        target_compile_options(project_options INTERFACE -ftime-trace)
    endif()
endif()

# enable cache system
include(Cache)

# standard compiler warnings
include(CompilerWarnings)

# sanitizer options if supported by compiler
include(Sanitizers)
enable_sanitizers(project_options)

# allow for static analysis options
include(StaticAnalyzers)

# allow for cmake options to depend on other options
include(CMakeDependentOption)

# Options
option(ENABLE_WARNINGS "Enable compiler warnings" FALSE)
# Off by default, feel free to switch to on by default if you current builds have zero errors
cmake_dependent_option(ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" FALSE "ENABLE_WARNINGS" ON)
# We should at least ENABLE_TESTING when we have some tests
option(ENABLE_TESTING "Enable Test Builds" OFF) 
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)
option(BUILD_DOCUMENTATION "Build documentation" OFF)
option(BUILD_EXAMPLES  "Build all examples" OFF)

# Enable testing at top level project
if(ENABLE_TESTING)
    include(CTest)
    enable_testing()

    include(catch2)
    Fetch_GetProperties(catch2)
    set(CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras" ${CMAKE_MODULE_PATH})
endif()

# Enable fuzzing
if(ENABLE_FUZZING)
endif()

# Build documentation
if(BUILD_DOCUMENTATION)
endif()

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
#
# But only if this variables are not defined yet
# (i.e. libevent is used via add_subdirectory())
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()
if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

add_subdirectory(src)

# TODO: Write install commands
