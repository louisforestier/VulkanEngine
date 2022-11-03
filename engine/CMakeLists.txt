
# Add source to this project's executable.
add_executable(vulkan_guide
    main.cpp
    vk_engine.cpp
    vk_engine.h
    vk_pipeline.cpp
    vk_pipeline.h
    vk_mesh.cpp
    vk_mesh.h
    vk_textures.cpp
    vk_textures.h
    vk_swapchain.cpp
    vk_swapchain.h
    vk_device_builder.cpp
    vk_device_builder.h
    vk_device_selector.cpp
    vk_device_selector.h
    vk_instance_builder.cpp
    vk_instance_builder.h
    vk_types.h
    vk_initializers.cpp
    vk_initializers.h)


set_property(TARGET vulkan_guide PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:vulkan_guide>")

target_include_directories(vulkan_guide PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(vulkan_guide vma glm tinyobjloader imgui stb_image lz4 json)

target_link_libraries(vulkan_guide Vulkan::Vulkan sdl2)

add_dependencies(vulkan_guide Shaders)
