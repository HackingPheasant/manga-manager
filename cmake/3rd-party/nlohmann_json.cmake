if(TARGET nlohmann_json::nlohmann_json)
    return()
endif()

include(FeatureSummary)

message(VERBOSE "Third-party targets available: 'nlohmann_json::nlohmann_json'")

if (NOT DEFINED JSON_SystemInclude)
    set(JSON_SystemInclude ON)
endif()

if (NOT DEFINED JSON_Diagnostics)
    set(JSON_Diagnostics ON)
endif()

# Note: Using alternative url as suggested by:
# https://github.com/nlohmann/json#embedded-fetchcontent
# For a (much) smaller repo size/download
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    SYSTEM
    FIND_PACKAGE_ARGS 3.11.3
    NAMES
    nlohmann-json
    nlohmann_json
)

FetchContent_MakeAvailable(nlohmann_json)

set_package_properties(nlohmann_json PROPERTIES
    URL "https://json.nlohmann.me/"
    DESCRIPTION "Modern C++ JSON library"
    TYPE REQUIRED)
