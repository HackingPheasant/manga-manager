# Project specific settings
cmake_minimum_required(VERSION 3.15)
project(mangadex-dl
        VERSION 0.1.0 
        DESCRIPTION "Mangadex downloader, built with C++"
        HOMEPAGE_URL https://github.com/HackingPheasant/mangadex-dl
        LANGUAGES CXX)

# CMake Modules
include(FetchContent)

# Build settings
# Wrapped if's to allow user override
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# CPR Configuration
if (NOT DEFINED BUILD_CPR_TESTS)
    set(BUILD_CPR_TESTS OFF)
endif()

if (NOT DEFINED BUILD_CPR_TESTS_SSL)
    set(BUILD_CPR_TESTS_SSL OFF)
endif()

# Grab dependencies
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/whoshuu/cpr.git
    GIT_TAG v1.5.1
    )
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.9.0
    )

FetchContent_MakeAvailable(cpr)
FetchContent_MakeAvailable(json)

# Static Checks
# Clang Tidy
# TODO: 
# - Put a check to make optional
# - Check if available on system before using
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)

# Compile program
add_executable(${PROJECT_NAME} src/main.cpp)

# Link the dependencies with our binary
target_link_libraries(${PROJECT_NAME}
                      PRIVATE cpr::cpr
                      PRIVATE nlohmann_json::nlohmann_json)
