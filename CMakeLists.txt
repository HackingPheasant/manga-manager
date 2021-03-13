# Project specific settings
cmake_minimum_required(VERSION 3.15...3.20)
project(manga-manager
        VERSION 0.1.0 
        DESCRIPTION "Manga manager, built with C++"
        HOMEPAGE_URL https://github.com/HackingPheasant/manga-manager
        LANGUAGES CXX)

# Add cmake helpers to cmake module path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(StandardProjectSettings)
include(PreventInSourceBuilds)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(Cache)

# standard compiler warnings
include(CompilerWarnings)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(Sanitizers)
enable_sanitizers(project_options)

# allow for static analysis options
include(StaticAnalyzers)

# Shared libs (.dll/.so) or Static libs (.lib/.a)
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
# We should at least ENABLE_TESTING when we have some tests
option(ENABLE_TESTING "Enable Test Builds" OFF) 
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests.")
  add_subdirectory(test)
endif()

if(ENABLE_FUZZING)
  message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
  add_subdirectory(fuzz_test)
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
