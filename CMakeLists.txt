# Project specific settings
cmake_minimum_required(VERSION 3.27...3.29)
# Reasons we bumbed the minimum cmake version:
# - 3.21: Absolute minimum because of CMakePresets.json changes and FindVulkan.cmake changes
# - 3.24 Because of improvements to FetchContent
# - 3.25 Because of improvements to CMakePresets.json and clang-tidy options
# - 3.26 Because of clang-tidy export fixes option.

project(manga-manager
    VERSION 0.1.0
    DESCRIPTION "Manga manager, built with C++"
    HOMEPAGE_URL https://github.com/HackingPheasant/manga-manager
    LANGUAGES C CXX)

# Add cmake helpers and 3rd party library helpers to cmake module path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/cmake/3rd-party" ${CMAKE_MODULE_PATH})

# Enable testing with CTest (Required to be in top level CMakeLists.txt)
include(CTest)

# Set some default project settings and throw project
# specific options if they have nowhere else to go
include(StandardProjectSettings)

# Prevent in source builds
include(PreventInSourceBuilds)

# Enable cache system
include(Cache)

# Standard compiler options and warnings
include(CompilerOptions)

# Sanitizer options if supported by compiler
include(Sanitizers)
enable_sanitizers(project_options)

# Allow for static analysis options
include(StaticAnalyzers)

# Our code to build
# Make sure to link to "project::options" on any targets you build.
add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(ENABLE_FUZZING)
endif()

if(BUILD_DOCUMENTATION)
endif()

# Packaging
# Make sure we get our LICENSE/README provided with the program when packaged.
# TODO https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(FILES LICENSE README.md
    TYPE DATA)

include(Packaging)

# Note: Doesn't seem to correctly mark pkg-config and FetchContent supplied
# dependencies as found
# https://gitlab.kitware.com/cmake/cmake/-/issues/22689
# So for now we will filter out the PACKAGES_NOT_FOUND because of the
# false postives. Meaning we can't make use of below flag.
# TODO: See about using FATAL_ON_MISSING_REQUIRED_PACKAGES
include(FeatureSummary)
feature_summary(DEFAULT_DESCRIPTION WHAT PACKAGES_FOUND ENABLED_FEATURES)
