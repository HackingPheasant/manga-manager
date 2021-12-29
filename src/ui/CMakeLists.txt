add_library("manga-manager_ui")
add_library("manga-manager::ui" ALIAS "manga-manager_ui")

message(STATUS "manga-manager: creating target 'manga-manager::ui'")

include(glm)
include(vulkan)
include(sdl2)

add_custom_command(COMMENT "Compiling Vulkan.hpp tutorial example vertex shader"
    OUTPUT vulkantut.vert.inc
    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V --target-env vulkan1.2 -x -o ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.vert.inc
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.vert
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.vert
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.vert ${GLSLANG_VALIDATOR})
add_custom_command(COMMENT "Compiling Vulkan.hpp tutorial example fragment shader"
    OUTPUT vulkantut.frag.inc
    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V --target-env vulkan1.2 -x -o ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.frag.inc
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.frag
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.frag
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.frag ${GLSLANG_VALIDATOR})

target_sources("manga-manager_ui"
    PRIVATE
    window.cpp
    PUBLIC
    window.h
    vulkan_defines.h
    ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.vert.inc
    ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.frag.inc
    )

target_link_libraries("manga-manager_ui" PUBLIC
    project::options
    Vulkan::Vulkan
    SDL2::SDL2
    glm::glm
    )

target_include_directories("manga-manager_ui" PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # This line is so we can find our compiled shaders
    )

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

#target_include_directories("ui-test"
#    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
#    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
#    )
