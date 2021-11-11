if(TARGET glm::glm)
    return()
endif()

message(STATUS "Third-party (external) targets available: 'glm::glm'")

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
    GIT_SHALLOW TRUE
)

#option(GLM_TEST_ENABLE "Build unit tests" OFF)
FetchContent_MakeAvailable(glm)
