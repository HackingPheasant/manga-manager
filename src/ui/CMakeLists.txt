add_library("manga-manager_ui")
add_library("manga-manager::ui" ALIAS "manga-manager_ui")

message(VERBOSE "manga-manager: creating target 'manga-manager::ui'")

include(glm)
include(vulkan)
include(sdl2)

if(BUILD_LINUX_X11_SUPPORT)
    include(X11)
endif()

if(BUILD_LINUX_WAYLAND_SUPPORT)
    include(Wayland)
endif()

add_custom_command(COMMENT "Compiling Vulkan.hpp tutorial example vertex shader"
    OUTPUT vulkantut.vert.inc
    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V --target-env vulkan1.3 -x -o ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.vert.inc
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.vert
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.vert
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.vert ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE})
add_custom_command(COMMENT "Compiling Vulkan.hpp tutorial example fragment shader"
    OUTPUT vulkantut.frag.inc
    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V --target-env vulkan1.3 -x -o ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.frag.inc
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.frag
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.frag
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/vulkantut.frag ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE})

target_sources("manga-manager_ui"
    PRIVATE
    window.cpp
    vulkan_renderer.cpp
    PUBLIC
    window.h
    vulkan_renderer.h
    ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.vert.inc
    ${CMAKE_CURRENT_BINARY_DIR}/vulkantut.frag.inc
    )

target_link_libraries("manga-manager_ui" PUBLIC
    project::options
    Vulkan::Vulkan
    SDL2::SDL2
    glm::glm
    )

if(BUILD_LINUX_X11_SUPPORT)
    target_link_libraries("manga-manager_ui" PUBLIC
        X11::xcb
        )
endif()

if(BUILD_LINUX_WAYLAND_SUPPORT)
    target_link_libraries("manga-manager_ui" PUBLIC
        wayland::wayland-client
        )
endif()

target_include_directories("manga-manager_ui" PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR} # This line is so we can find our compiled shaders
    )

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
