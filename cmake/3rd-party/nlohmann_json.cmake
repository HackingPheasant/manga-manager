if(TARGET nlohmann_json::nlohmann_json)
    return()
endif()

message(VERBOSE "Third-party targets available: 'nlohmann_json::nlohmann_json'")

if (NOT DEFINED JSON_SystemInclude)
    set(JSON_SystemInclude ON)
endif()

if (NOT DEFINED JSON_Diagnostics)
    set(JSON_Diagnostics ON)
endif()

# Note (2023-11-13): Swapped to offical repo again and pulling via a commit tag 
# anytime after
# https://github.com/nlohmann/json/commit/fac07e22c5d7dd0423ccf31c02db5603d27e6556
# so we don't face CMake deprecation warings while using CMake 2.7 or above

# Note: Using alternative url as suggested by:
# https://github.com/nlohmann/json#embedded-fetchcontent
# For a (much) smaller repo size/download
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    # GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
    GIT_TAG 6eab7a2b187b10b2494e39c1961750bfd1bda500
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
    )

FetchContent_MakeAvailable(nlohmann_json)
