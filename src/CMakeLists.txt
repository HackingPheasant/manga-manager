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
    GIT_TAG 1.5.2
    )
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.9.1
    )

FetchContent_MakeAvailable(cpr)
FetchContent_MakeAvailable(json)

# Compile program
add_executable(${PROJECT_NAME} mangadex.cpp main.cpp)

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(${PROJECT_NAME} 
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link the dependencies with our binary
target_link_libraries(${PROJECT_NAME}
                      PRIVATE project_options
                      PRIVATE project_warnings
                      PRIVATE cpr::cpr
                      PRIVATE nlohmann_json::nlohmann_json)
