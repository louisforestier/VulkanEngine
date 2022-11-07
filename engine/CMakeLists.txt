
set(CMAKE_CXX_STANDARD 17)
# Add source to this project's executable.


file(GLOB ENGINE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable (engine ${ENGINE_FILES})


set_property(TARGET engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:engine>")

target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(engine vma glm tinyobjloader imgui stb_image)

target_link_libraries(engine Vulkan::Vulkan sdl2 assetlib)

add_dependencies(engine Shaders)
