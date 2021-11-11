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

add_executable("ui-test" 
    src/main.cpp
    vulkantut.vert.inc
    vulkantut.frag.inc
    )

target_link_libraries("ui-test" PUBLIC
    project::options
    project::warnings
    Vulkan::Vulkan
    SDL2::SDL2
    glm::glm
    )

target_include_directories("ui-test"
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    )
