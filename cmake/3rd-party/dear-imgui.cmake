if(TARGET imgui::imgui)
    return()
endif()

message(STATUS "Third-party (external) targets available: 'imgui::imgui'")

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.81
    )

FetchContent_MakeAvailable(imgui)

# TODO: Make all below "if's" avaible via the option target
# But for now this will do.
set(IMGUI_BUILD_SDL2_BINDING ON)
set(IMGUI_BUILD_VULKAN_BINDING ON)

# Taken from:
# - https://github.com/microsoft/vcpkg/blob/master/ports/imgui/CMakeLists.txt
# - https://github.com/microsoft/vcpkg/blob/master/ports/imgui/imgui-config.cmake.in
add_library(imgui "")
add_library(imgui::imgui ALIAS imgui)
target_include_directories(
    imgui
    SYSTEM
    PUBLIC
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_sources(
    imgui
    PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

if(IMGUI_BUILD_ALLEGRO5_BINDING)
    find_path(ALLEGRO5_INCLUDE_DIRS allegro5/allegro.h)
    target_include_directories(imgui PRIVATE ${ALLEGRO5_INCLUDE_DIRS})
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_allegro5.cpp)
endif()

if(IMGUI_BUILD_DX9_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp)
endif()

if(IMGUI_BUILD_DX10_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_dx10.cpp)
endif()

if(IMGUI_BUILD_DX11_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)
endif()

if(IMGUI_BUILD_DX12_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp)
endif()

if(IMGUI_BUILD_GLFW_BINDING)
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(imgui PUBLIC glfw)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
endif()

if(IMGUI_BUILD_GLUT_BINDING)
    find_package(GLUT REQUIRED)
    target_link_libraries(imgui PUBLIC GLUT::GLUT)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_glut.cpp)
endif()

if(IMGUI_BUILD_METAL_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm)
endif()

if(IMGUI_BUILD_OPENGL2_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp)
endif()

if(IMGUI_BUILD_OPENGL3_GLEW_BINDING)
    find_package(GLEW REQUIRED)
    target_link_libraries(imgui PUBLIC GLEW::GLEW)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
endif()

if(IMGUI_BUILD_OPENGL3_GLAD_BINDING)
    find_package(glad CONFIG REQUIRED)
    target_link_libraries(imgui PUBLIC glad::glad)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
endif()

if(IMGUI_BUILD_OPENGL3_GL3W_BINDING)
    find_package(gl3w CONFIG REQUIRED)
    target_link_libraries(imgui PUBLIC unofficial::gl3w::gl3w)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
endif()

if(IMGUI_BUILD_OPENGL3_GLBINDING_BINDING)
    find_package(glbinding CONFIG REQUIRED)
    target_link_libraries(imgui PUBLIC glbinding::glbinding)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
endif()

if(IMGUI_BUILD_OSX_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm)
endif()

if(IMGUI_BUILD_SDL2_BINDING)
    #find_package(SDL2 CONFIG REQUIRED)
    target_link_libraries(imgui PUBLIC SDL2::SDL2)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp)
endif()

if(IMGUI_BUILD_VULKAN_BINDING)
    #find_package(Vulkan REQUIRED)
    target_link_libraries(imgui PUBLIC Vulkan::Vulkan)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)
endif()

if(IMGUI_BUILD_WIN32_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
endif()

if(IMGUI_FREETYPE)
    find_package(freetype CONFIG REQUIRED)
    target_link_libraries(imgui PUBLIC freetype)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp)
    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)
endif()

if(IMGUI_USE_WCHAR32)
    target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32)
endif()

# Check for obsolete functions
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

list(REMOVE_DUPLICATES BINDINGS_SOURCES)

install(
    TARGETS imgui
    EXPORT imgui_target
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

foreach(BINDING_TARGET ${BINDING_TARGETS})
    install(
        TARGETS ${BINDING_TARGET}
        EXPORT imgui_target
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endforeach()

if(NOT IMGUI_SKIP_HEADERS)
    install(FILES
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imconfig.h
        ${imgui_SOURCE_DIR}/imgui_internal.h
        ${imgui_SOURCE_DIR}/imstb_textedit.h
        ${imgui_SOURCE_DIR}/imstb_rectpack.h
        ${imgui_SOURCE_DIR}/imstb_truetype.h
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
        DESTINATION include
    )

    if(IMGUI_BUILD_ALLEGRO5_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_allegro5.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_DX9_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_DX10_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_dx10.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_DX11_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_DX12_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_GLFW_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_GLUT_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_glut.h DESTINATION include)
    endif()

    if(IMGUI_COPY_MARMALADE_BINDING)
        file(GLOB MARMALADE_BINDING_SRCS ${imgui_SOURCE_DIR}/backends/imgui_impl_marmalade.*)
        install(FILES ${MARMALADE_BINDING_SRCS} DESTINATION include/bindings)
    endif()

    if(IMGUI_BUILD_METAL_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_OPENGL2_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_OPENGL3_GLEW_BINDING OR IMGUI_BUILD_OPENGL3_GLAD_BINDING OR IMGUI_BUILD_OPENGL3_GL3W_BINDING OR IMGUI_BUILD_OPENGL3_GLBINDING_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_OSX_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_SDL2_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_VULKAN_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_WIN32_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.h DESTINATION include)
    endif()

    if(IMGUI_FREETYPE)
        install(FILES ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.h DESTINATION include)
    endif()
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(imgui-config.cmake.in imgui-config.cmake INSTALL_DESTINATION 3rd-party/imgui)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/imgui-config.cmake DESTINATION 3rd-party/imgui)

install(
    EXPORT imgui_target
    NAMESPACE imgui::
    FILE imgui-targets.cmake
    DESTINATION 3rd-party/imgui
)
