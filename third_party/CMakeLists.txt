find_package(Vulkan REQUIRED)
include(ExternalProject)

add_library(glm INTERFACE)
add_library(vma INTERFACE)

add_library(stb_image INTERFACE)
add_library(json INTERFACE)

add_library(tinyobjloader STATIC)

add_library(lz4 STATIC)

target_sources(lz4 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.c"
)

target_include_directories(lz4 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lz4" )

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)

target_include_directories(json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json" )
target_sources(tinyobjloader PRIVATE 
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

add_subdirectory(SDL)

add_library(imgui STATIC)

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
# It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(imgui PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(imgui PRIVATE SDL2::SDL2)


target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE 
    imgui/imgui.h
    imgui/imgui.cpp
    imgui/imgui_stdlib.h
    imgui/imgui_stdlib.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp

    imgui/imgui_impl_vulkan.cpp
    imgui/imgui_impl_sdl2.cpp
    )

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

target_include_directories(stb_image INTERFACE stb_image)

add_subdirectory(fmt)

option( TRACY_ENABLE "Enable profiling" OFF)
add_subdirectory(tracy)
