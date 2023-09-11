if(TARGET glm::glm)
    return()
endif()

message(STATUS "Third-party targets available: 'glm::glm'")

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    # Latest commit as of 2023/09/11
    # glm hasn't had an offical release since April 2020
    GIT_TAG 47585fde0c49fa77a2bf2fb1d2ead06999fd4b6e 
    GIT_SHALLOW TRUE
)

#option(GLM_TEST_ENABLE "Build unit tests" OFF)
FetchContent_MakeAvailable(glm)
