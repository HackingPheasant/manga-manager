# Allow for cmake options to depend on other options
include(CMakeDependentOption)

# Some project specific options
option(ENABLE_WARNINGS "Enable compiler warnings" OFF)
cmake_dependent_option(ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF "ENABLE_WARNINGS" ON)
# Off by default, feel free to switch to on by default if you current builds have zero errors
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)
option(BUILD_DOCUMENTATION "Build documentation" OFF)
option(BUILD_EXAMPLES  "Build all examples" OFF)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE
        RelWithDebInfo
        CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui, ccmake
    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS
        "Debug"
        "Release"
        "MinSizeRel"
        "RelWithDebInfo")
endif()

# Generate compile_commands.json to make it easier to work with clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable IPO/LTO
option(ENABLE_IPO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" OFF)

if(ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(
        RESULT
        result
        OUTPUT
        output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(SEND_ERROR "IPO is not supported: ${output}")
    endif()
endif()

# Enable Color Diagnostics on supported generators and compilers
set(CMAKE_COLOR_DIAGNOSTICS ON)
