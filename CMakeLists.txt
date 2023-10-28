# Project specific settings
cmake_minimum_required(VERSION 3.24)
# Absolute minimum was 3.21 because of CMakePresets.json changes and FindVulkan.cmake changes

project(manga-manager
    VERSION 0.1.0
    DESCRIPTION "Manga manager, built with C++"
    HOMEPAGE_URL https://github.com/HackingPheasant/manga-manager
    LANGUAGES CXX)

# Add cmake helpers and 3rd party library helpers to cmake module path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/cmake/3rd-party" ${CMAKE_MODULE_PATH})

# Set some default project settins and throw project 
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
add_subdirectory(src)
