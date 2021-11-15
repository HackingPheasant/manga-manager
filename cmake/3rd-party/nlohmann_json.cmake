if(TARGET nlohmann_json::nlohmann_json)
    return()
endif()

message(STATUS "Third-party (external) targets available: 'nlohmann_json::nlohmann_json'")

# Note: Using alternative url as suggested by:
# https://github.com/nlohmann/json#embedded-fetchcontent
# For a (much) smaller repo size/download
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    # GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
    GIT_TAG v3.10.4
    )

FetchContent_MakeAvailable(nlohmann_json)

if (NOT DEFINED JSON_SystemInclude)
    set(JSON_SystemInclude ON)
endif()

if (NOT DEFINED JSON_Diagnostics)
    set(JSON_Diagnostics ON)
endif()
