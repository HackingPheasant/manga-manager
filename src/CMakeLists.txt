# Enable CMake Modules
include(FetchContent)

# lib configuration
#json
if (NOT DEFINED JSON_BuildTests)
    set(JSON_BuildTests OFF)
endif()

#cpr
if (NOT DEFINED BUILD_CPR_TESTS)
    set(BUILD_CPR_TESTS OFF)
endif()

if (NOT DEFINED BUILD_CPR_TESTS_SSL)
    set(BUILD_CPR_TESTS_SSL OFF)
endif()

if (NOT DEFINED USE_SYSTEM_CURL)
    set(USE_SYSTEM_CURL ON)
endif()

# Grab dependencies
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/whoshuu/cpr.git
    GIT_TAG c34ddb9
    )
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.9.1
    )
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 7.1.0
    )

FetchContent_MakeAvailable(cpr)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(fmt)
# Compile program
add_executable(${PROJECT_NAME} mangadex.cpp
                               main.cpp)

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(${PROJECT_NAME} 
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link the dependencies with our binary
target_link_libraries(${PROJECT_NAME}
                      PRIVATE project_options
                      PRIVATE project_warnings
                      PRIVATE cpr::cpr
                      PRIVATE nlohmann_json::nlohmann_json
                      PRIVATE fmt::fmt)
