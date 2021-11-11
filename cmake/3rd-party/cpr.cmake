if(TARGET cpr::cpr)
    return()
endif()

# Disable/Enable CPR configuration
if (NOT DEFINED CPR_BUILD_TESTS)
    set(CPR_BUILD_TESTS OFF)
endif()

if (NOT DEFINED CPR_BUILD_TESTS_SSL)
    set(CPR_BUILD_TESTS_SSL OFF)
endif()

if (NOT DEFINED CPR_FORCE_USE_SYSTEM_CURL)
    set(CPR_FORCE_USE_SYSTEM_CURL ON)
endif()

message(STATUS "Third-party (external) targets available: 'cpr::cpr'")

include(FetchContent)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/whoshuu/cpr.git
    GIT_TAG 1.6.0
    )

FetchContent_MakeAvailable(cpr)
