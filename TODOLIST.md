why is it so slow..? -> because bootstrap select llvmpipe instead of actual gpu 
-> solution : remove bootstrap

create generated assets in a specific folder

refactor everything in snake or camel case

add index buffer in vk_mesh.cpp

Right now, we have one descriptor set per frame for the Set 0 (camera and scene buffers). 
Try to refactor it so it only uses 1 descriptor set and 1 buffer for both camera and scene buffers,
packing both the structs for all frames into the same uniform buffer, and then using dynamic offsets.

decompose shader creation

add batch rendering

add instance rendering

make a glsl generator 

The use of VMA_MEMORY_USAGE_CPU_ONLY and VMA_MEMORY_USAGE_GPU_ONLY is deprecated.
should use VMA_MEMORY_USAGE_AUTO and set flags to a combination of one or more VmaAllocationCreateFlagBits.


separate renderer from logic (movement, etc.)

abstract renderer to be able to use other APIs