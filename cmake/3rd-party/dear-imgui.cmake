if(TARGET imgui::imgui)
    return()
endif()

message(STATUS "Third-party targets available: 'imgui::imgui'")

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
    )

FetchContent_MakeAvailable(imgui)

set(IMGUI_BUILD_METAL_BINDING OFF)
set(IMGUI_BUILD_OSX_BINDING OFF)
set(IMGUI_BUILD_SDL2_BINDING ON)
set(IMGUI_BUILD_SDL2_RENDERER_BINDING OFF)
set(IMGUI_BUILD_VULKAN_BINDING ON)
set(IMGUI_BUILD_WIN32_BINDING OFF)
set(IMGUI_FREETYPE OFF)
set(IMGUI_USE_WCHAR32 OFF)

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

if(IMGUI_BUILD_METAL_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_metal.mm PROPERTIES COMPILE_FLAGS -fobjc-weak)
endif()

if(IMGUI_BUILD_OSX_BINDING)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm)
endif()

if(IMGUI_BUILD_SDL2_BINDING)
    target_link_libraries(imgui PUBLIC SDL2::SDL2)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp)
endif()

if(IMGUI_BUILD_SDL2_RENDERER_BINDING)
    target_link_libraries(imgui PUBLIC SDL2::SDL2)
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp)
endif()

if(IMGUI_BUILD_VULKAN_BINDING)
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

    if(IMGUI_BUILD_METAL_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_OSX_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.h DESTINATION include)
    endif()

    if(IMGUI_BUILD_SDL2_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h DESTINATION include)
    endif()

     if(IMGUI_BUILD_SDL2_RENDERER_BINDING)
        install(FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer.h DESTINATION include)
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
