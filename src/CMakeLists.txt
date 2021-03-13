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

FetchContent_MakeAvailable(cpr json fmt)

# Compile Library
add_library(${PROJECT_NAME} SHARED
                            http.cpp
                            providers/mangadex.cpp)

# Set library versioning information
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION}
                                                 SOVERSION ${PROJECT_VERSION_MAJOR})

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(${PROJECT_NAME} 
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Check (and enable) position independent code
include(CheckPIESupported)
# Retrieve any error message.
check_pie_supported(OUTPUT_VARIABLE output LANGUAGES CXX)
# Only explicitly enable it for cpr/fmt since thats whats causing the errors
set_property(TARGET cpr PROPERTY POSITION_INDEPENDENT_CODE TRUE)
set_property(TARGET fmt PROPERTY POSITION_INDEPENDENT_CODE TRUE)
if(NOT CMAKE_CXX_LINK_PIE_SUPPORTED)
  message(WARNING "PIE is not supported at link time: ${output}.\n"
                  "PIE link options will not be passed to linker.")
endif()

# Compile (Test) program
add_executable("${PROJECT_NAME}-test" main.cpp)

# Link the dependencies with our binary
target_link_libraries(${PROJECT_NAME}
                      PRIVATE project_options
                      PRIVATE project_warnings
                      PUBLIC cpr::cpr
                      PUBLIC nlohmann_json::nlohmann_json
                      PUBLIC fmt::fmt)


target_link_libraries("${PROJECT_NAME}-test"
                      PRIVATE project_options
                      PRIVATE project_warnings
                      PRIVATE ${PROJECT_NAME})
