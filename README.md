# VulkanEngine

Vulkan rendering engine, made mainly by following https://vkguide.dev/ at first.

This is my main side project, outside of College and work.

Right now, I'm still working on the tutorial but the last parts, especially the GPU driven rendering and the beginning of "bindless rendering".

When it will be done, I'm planning to expand on it to try my hand at other subjects like using other APIs, other rendering method (deferred rendering, forward+, Vulkan Ray Tracing, Entity Component System, etc.).

This is mostly a learning project.

The roadmap for GPU Driven Rendering:
- CVARS
- logger
- imgui widgets
- profiler (tracy)
- camera
- indexed drawcall
- improved buffer handling
- pushbuffer
- better asset system
- scene (scenerender)
- shaders
- draw indirect
- compute shaders
- material system
- rendering
- compute base culling

After: 

- add vk_check to vk descriptor methods
- create generated assets in a specific folder
- refactor everything in snake or camel case
- add index buffer in vk_mesh.cpp
- Right now, we have one descriptor set per frame for the Set 0 (camera and scene buffers). 
Try to refactor it so it only uses 1 descriptor set and 1 buffer for both camera and scene buffers,
packing both the structs for all frames into the same uniform buffer, and then using dynamic offsets.
- decompose shader creation
- add batch rendering
- add instance rendering
- make a glsl generator 
- The use of VMA_MEMORY_USAGE_CPU_ONLY and VMA_MEMORY_USAGE_GPU_ONLY is deprecated.
should use VMA_MEMORY_USAGE_AUTO and set flags to a combination of one or more VmaAllocationCreateFlagBits.
- separate renderer from logic (movement, etc.)
- abstract renderer to be able to use other APIs
