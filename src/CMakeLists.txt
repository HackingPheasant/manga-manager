# Enable CMake Modules
include(FetchContent)

# lib configuration
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

# Compile program
add_executable(${PROJECT_NAME} main.cpp)

# Link the dependencies with our binary
target_link_libraries(${PROJECT_NAME}
                      PRIVATE project_options
                      PRIVATE project_warnings
                      PRIVATE cpr::cpr
                      PRIVATE nlohmann_json::nlohmann_json)
