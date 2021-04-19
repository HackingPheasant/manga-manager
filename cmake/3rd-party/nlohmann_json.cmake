if(TARGET nlohmann_json::nlohmann_json)
    return()
endif()

message(STATUS "Third-party (external): creating target 'nlohmann_json::nlohmann_json'")

# Note: Using alternative url as suggested by:
# https://github.com/nlohmann/json#embedded-fetchcontent
# For a (much) smaller repo size/download
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    # GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
    GIT_TAG v3.9.1
    )

FetchContent_MakeAvailable(nlohmann_json)

if (NOT DEFINED JSON_BuildTests)
    set(JSON_BuildTests OFF)
endif()
