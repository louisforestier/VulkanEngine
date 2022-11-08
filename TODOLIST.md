why is it so slow..? -> because bootstrap select llvmpipe instead of actual gpu 
-> solution : remove bootstrap

A specification violation or potential error has occurred.
validation layer: Validation Error: [ VUID-VkMappedMemoryRange-size-01389 ] Object 0: handle = 0xee24d0000000059, type = VK_OBJECT_TYPE_DEVICE_MEMORY; | MessageID = 0xee4872d | vkFlushMappedMemoryRanges: Size in pMemRanges[1] is VK_WHOLE_SIZE and the mapping end (0xf9c = 0x0 + 0xf9c) not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (0x80) and not equal to the end of the memory object 
(0x1000). The Vulkan spec states: If size is equal to VK_WHOLE_SIZE, the end of the current mapping of memory must either be a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize bytes from the beginning of the memory object, or be equal to the end of the memory object (https://vulkan.lunarg.com/doc/view/1.3.224.1/windows/1.3-extensions/vkspec.html#VUID-VkMappedMemoryRange-size-01389)

refactor everything in snake or camel case

add index buffer

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